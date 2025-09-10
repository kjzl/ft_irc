#include "Channel.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include "MessageQueueManager.hpp"

Channel::Channel(const std::string &name, const Client &op,
				 MessageQueueManager &queueManager)
	: mqr_(queueManager), name_(name), members_(), whiteList_(), operators_(),
	  topic_(""), password_(""), userLimit_(0), isInviteOnly_(false),
	  isTopicProtected_(false) {
	members_[op.getNickname()] = op.getSocket();
	operators_.insert(op.getNickname());
}

Channel::Channel(const Channel &other) : mqr_(other.mqr_) { *this = other; }

// This can't change the MessageQueueManager reference stored inside
Channel &Channel::operator=(const Channel &other) {
	if (this != &other) {
		this->name_				= other.name_;
		this->members_			= other.members_;
		this->whiteList_		= other.whiteList_;
		this->operators_		= other.operators_;
		this->topic_			= other.topic_;
		this->password_			= other.password_;
		this->userLimit_		= other.userLimit_;
		this->isInviteOnly_		= other.isInviteOnly_;
		this->isTopicProtected_	= other.isTopicProtected_;
	}
	return *this;
}

Channel::~Channel()
{}

// Getters
const	std::string &Channel::getName() const
{
	return name_;
}

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

const time_t &Channel::getCreationTime() const
{
	return creationTime_;
}

const std::string &Channel::getTopicWho() const
{
	return topicWho_;
}

const time_t &Channel::getTopicTime() const
{
	return topicTime_;
}
// Setters
void Channel::setTopic(const std::string &topic)
{
	topic_ = topic;
}

void Channel::setTopicWho(const std::string &topicWho)
{
	topicWho_ = topicWho;
}

void Channel::setTopicTime()
{
	topicTime_ = time(NULL);
}

void Channel::setPassword(const std::string &password)
{
	password_ = password;
}

void Channel::setUserLimit(int limit)
{
	userLimit_ = limit;
}

void Channel::broadcastMsg(const std::string &senderNickname,
						   const Message	 &message) const {
	const std::string wire = message.toString();
	for (std::map<std::string, int>::const_iterator memberIt = members_.begin();
		 memberIt != members_.end(); ++memberIt) {
		if (memberIt->first == senderNickname)
			continue;
		mqr_.send(memberIt->second, wire);
	}
}

void Channel::broadcastMsg(const Client &sender, const Message &message) const {
	broadcastMsg(sender.getNickname(), message);
}

bool Channel::checkKey(const std::string& key) const
{
	if (password_.empty())
		return true;
	return key == password_;
}

bool Channel::isInviteOnly() const
{
	return (isInviteOnly_);
}

bool Channel::isTopicProtected() const
{
	return (isTopicProtected_);
}

void Channel::addToWhiteList(const std::string &nickname)
{
	whiteList_.insert(nickname);
}

void Channel::removeFromWhiteList(const std::string &nickname)
{
	whiteList_.erase(nickname);
}

bool Channel::isWhiteListed(const std::string &nickname) const
{
	return whiteList_.find(nickname) != whiteList_.end();
}

void Channel::addMember(const Client* client)
{
	const std::string nickname = client->getNickname();
	for (std::map<std::string, int>::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if (it->first == nickname)
			return;
	}
	members_[nickname] = client->getSocket();
}

void Channel::removeMember(const std::string &nickname)
{
	std::map<std::string, int>::iterator foundMemberIt = members_.find(nickname);
	if (foundMemberIt != members_.end())
		members_.erase(foundMemberIt);
}

bool Channel::isMember(const std::string &nickname) const
{
	if (members_.find(nickname) != members_.end())
		return (true);
	return (false);
}

void	Channel::addOperator(const std::string &nickname)
{
	operators_.insert(nickname);
}

void	Channel::removeOperator(const std::string &nickname)
{
	operators_.erase(nickname);
}

bool	Channel::isOperator(const std::string &nickname) const
{
	if (operators_.find(nickname) != operators_.end())
		return (true);
	return (false);
}

void	Channel::setInviteOnly(bool value)
{
	isInviteOnly_ = value;
}

void	Channel::setTopicProtected(bool value)
{
	isTopicProtected_ = value;
}
