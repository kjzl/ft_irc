#include "../../include/PrivmsgCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"
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
	if (recipient[0] == '#')
	{
		// for now, all messages to channels fail, as we don't have channels yet
		{
			// ERR_NOSUCHCHANNEL = 403, (:no such channel)
			std::string arr[] = {sender.getNickname(), recipient};
			return (sender.sendErrorMessage(ERR_NOSUCHCHANNEL, arr, 1));
		}
		// {
		// 	// ERR_CANNOTSENDTOCHAN = 404, (not enough rights)
		// 	std::string arr[] = {sender.getNickname(), recipient};
		// 	return (sender.sendErrorMessage(ERR_CANNOTSENDTOCHAN, arr, 1));
		// }
	}
	else
	{
		if (!sender.sendMessageTo(message, recipient, server))
		{
			std::string arr[] = {sender.getNickname(), recipient};
			return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 1));
		}
	}
}

/*
    https://modern.ircdocs.horse/#privmsg-message
	ERR_NOSUCHNICK = 401, x
	ERR_NOSUCHCHANNEL = 403, x
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
	std::cerr << BLUE << "message: '" << outMessage << "'" << RESET << std::endl;
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
		// if (recipient[0] == '#')
		// {
		// 	// for now, all messages to channels fail, as we don't have channels yet
		// 	{
		// 		// ERR_NOSUCHCHANNEL = 403, (:no such channel)
		// 		std::string arr[] = {sender.getNickname(), recipient};
		// 		return (sender.sendErrorMessage(ERR_NOSUCHCHANNEL, arr, 1));
		// 	}
		// 	// {
		// 	// 	// ERR_CANNOTSENDTOCHAN = 404, (not enough rights)
		// 	// 	std::string arr[] = {sender.getNickname(), recipient};
		// 	// 	return (sender.sendErrorMessage(ERR_CANNOTSENDTOCHAN, arr, 1));
		// 	// }
		// }
		// else
		// {
		// 	if (!sender.sendMessageTo(outMessage, recipient, server))
		// 	{
		// 		std::string arr[] = {sender.getNickname(), recipient};
		// 		return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 1));
		// 	}
		// }
	}
	return;
}
