#include "Bot.hpp"
#include "Debug.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <poll.h>
#include "IrcUtils.hpp"

Bot::Bot(std::string host, unsigned short port, std::string password,
		std::string nickname, std::string username, std::string realname)
		: socket_(-1), host_(host), port_(port), password_(password),
		  nickname_(nickname), username_(username), realname_(realname) {
	debug("Bot constructor called");
}

Bot::Bot(const Bot& other) {
	debug("Bot copy constructor called");
	*this = other;
}

Bot& Bot::operator=(const Bot& other) {
	debug("Bot assignment operator called");
	if (this != &other) {
		this->host_ = other.host_;
		this->port_ = other.port_;
		this->password_ = other.password_;
		this->nickname_ = other.nickname_;
		this->username_ = other.username_;
		this->realname_ = other.realname_;
		//socket_ is not copied
	}
	return *this;
}

Bot::~Bot() {
	debug("Bot destructor called");
}

void	Bot::sendRaw(const std::string& msg) const
{
	if (this->socket_ == -1)
		throw std::runtime_error("[Bot] not connected");
	::safeSend(this->socket_, msg + "\r\n");
}

void Bot::sendPass() const
{
	sendRaw("PASS " + password_);
}

void Bot::sendNick() const
{
	sendRaw("NICK " + nickname_);
}
void Bot::sendUser() const
{
	sendRaw("USER " + username_ + " 0 * :" + realname_);
}

bool Bot::connect() {
	struct addrinfo hints;
	struct addrinfo *res = 0, *p = 0;
	int err = 0;
	int socket_fd = -1;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;  // TCP

	std::string port_str = toString(port_);
	err = getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &res);
	if (err != 0) {
		std::cout << "getaddrinfo() failed: " << gai_strerror(err) << std::endl;
		return false;
	}

	for (p = res; p != 0; p = p->ai_next) {
		socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_fd == -1) {
			continue; // try next address
		}
		if (::connect(socket_fd, p->ai_addr, p->ai_addrlen) == 0) {
			break; // success
		}
		// connect failed for this addr
		close(socket_fd);
		socket_fd = -1;
	}

	freeaddrinfo(res);

	if (socket_fd == -1) {
		std::cout << "connect() failed: " << strerror(errno) << std::endl;
		return false;
	}

	this->socket_ = socket_fd;
	std::cout << "Connected socket " << socket_fd << std::endl;
	return true;
}

void Bot::disconnect()
{
	this->running_ = false;
	if (socket_ != -1) {
		try {
			sendRaw("QUIT");
		} catch (const std::exception&) {
			// ignore send errors on disconnect
		}
		// TODO, check via poll loop whether message was sent before closing socket
		// (implement in safeSend)
		// for now, just sleep a bit
		close(socket_);
		socket_ = -1;
		std::cout << "Disconnected" << std::endl;
	}
}

void Bot::login()
{
	// TODO send PASS, NICK, USER commands
	if (!password_.empty())
		sendPass();
	sendNick();
	sendUser();
}

// Reply to server PINGs to keep the connection alive
void Bot::onPing(const std::string& token)
{
	try {
		if (token.empty())
			sendRaw("PONG");
		else
			sendRaw("PONG " + token); // preserve leading ':' if present
	} catch (const std::exception& e) {
		std::cerr << "[Bot] Failed to send PONG: " << e.what() << std::endl;
		this->running_ = false;
	}
}

// Non-Blocking poll loop
void Bot::run()
{
	this->running_ = true;
	try {
		if (this->socket_ == -1)
			throw std::runtime_error("[Bot] run() called without a connected socket");

		struct pollfd pfd;
		pfd.fd = this->socket_;
		pfd.events = POLLIN | POLLHUP;
		pfd.revents = 0;

		std::string buffer; // accumulate until CRLF

		while (this->running_) {
			int rc = poll(&pfd, 1, 1000); // 1s timeout
			if (!this->running_)
				break;

			if (rc == -1) {
				if (errno == EINTR)
					continue; // interrupted by signal; retry
				throw std::runtime_error("[Bot] poll() failed");
			}
			if (rc == 0) {
				// timeout; nothing to read
				continue;
			}

			if (pfd.revents & POLLHUP) {
				// server closed the connection
				break;
			}

			if (pfd.revents & POLLIN) {
				char chunk[4096];
				ssize_t n = recv(this->socket_, chunk, sizeof(chunk), MSG_DONTWAIT);
				if (n == 0) {
					// server closed the connection
					break;
				} else if (n < 0) {
					if (errno == EAGAIN)
						continue;
					throw std::runtime_error("[Bot] recv() failed");
				}
				buffer.append(chunk, n);
				// Process complete lines ending with CRLF
				size_t pos;
				while ((pos = buffer.find("\n")) != std::string::npos) {
					std::string line = buffer.substr(0, pos);
					buffer.erase(0, pos + 1); // remove line + CRLF
					if (line.empty())
						continue;
					if (line[line.size() - 1] == '\r')
						line.erase(line.size() - 1); // remove CR
					if (line.empty())
						continue;
					std::cout << "<<< " << line << std::endl;
					// TODO call line handler here
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error in Bot run loop: " << e.what() << std::endl;
	}
	this->disconnect();
}
