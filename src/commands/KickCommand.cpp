#include "KickCommand.hpp"
#include "Debug.hpp"
#include <sstream>
// Default Constructor
KickCommand::KickCommand( void ): Command()
{
	debug("Default Constructor called");
}

KickCommand::KickCommand(const Message& msg) : Command(msg)
{}

// Destructor
KickCommand::~KickCommand()
{
	debug("Destructor called");
}

// Copy Constructor
KickCommand::KickCommand(const KickCommand &copy): Command(copy)
{}

// Copy Assignment Operator
KickCommand& KickCommand::operator=( const KickCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	KickCommand::fromMessage(const Message& message)
{
	return new KickCommand(message);
}

/*
https://modern.ircdocs.horse/#kick-message
ERR_NEEDMOREPARAMS (461)	=> done
ERR_NOSUCHCHANNEL (403)		=> done
ERR_CHANOPRIVSNEEDED (482)	=> done
ERR_USERNOTINCHANNEL (441)	=> done
ERR_NOTONCHANNEL (442)		=> done
*/
void	KickCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();

	// 451
	if (!sender.isAuthenticated())
	{
		std::string params[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_NOTREGISTERED, params, 1));
	}
	// 461
	if (inParams.size() < 2)
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
	// ERR_CHANOPRIVSNEEDED (482)
	if (!channel->isOperator(sender.getNickname()))
	{
		std::string arr[] = {sender.getNickname(), channelName};
		return (sender.sendErrorMessage(ERR_CHANOPRIVSNEEDED, arr, 2));
	}
	std::stringstream targetClientStream(inParams[1]);
	while (std::getline(targetClientStream, targetClient, ','))
	{
		// ERR_USERNOTINCHANNEL (441)
		if (! channel->isMember(targetClient))
		{
			std::string arr[] = {sender.getNickname(), targetClient, channelName};
			return (sender.sendErrorMessage(ERR_USERNOTINCHANNEL, arr, 3));
		}
		// ===> Success :)
		channel->removeMember(targetClient);
		channel->removeFromWhiteList(targetClient);
		channel->removeOperator(targetClient);
		// sending the KICK to the target
		inMessage_.getParams()[1] = targetClient;
		if (inMessage_.getParams().size() == 2)
			inMessage_.getParams().push_back(":You have been kicked out (No reason provided)");
		sender.sendMessageTo(inMessage_, targetClient, server);
	}
}

