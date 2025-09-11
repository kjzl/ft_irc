#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Command.hpp"
#include "../include/Debug.hpp"
#include "../include/IrcUtils.hpp"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <poll.h>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

bool Server::running_ = false;

void	Server::signalHandler(int signum)
{
	static_cast<void>(signum);
	running_ = false;
	debug("received a signal");
}

// Default Constructor
Server::Server(void): name_(HOSTNAME), port_(6667), password_("password"), timeCreated_(std::time(NULL))
{
	running_ = true;
	debug("Default Constructor called");
}

// Parameterized Constructor
Server::Server(int port, std::string password): name_(HOSTNAME), port_(port), password_(password), serverSocket_(-1), timeCreated_(std::time(NULL))
{
	debug("Parameterized Constructor called");
	std::cout << GREEN << "==== STARTING SERVER ====" << RESET << std::endl;
	std::cout << BLUE << "port: " << port << ", password: " << password << RESET << std::endl;
	running_ = true;
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	serverInit();
}

// Destructor
Server::~Server()
{
	debug("Destructor called");
	// serverShutdown();
}

// Copy Constructor
Server::Server(const Server &other)
	: name_(other.name_), port_(other.port_), password_(other.password_),
	  serverSocket_(other.serverSocket_), pollFds_(other.pollFds_),
	  clients_(other.clients_), timeCreated_(other.timeCreated_),
	  messageQueueManager_(other.messageQueueManager_),
	  pendingCloseClients_(other.pendingCloseClients_)
// channels_(other.channels_)
{}

// Copy Assignment Operator
Server &Server::operator=(const Server &other) {
	if (this != &other) {
		serverSocket_		 = other.serverSocket_;
		pollFds_			 = other.pollFds_;
		clients_			 = other.clients_;
		channels_			 = other.channels_;
		messageQueueManager_ = other.messageQueueManager_;
		pendingCloseClients_ = other.pendingCloseClients_;
	}
	return *this;
}

const char	*Server::getTimeCreatedHumanReadable() const
{
	char * humanTime = std::ctime(&timeCreated_);
	size_t i = 0;
	while (humanTime[i] != '\0' && humanTime[i] != '\n')
		++i;
	if (humanTime[i] == '\n')
		humanTime[i] = '\0';
	return (humanTime);
}

const std::string	&Server::getName( void ) const
{
	return (name_);
}

const std::string	&Server::getPassword( void ) const
{
	return (password_);
}

std::vector<Client>& Server::getClients( void )
{
	return clients_;
}


int	Server::getPort( void ) const
{
	return (port_);
}

int	Server::getServerSocket( void ) const
{
	return this->serverSocket_;
}

void	Server::addPollFd(const int fd, const short events, const short revents)
{
	struct pollfd newPollfd = {fd, events, revents};
	pollFds_.push_back(newPollfd);
}

void	Server::setServerSocket( int serverSocketFd )
{
	this->serverSocket_ = serverSocketFd;
}


// accepts a connection from client and adds it to pollFds_
void	Server::acceptConnection( void )
{
	while (true) {
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int clientFd = accept(getServerSocket(), (sockaddr *)&client_addr, &client_len);
		if (clientFd == -1) {
			if (errno == EINTR)
				continue; // retry accept
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // no more incoming connections right now
			// error: log and stop trying for this cycle
			debug(std::string("[Server] accept error: ") + strerror(errno));
			break;
		}

		addPollFd(clientFd, POLLIN, 0);
		debug("[Server] accepted new connection");
		Client newcomer(messageQueueManager_);
		newcomer.setSocket(clientFd);
		newcomer.setIP(inet_ntoa(client_addr.sin_addr));
		clients_.push_back(newcomer);
	}
}

Message	Server::buildErrorMessage(MessageType type, std::vector<std::string> messageParams) const
{
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
	IrcErrorInfo info = ErrorMap.find(type)->second;
	messageParams.push_back(info.message);
	Message message(info.code, messageParams);
	return (message);
}

