#include "InviteCommand.hpp"
#include "Debug.hpp"
#include "Message.hpp"
// Default Constructor
InviteCommand::InviteCommand( void ): Command()
{
	debug("Default Constructor called");
}

InviteCommand::InviteCommand(const Message& msg) : Command(msg)
{}

// Destructor
InviteCommand::~InviteCommand()
{
	debug("Destructor called");
}

// Copy Constructor
InviteCommand::InviteCommand(const InviteCommand &copy): Command(copy)
{}

// Copy Assignment Operator
InviteCommand& InviteCommand::operator=( const InviteCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	InviteCommand::fromMessage(const Message& message)
{
	return new InviteCommand(message);
}

/*
https://modern.ircdocs.horse/#invite-message

    RPL_INVITING (341)			=> done
    ERR_NEEDMOREPARAMS (461)	=> done
    ERR_NOSUCHCHANNEL (403)		=> done
    ERR_NOTONCHANNEL (442)		=> done
    ERR_CHANOPRIVSNEEDED (482)	=> done
    ERR_USERONCHANNEL (443)		=> done
	ERR_NOSUCHNICK (401)		=> done
*/
void	InviteCommand::execute(Server& server, Client& sender)
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
	std::string	channelName = inParams[1];
	std::string	invitedClient = inParams[0];
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
	if (channel->isInviteOnly()	&& !channel->isOperator(sender.getNickname()))
	{
		std::string arr[] = {sender.getNickname(), channelName};
		return (sender.sendErrorMessage(ERR_CHANOPRIVSNEEDED, arr, 2));
	}
	// ERR_USERONCHANNEL (443)
	if (channel->isMember(invitedClient))
	{
		std::string arr[] = {sender.getNickname(), invitedClient, channelName};
		return (sender.sendErrorMessage(ERR_USERONCHANNEL, arr, 3));
	}
	// ERR_NOSUCHNICK (401)
	CaseMappedString tmp(invitedClient);
	if (!server.clientNickExists(tmp))
	{
		std::string arr[] = {sender.getNickname(), invitedClient};
		return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 2));
	}
	// ===> Success :)
	channel->addToWhiteList(invitedClient);
	// RPL_INVITING (341)
	std::string arr[] = {sender.getNickname(), invitedClient, channelName};
	sender.sendErrorMessage(RPL_INVITING, arr, 3);
	// sending the invitation !
	Message outMessage(inMessage_);
	outMessage.setSource(sender.getNickname(), sender.getUsername());
	sender.sendMessageTo(outMessage, invitedClient, server);
}

