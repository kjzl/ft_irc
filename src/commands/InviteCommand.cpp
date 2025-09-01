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
		return (sender.sendErrorMessage(ERR_NOTREGISTERED, sender.getNickname()));
	// 461
	if (inParams.size() < 2)
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
	
	std::string	channelName = inParams[1];
	std::string	invitedClient = inParams[0];
	Channel *channel = server.mapChannel(channelName);
	// 403
	if (!channel)
		return (sender.sendErrorMessage(ERR_NOSUCHCHANNEL, sender.getNickname(), channelName));
	// 442
	if (!channel->isMember(sender.getNickname()))
		return (sender.sendErrorMessage(ERR_NOTONCHANNEL, sender.getNickname(), channelName));
	// ERR_CHANOPRIVSNEEDED (482)
	if (channel->isInviteOnly()	&& !channel->isOperator(sender.getNickname()))
		return (sender.sendErrorMessage(ERR_CHANOPRIVSNEEDED, sender.getNickname(), channelName));
	// ERR_USERONCHANNEL (443)
	if (channel->isMember(invitedClient))
		return (sender.sendErrorMessage(ERR_USERONCHANNEL, sender.getNickname(), invitedClient, channelName));
	// ERR_NOSUCHNICK (401)
	CaseMappedString tmp(invitedClient);
	if (!server.clientNickExists(tmp))
		return (sender.sendErrorMessage(ERR_NOSUCHNICK, sender.getNickname(), invitedClient));
	// ===> Success :)
	channel->addToWhiteList(invitedClient);
	// RPL_INVITING (341)
	sender.sendErrorMessage(RPL_INVITING, sender.getNickname(), invitedClient, channelName);
	// sending the invitation !
	sender.sendMessageTo(inMessage_, invitedClient, server);
}