void	Server::quitClient(const Client &quitter)
{
	Message msg("QUIT", "Quit", quitter);
	quitClient(quitter, msg);
}

void	Server::quitClient(const Client &quitter,  const std::string &reason)
{
	Message msg("QUIT", "Quit", reason, quitter);
	quitClient(quitter, msg);
}

// used to removeClient from server (in poll and clients) and broadcast the Quit
// message. if server needs to diconnect the client, modify messageParams to
// reflect reason
// void Server::quitClient(const Client			 &quitter,
// 						std::vector<std::string> &messageParams) {
void	Server::quitClient(const Client &quitter,  const Message &msg)
{
	std::string qNickname = quitter.getNickname();
	// Defer the actual close to allow queued data to flush
	schedulePendingClose(quitter.getSocket());
	for (std::map<std::string, Channel>::iterator cMapIter = channels_.begin();
		 cMapIter != channels_.end(); ++cMapIter) {
		Channel &quittersChannel = cMapIter->second;
		if (!quittersChannel.isMember(qNickname))
			continue;
		quittersChannel.broadcastMsg(qNickname, msg);
		quittersChannel.removeMember(qNickname);
	}
}

// closes and delets an elements from pollIndex_
// the entry in pollFds_ corresponds to the same index -1 in clients_ for that
// particular client. they should have the same fd.
void Server::removeClient(int fd) {
	debug("removing Client");
	std::cout << "[Server] Client on fd " << fd << " has disconnected."
			  << std::endl;
	// Ensure it's no longer scheduled for deferred close
	// Remove from pendingCloseClients_ if present
	bool wasPending = false;
	for (std::vector<Client>::iterator it = pendingCloseClients_.begin();
		 it != pendingCloseClients_.end(); ++it) {
		if (it->getSocket() == fd) {
			removeAndSwapBack(
				pendingCloseClients_,
				static_cast<size_t>(it - pendingCloseClients_.begin()));
			wasPending = true;
			break;
		}
	}
	// Drop any pending outbound data for this fd via MessageQueueManager
	messageQueueManager_.discard(fd);
	if (close(fd) == -1) {
		// Treat as already closed; continue cleanup non-fatally
		debug(std::string("close failed on client fd ") + toString(fd) +
			  ", treating as already closed");
	}
	int cIdx = clientIndexFromFd(fd);
	if (cIdx != -1) {
		removeAndSwapBack(clients_, cIdx);
	} else if (!wasPending) {
		// Only warn when not previously scheduled for pending close
		debug("client list out of sync; could not find fd to remove");
	}
	int pfdIdx = pollFdIndexFromFd(fd);
	if (pfdIdx != -1) {
		removeAndSwapBack(pollFds_, pfdIdx);
	} else {
		debug("pollFds_ list out of sync; could not find fd to remove");
	}
}

void Server::executeIncomingCommandMessage(Client& sender, const std::string& rawMessage)
{
	if (rawMessage.empty())
		return;
	Message message(rawMessage);
	debug("Parsed message: " + message.getType() + " with params: " + toString(message.getParams().size()));
	Command* cmd = convertMessageToCommand(message);
	cmd->execute(*this, sender);
	delete cmd;
}

void Server::broadcastErrorMessage(MessageType				 type,
								   std::vector<std::string> &args) {
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
	IrcErrorInfo info = ErrorMap.find(type)->second;
	args.push_back(info.message);
	Message outMessage(info.code, args);
	broadcastMsg(outMessage);
}

void Server::broadcastErrorMessage(MessageType type, std::string args[],
								   int size) {
	std::vector<std::string>				   outParams(args, args + size);
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
	IrcErrorInfo info = ErrorMap.find(type)->second;
	outParams.push_back(info.message);
	Message outMessage(info.code, outParams);
	broadcastMsg(outMessage);
}

void Server::broadcastMsg(const Message &message) {
	const std::string wire = message.toString();
	for (std::vector<Client>::const_iterator it = clients_.begin();
		 it != clients_.end(); ++it) {
		messageQueueManager_.send(it->getSocket(), wire);
	}
}

