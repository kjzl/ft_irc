#include "../include/Client.hpp"

Client::Client() : is_authenticated_(false), nickname_(""), username_(""), realname_(""), socket_(-1)
{}

Client::Client(const Client &other) 
{
    *this = other;
}

Client &Client::operator=(const Client &other)
{
    if (this != &other) {
        this->is_authenticated_ = other.is_authenticated_;
        this->nickname_ = other.nickname_;
        this->username_ = other.username_;
        this->realname_ = other.realname_;
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
