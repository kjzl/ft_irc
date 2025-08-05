#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/MessageType.hpp"
#include <cstdio>
#include <stdexcept>
#include <sys/socket.h>
#include "Message.hpp"


Client::Client() : registrationLevel_(0), socket_(-1), nickname_(""), username_("*"), realname_(""), rawMessage_("")
{}

Client::Client(const Client &other)
{
    *this = other;
}

Client &Client::operator=(const Client &other)
{
    if (this != &other) {
		clearMessage();
        this->registrationLevel_ = other.registrationLevel_;
        this->nickname_ = other.nickname_;
        this->username_ = other.username_;
        this->realname_ = other.realname_;
		this->rawMessage_ = other.rawMessage_;
        this->socket_ = other.socket_;
    }
    return *this;
}

Client::~Client()
{}

// Getters

bool Client::isAuthenticated() const
{
    return (registrationLevel_ == 3);
}

const std::string &Client::getNickname() const
{
    return nickname_;
}

const std::string &Client::getUsername() const
{
    return username_;
}

const std::string &Client::getRealname() const
{
    return realname_;
}

int Client::getSocket() const
{
    return socket_;
}

const std::string &Client::getRawMessage() const
{
    return rawMessage_;
}

void Client::clearMessage()
{
	rawMessage_.clear();
}


void Client::incrementRegistrationLevel(void)
{
    ++registrationLevel_;
}

int Client::getRegistrationLevel(void) const
{
    return (registrationLevel_);
}

// Setters
void Client::setNickname(const std::string &nickname)
{
    nickname_ = nickname;
}

void Client::setUsername(const std::string &username)
{
    username_ = username;
}

void Client::setRealname(const std::string &realname)
{
    realname_ = realname;
}

void Client::setSocket(int socket)
{
    socket_ = socket;
}


void Client::setRawMessage(const std::string &rawMessage)
{
    rawMessage_ = rawMessage;
}

void Client::appendRawMessage(const char partialMessage[BUFSIZ], size_t length)
{
	rawMessage_ += std::string(partialMessage, length);
}


void	Client::sendMessage(Message toSend)
{
	safeSend(toSend.toString());
}

void Client::sendErrorMessage(MessageType type, const Server& server, std::vector<std::string>& args)
{
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
    IrcErrorInfo info = ErrorMap.find(type)->second;
    args.push_back(info.message);
    Message outMessage(info.code, server.getName(), args);
    sendMessage(outMessage);
}

// sends the entire string with send() even when more than one send() call is needed
// throws and error if send fails
int		Client::safeSend(const std::string &string)
{
	int sendBytes;
	int	total_sent = 0;
	int	left_size = string.size();

	while (left_size)
	{
		sendBytes = send(this->getSocket(), string.substr(total_sent, left_size).c_str(), left_size, 0);
		if (sendBytes == -1)
			throw std::runtime_error("[Server] send error with client: X"); //TODO:  + toString(getSocket()));
		total_sent += sendBytes;
		left_size -= sendBytes;
	}
	return (0);
}