bool	Server::clientNickExists(CaseMappedString& toCheck)
{
	for (size_t clientIndex = 0; clientIndex < clients_.size(); clientIndex++)
	{
		Client	compareTo = clients_[clientIndex];
		if (toCheck == compareTo.getNickname())
			return (1);
	}
	return (0);
}

//attempts to extract a full message from the clients sent input
//if it extraced a string, calls the parser and executes the command
void	Server::makeMessage(Client &client)
{
	std::string	command;
	std::string	raw_message = client.getRawMessage();
	size_t		position;

	while ((position = raw_message.find('\n')) != raw_message.npos)
	{
		if (position != 0 && raw_message[position - 1] == '\r')
			command = raw_message.substr(0, position - 1);
		else
			command = raw_message.substr(0, position);
		raw_message.erase(0, position + 1);
		client.setRawMessage(raw_message);
		std::cout << command << std::endl;
		executeIncomingCommandMessage(client, command);
		debug(raw_message);
	}
}

// interpret the message and execute it
void Server::processPollIn(struct pollfd request) {
	char message[BUFSIZ];
	int	 bytesRead;

	bytesRead = recv(request.fd, message, BUFSIZ, MSG_DONTWAIT);
	if (bytesRead == 0)
		schedulePendingClose(request.fd);
	else if (bytesRead == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		throw std::runtime_error("[Server] recv error");
	} else {
		std::cout << CYN << "[received a message from client: " << request.fd
				  << " ]" << RESET << std::endl;
		int cidx = clientIndexFromFd(request.fd);
		if (cidx >= 0) {
			clients_[static_cast<size_t>(cidx)].appendRawMessage(message,
																 bytesRead);
			makeMessage(clients_[static_cast<size_t>(cidx)]);
		} // else Client may have been moved to closing; ignore input
	}
}

void Server::handlePollIn(const std::vector<struct pollfd> &polled) {
	for (std::vector<struct pollfd>::const_iterator it = polled.begin() + 1;
		 it != polled.end(); ++it) {
		if (it->revents & (POLLHUP | POLLERR | POLLNVAL)) {
			std::string msg = "Socket error";
			Client	   *c	= tryClientFromFd(it->fd);
			if (c) {
				// Notify peers with a QUIT, then force immediate removal
				quitClient(*c, msg);
			}
			removeClient(it->fd);
			continue;
		}
		if (!(it->revents & POLLIN))
			continue;
		// If this fd is already scheduled for close, ignore any POLLIN
		if (isPendingCloseFd(it->fd)) {
			continue;
		}
		try {
		processPollIn(*it);
		} catch (std::runtime_error &e) {
			// TODO: send gerenic error reply to client
			std::cerr << "[Server] " << e.what() << ": " << strerror(errno) << std::endl;
		}
	}
}

// pollfd/socket with index 0 is the listening socket that accepts new
// connections so we only check that one here
void Server::handleNewConnection(const struct pollfd &polled) {
	if (polled.revents & (POLLHUP | POLLERR | POLLNVAL)) {
		debug("[Server] listening socket hangup/error");
		running_ = false;
	} else if (polled.revents & POLLIN) {
		acceptConnection();
	}
}

