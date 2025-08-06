#include "../include/Channel.hpp"
#include <vector>
#include "Client.hpp"
#include "Message.hpp"

Channel::Channel()
	: topic_(""), password_(""), userLimit_(0)
{}

// Channel::Channel(std::vector<const Client*> members, std::set<std::string> whiteList, std::set<std::string> operators, std::string topic, std::string password, int userLimit)
// 	:	members_(members),
// 		whiteList_(whiteList),
// 		operators_(operators),
// 		topic_(topic),
// 		password_(password),
// 		userLimit_(userLimit)
// {}
// constructor for JOIN in order to create a new channel :
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
    members_.push_back(&sender);
    operators_.insert(&sender);
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
const std::vector<const Client*> &Channel::getMembers() const
{
	return members_;
}

const std::set<const Client*> &Channel::getWhiteList() const
{
	return whiteList_;
}

const std::set<const Client*> &Channel::getOperators() const
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

// // sends a message to members by fd
// // sender can be -1 to send to all Clients in server, used for server replies
// void	Channel::broadcastMsg(const Client &sender, const Message &message)
// {
// 	for (std::vector<int>::iterator	memberIt = (members_.begin()); memberIt != members_.end(); memberIt++)
// 	{
// 		int	senderFd = sender.getSocket();
// 		if (senderFd != -1 && *memberIt != senderFd)
// 			sender.sendMessageToFd( message, *memberIt);
// 	}
// }

void Channel::broadcastMsg(const Client &sender, const Message &message)
{
	for (std::vector<const Client*>::iterator	memberIt = (members_.begin()); memberIt != members_.end(); memberIt++)
	{
		if (*memberIt != &sender)
			(*memberIt)->sendMessage(message);
	}
}

bool Channel::checkKey(const std::string& key) const
{
	if (password_.empty())
		return true;
	return key == password_;
}

bool Channel::isInviteOnly() const
{
	return isInviteOnly_;
}

bool Channel::isWhiteListed(const Client& sender) const
{
	return whiteList_.find(&sender) != whiteList_.end();
}

void Channel::addMember(const Client* client)
{
	for (std::vector<const Client*>::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if (*it == client)
			return;
	}
	members_.push_back(client);
}
