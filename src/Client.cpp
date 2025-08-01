#include "../include/Client.hpp"
#include <cstdio>
#include <iostream>

Client::Client() : is_authenticated_(false), socket_(-1), nickname_(""), username_(""), realname_(""), rawMessage_("")
{}

Client::Client(const Client &other) 
{
    *this = other;
}

Client &Client::operator=(const Client &other)
{
    if (this != &other) {
		clearMessage();
        this->is_authenticated_ = other.is_authenticated_;
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
    return is_authenticated_;
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

// Setters

void Client::setAuthenticated(bool authenticated)
{
    is_authenticated_ = authenticated;
}

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

#include "../include/Debug.hpp"

void Client::appendRawMessage(const char partialMessage[BUFSIZ], size_t length)
{
	std::cerr << BLUE << "appending rawMessage with: '" << std::string(partialMessage, length) << "'" << RESET << std::endl;
	rawMessage_ += std::string(partialMessage, length);
}
