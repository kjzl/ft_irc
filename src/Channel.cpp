#include "../include/Channel.hpp"

Channel::Channel()
    : topic_(""), password_(""), user_limit_(0)
{}

Channel::Channel(const Channel &other)
{
    *this = other;
}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        this->members_ = other.members_;
        this->whiteList_ = other.whiteList_;
        this->operators_ = other.operators_;
        this->topic_ = other.topic_;
        this->password_ = other.password_;
        this->user_limit_ = other.user_limit_;
    }
    return *this;
}

Channel::~Channel()
{}

// Getters
const std::vector<std::string> &Channel::getMembers() const
{
    return members_;
}

const std::set<std::string> &Channel::getWhiteList() const
{
    return whiteList_;
}

const std::set<std::string> &Channel::getOperators() const
{
    return operators_;
}

const std::string &Channel::getTopic() const
{
    return topic_;
}

const std::string &Channel::getPassword() const
{
    return password_;
}

int Channel::getUserLimit() const
{
    return user_limit_;
}

// Setters
void Channel::setTopic(const std::string &topic)
{
    topic_ = topic;
}

void Channel::setPassword(const std::string &password)
{
    password_ = password;
}

void Channel::setUserLimit(int limit)
{
    user_limit_ = limit;
}
