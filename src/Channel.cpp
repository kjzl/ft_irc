#include "../include/Channel.hpp"
#include <vector>
#include "Client.hpp"
#include "Message.hpp"

Channel::Channel()
    : topic_(""), password_(""), userLimit_(0)
{}

// Channel::Channel(std::vector<const std::string> members, std::set<std::string> whiteList, std::set<std::string> operators, std::string topic, std::string password, int userLimit)
// 	:	members_(members),
// 		whiteList_(whiteList),
// 		operators_(operators),
// 		topic_(topic),
// 		password_(password),
// 		userLimit_(userLimit)
// {}

Channel::Channel(const std::string& name, const Client& sender)
    : name_(name),
      members_(),        // initialize explicitly or leave it out (default)
      whiteList_(),
      operators_(),
      topic_(""),
      password_(""),
      userLimit_(0),
      isInviteOnly_(false)
{
	members_[sender.getNickname()] = sender.getSocket();
    operators_.insert(sender.getNickname());
}

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
        this->userLimit_ = other.userLimit_;
    }
    return *this;
}

Channel::~Channel()
{}

// Getters
const	std::map<std::string, int> &Channel::getMembers() const
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
    return userLimit_;
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
    userLimit_ = limit;
}

void Channel::broadcastMsg(const Client &sender, const Message &message)
{
	for (std::map<std::string, int>::iterator	memberIt = (members_.begin()); memberIt != members_.end(); memberIt++)
	{
		if (memberIt->first != sender.getNickname())
			sender.sendMessageToFd(message, memberIt->second);
	}
}
