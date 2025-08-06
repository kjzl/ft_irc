#include "../../include/PrivmsgCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"
#include "Channel.hpp"
#include <cstdlib>
#include <iostream>

PrivmsgCommand::PrivmsgCommand(const Message& msg) : Command(msg)
{}

Command* PrivmsgCommand::fromMessage(const Message& message)
{
	return new PrivmsgCommand(message);
}

void	PrivmsgCommand::privmsgRecipient(std::string recipient, const Message& message, Server& server, Client& sender)
{
	bool	messageSentSuccessfully = false;
	if (recipient[0] == '#')
	{
		Channel *recipientChannel = server.mapChannel(recipient); 
		if (recipientChannel)
		{
			messageSentSuccessfully = true;
			recipientChannel->broadcastMsg(sender, message);
		}
		// {
		// 	// ERR_CANNOTSENDTOCHAN = 404, (not enough rights)
		// 	std::string arr[] = {sender.getNickname(), recipient};
		// 	return (sender.sendErrorMessage(ERR_CANNOTSENDTOCHAN, arr, 1));
		// }
	}
	else
		 messageSentSuccessfully = (sender.sendMessageTo(message, recipient, server));
	if (!messageSentSuccessfully)
	{
		std::string arr[] = {sender.getNickname(), recipient};
		return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 1));
	}
}

/*
    https://modern.ircdocs.horse/#privmsg-message
	ERR_NOSUCHNICK = 401, x
	ERR_CANNOTSENDTOCHAN = 404, TODO:
	ERR_NORECIPIENT = 411, x
	ERR_NOTEXTTOSEND = 412, x
	RPL_AWAY = 301 // not doing that one anymore, doesn't make sense, as we don't register users

*/
void PrivmsgCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();
	
	// Not Authenticated ==> ignore it...
	if (!sender.isAuthenticated())
		return;
	Message outMessage = inMessage_;
	outMessage.setSource(sender.getNickname(), sender.getUsername());
	debug("message to send: " + outMessage);
	if (inParams[0].empty())
	{
		std::string arr[] = {sender.getNickname(), };
		return (sender.sendErrorMessage(ERR_NORECIPIENT, arr, 1));
	}
	if (inParams.size() < 2)
	{
		std::string arr[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_NOTEXTTOSEND, arr, 1));
	}

	std::stringstream stream(inParams[0]);
	std::string recipient;
	while (std::getline(stream, recipient, ','))
	{
		privmsgRecipient(recipient, outMessage, server, sender);
	}
	return;
}
