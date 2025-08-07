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
	{
		std::string params[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_NOTREGISTERED, params, 1));
	}
	// 461
	if (inParams.size() < 1)
	{
		std::string arr[] = {sender.getNickname(), inMessage_.getType()};
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, arr, 2));
	}
	std::string	channelName = inParams[0];
	std::string	targetClient;
	Channel *channel = server.mapChannel(channelName);
	// 403
	if (!channel)
	{
		std::string arr[] = {sender.getNickname(), channelName};
		return (sender.sendErrorMessage(ERR_NOSUCHCHANNEL, arr, 2));
	}
	// 442
	if (!channel->isMember(sender.getNickname()))
	{
		std::string arr[] = {sender.getNickname(), channelName};
		return (sender.sendErrorMessage(ERR_NOTONCHANNEL, arr, 2));
	}
	if (inParams.size() == 1)	// client wants to get the channel topic
	{
		// RPL_NOTOPIC (331)
		if (!channel->getTopic().size())
		{
			std::string arr[] = {sender.getNickname(), channelName};
			return (sender.sendErrorMessage(RPL_NOTOPIC, arr, 2));
		}
		// RPL_TOPIC (332)
		std::string arr[] = {sender.getNickname(), channelName, channel->getTopic()};
		return (sender.sendErrorMessage(RPL_TOPIC, arr, 3));
	}
	else	// client wants to set the topic
	{
		// ERR_CHANOPRIVSNEEDED (482)
		if (!channel->isOperator(sender.getNickname()))
		{
			std::string arr[] = {sender.getNickname(), channelName};
			return (sender.sendErrorMessage(ERR_CHANOPRIVSNEEDED, arr, 2));
		}
		channel->setTopic(inParams[1]);
	}
}

