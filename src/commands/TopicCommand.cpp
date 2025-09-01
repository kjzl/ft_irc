#include "TopicCommand.hpp"
#include "Debug.hpp"
// Default Constructor
TopicCommand::TopicCommand( void ): Command()
{
	debug("Default Constructor called");
}

TopicCommand::TopicCommand(const Message& msg) : Command(msg)
{}

// Destructor
TopicCommand::~TopicCommand()
{
	debug("Destructor called");
}

// Copy Constructor
TopicCommand::TopicCommand(const TopicCommand &copy): Command(copy)
{}

// Copy Assignment Operator
TopicCommand& TopicCommand::operator=( const TopicCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	TopicCommand::fromMessage(const Message& message)
{
	return new TopicCommand(message);
}

/*
https://modern.ircdocs.horse/#topic-message
    ERR_NEEDMOREPARAMS (461)	=> done
    ERR_NOSUCHCHANNEL (403)		=> done
    ERR_NOTONCHANNEL (442)		=> done
    ERR_CHANOPRIVSNEEDED (482)	=> done
    RPL_NOTOPIC (331)			=> done
    RPL_TOPIC (332)				=> done
    RPL_TOPICWHOTIME (333)		=> NO : we dont hav history in our server !!

*/
void	TopicCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();

	// 451
	if (!sender.isAuthenticated())
		return (sender.sendErrorMessage(ERR_NOTREGISTERED, sender.getNickname()));
	// 461
	if (inParams.size() < 1)
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));

	std::string	channelName = inParams[0];
	std::string	targetClient;
	Channel *channel = server.mapChannel(channelName);
	// 403
	if (!channel)
		return (sender.sendErrorMessage(ERR_NOSUCHCHANNEL, sender.getNickname(), channelName));
	// 442
	if (!channel->isMember(sender.getNickname()))
		return (sender.sendErrorMessage(ERR_NOTONCHANNEL, sender.getNickname(), channelName));
	if (inParams.size() == 1)	// client wants to get the channel topic
	{
		// RPL_NOTOPIC (331)
		if (!channel->getTopic().size())
			return (sender.sendErrorMessage(RPL_NOTOPIC, sender.getNickname(), channelName));
		// RPL_TOPIC (332) + TODO: REPLYTOPICWHOTIME
		return (sender.sendErrorMessage(RPL_TOPIC, sender.getNickname(), channelName, channel->getTopic()));
	}
	else	// client wants to set the topic
	{
		// ERR_CHANOPRIVSNEEDED (482)
		if (channel->isTopicProtected() && !channel->isOperator(sender.getNickname()))
			return (sender.sendErrorMessage(ERR_CHANOPRIVSNEEDED, sender.getNickname(), channelName));
		channel->setTopic(inParams[1]);
		// TODO: send confirmation to channel
	}
}