// main loop to be in while running
// checks if any fd is ready and prints waiting messages inbetween
// if ready, checks polls through and directs them toward acceptConnection or
// readFromSocket
void Server::waitForRequests(void) {
	try {
		while (running_) {
			std::vector<struct pollfd> polled = pollFds_;
			messageQueueManager_.mergePollfds(polled);
			// polled vec is structurally the same as pollFds_ but with added
			// POLLOUT events for the fd's that have a non-empty queue, thus it
			// is safe to use the same indexes for both the polled array and the
			// pollFds_ array
			int rdyPollsCount = poll(&(polled[0]), polled.size(), TIMEOUT);
			if (running_ && rdyPollsCount == -1)
				throw std::runtime_error("[Server] poll error");
			else if (rdyPollsCount == 0) {
				continue;
			}
			// Before draining, check if any pending-close fds are ready to be
			// closed
			processPendingCloses(polled);
			messageQueueManager_.drainQueuesForPolled(polled);
			handlePollIn(polled);
			handleDeadFds();
			handleNewConnection(polled[0]);
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << ": " << strerror(errno) << std::endl;
		// serverShutdown();
	}
	std::cout << YEL << "[Server] Stopped listening for requests" << RESET
			  << std::endl;
}

void Server::handleDeadFds() {
	if (!messageQueueManager_.hasDeadFds())
		return;
	std::vector<int> deadFds = messageQueueManager_.takeDeadFds();
	for (std::vector<int>::iterator it = deadFds.begin(); it != deadFds.end();
		 ++it) {
		std::string msg = "Socket error or backlog overflow";
		Client	   *c	= tryClientFromFd(*it);
		// Immediate removal on fatal send error
		if (c) {
			quitClient(*c, msg);
		}
		removeClient(*it);
	}
}

void Server::schedulePendingClose(int fd) {
	// If already scheduled, nothing to do
	if (isPendingCloseFd(fd))
		return;
	// Remove POLLIN, ensure POLLOUT is monitored
	int pidx = pollFdIndexFromFd(fd);
	if (pidx != -1) {
		short &ev = pollFds_[pidx].events;
		ev		  = static_cast<short>((ev & ~POLLIN) | POLLOUT);
	}
	// Move the Client object to closingClients_ and remove from channels
	int cidx = clientIndexFromFd(fd);
	if (cidx != -1) {
		Client		dying	 = clients_[static_cast<size_t>(cidx)];
		std::string nickname = dying.getNickname();
		pendingCloseClients_.push_back(dying);
		removeAndSwapBack(clients_, static_cast<size_t>(cidx));
		for (std::map<std::string, Channel>::iterator cMapIter =
				 channels_.begin();
			 cMapIter != channels_.end(); ++cMapIter) {
			Channel &ch = cMapIter->second;
			if (ch.isMember(nickname))
				ch.removeMember(nickname);
		}
	}
}

void Server::processPendingCloses(const std::vector<struct pollfd> &polled) {
	if (pendingCloseClients_.empty())
		return;

	// Build a quick lookup of fds that reported POLLOUT this cycle so we don't
	// rescan the polled vector for every pending client.
	std::set<int> polloutReadyFds;
	for (std::vector<struct pollfd>::const_iterator pit = polled.begin();
		 pit != polled.end(); ++pit) {
		if (pit->revents & POLLOUT)
			polloutReadyFds.insert(pit->fd);
	}

	// Collect fds that are ready to close. We avoid mutating the
	// pendingCloseClients_ vector while iterating it which was forcing us to
	// restart the loop previously.
	std::vector<int> readyToClose;
	readyToClose.reserve(pendingCloseClients_.size());
	for (std::vector<Client>::const_iterator it = pendingCloseClients_.begin();
		 it != pendingCloseClients_.end(); ++it) {
		int fd = it->getSocket();
		// Only eligible when there's no remaining outbound backlog AND the fd
		// produced a POLLOUT event this cycle (meaning kernel send buffer became
		// writable and we now know all queued data was flushed).
		if (messageQueueManager_.hasBacklog(fd))
			continue;
		if (polloutReadyFds.find(fd) == polloutReadyFds.end())
			continue;
		readyToClose.push_back(fd);
	}

	for (std::vector<int>::const_iterator it = readyToClose.begin();
		 it != readyToClose.end(); ++it) {
		removeClient(*it); // also erases from pendingCloseClients_
		debug("closed pending-close client " + toString(*it));
	}
}

bool Server::isPendingCloseFd(int fd) const {
	for (std::vector<Client>::const_iterator it = pendingCloseClients_.begin();
		 it != pendingCloseClients_.end(); ++it) {
		if (it->getSocket() == fd)
			return true;
	}
	return false;
}

// creates the one listening socket the server has to start out with
void Server::createListeningSocket(void) {
	int				 err   = 0;
	struct addrinfo	 hints = {0, 0, 0, 0, 0, 0, 0, 0};
	struct addrinfo *res   = {0};
	hints.ai_family		   = AF_UNSPEC;	  // IPv4 or IPv6
	hints.ai_socktype	   = SOCK_STREAM; // TCP
	hints.ai_flags		   = AI_PASSIVE;  // put in my ip for me
	std::string port_str   = toString(port_);
	int			optval	   = 1;

	err = (getaddrinfo(NULL, port_str.c_str(), &hints, &res));
	if (err != 0)
	{
		freeaddrinfo(res);
		throw std::runtime_error(std::string(gai_strerror(err)));
	}
	//create Socket
	serverSocket_ = socket(res->ai_family, res->ai_socktype | SOCK_NONBLOCK, res->ai_protocol);
	if (serverSocket_ == -1)
	{
		freeaddrinfo(res);
		throw std::runtime_error("[Server] socket error");
	}
	//reusing old socket, if still open, to circumvent TIME_WAIT
	setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	//bind socket
	if (-1 == bind(getServerSocket(), res->ai_addr, res->ai_addrlen))
	{
		close(serverSocket_);
		freeaddrinfo(res);
		throw std::runtime_error("[Server] bind error");
	}
	freeaddrinfo(res);
	// mark as passive socket listening to incoming connections from clients
	if (-1 == listen(getServerSocket(), BACKLOG))
		throw std::runtime_error("[Server] listen error");
}


// makes first listeing socket and adds it to pollFds_
void	Server::serverInit(void)
{
	pollFds_.reserve(5);

	this->createListeningSocket();
	this->addPollFd(getServerSocket(), POLLIN, 0);
}

// cleanup
void Server::serverShutdown(void) {
	std::cout << BRED << "==== STARTING SERVER SHUTDOWN ====" << RESET
			  << std::endl;
	for (size_t pollIndex = 1; pollIndex < pollFds_.size(); pollIndex++) {
		if (-1 == close(pollFds_[pollIndex].fd)) {
			debug(std::string("close failed on fd ") +
				  toString(pollFds_[pollIndex].fd) +
				  ", treating as already closed");
		}
	}
	std::cout << "[Server] diconnected all clients sockets" << RESET
			  << std::endl;
	if (serverSocket_ != -1) {
		if (-1 == close(serverSocket_)) {
			debug("close failed on serverSocket; treating as already closed");
		}
		std::cout << "[Server] diconnected listening socket" << RESET
				  << std::endl;
	}
	std::cout << GREEN << "[Server] Shutdown complete" << RESET << std::endl;
}

Channel* Server::mapChannel(const std::string& channelName)
{
	std::map<std::string, Channel>::iterator it = channels_.find(channelName);
	if (it != channels_.end())
		return &(it->second);
	return NULL;
}

std::map<std::string, Channel>&	Server::getChannels(void)
{
	return (channels_);
}

// Accessor for outbound message queue manager
MessageQueueManager &Server::getMessageQueueManager() {
	return messageQueueManager_;
}

int Server::pollFdIndexFromFd(int fd) const {
	for (size_t i = 0; i < pollFds_.size(); ++i) {
		if (pollFds_[i].fd == fd)
			return static_cast<int>(i);
	}
	return -1;
}

int Server::clientIndexFromFd(int fd) const {
	int pidx = pollFdIndexFromFd(fd);
	if (pidx > 0 && static_cast<size_t>(pidx) < pollFds_.size()) {
		size_t cidx = static_cast<size_t>(pidx - 1);
		if (cidx < clients_.size() &&
			clients_[cidx].getSocket() == pollFds_[pidx].fd)
			return static_cast<int>(cidx);
		// fall through to linear scan if mismatch
	}
	// Fallback: linear scan of clients_ by socket fd
	for (size_t i = 0; i < clients_.size(); ++i) {
		if (clients_[i].getSocket() == fd)
			return static_cast<int>(i);
	}
	return -1;
}

Client *Server::tryClientFromFd(int fd) {
	int idx = clientIndexFromFd(fd);
	return (idx >= 0) ? &clients_[static_cast<size_t>(idx)] : NULL;
}
