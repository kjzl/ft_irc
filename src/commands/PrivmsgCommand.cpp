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

/*
    https://modern.ircdocs.horse/#privmsg-message
	ERR_NOSUCHNICK = 401,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
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
	// ERR_NORECIPIENT = 411, TODO:
	//  "<client> :No recipient given (<command>)"
	if (inParams[0].empty())
	{
		std::string arr[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_NORECIPIENT, arr, 1));
	}
	// ERR_NOTEXTTOSEND = 412, TODO: check this, i am unsure.
	if (inParams.size() < 2)
	{
		std::string arr[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_NOTEXTTOSEND, arr, 1));
	}

	std::stringstream stream(inParams[0]);
	std::string recipient;
	while (std::getline(stream, recipient, ','))
	{
		outMessage.getParams()[0] = recipient;
		if (recipient[0] == '#')
		{

			// {
			// 	// ERR_CANNOTSENDTOCHAN = 404, (not enough rights)
			// 	std::string arr[] = {sender.getNickname(), recipient};
			// 	return (sender.sendErrorMessage(ERR_CANNOTSENDTOCHAN, arr, 1));
			// }
		}
		else
		{
			if (!sender.sendMessageTo(outMessage, recipient, server))
			{
				// ERR_NOSUCHNICK (401)
				std::string arr[] = {sender.getNickname(), recipient};
				return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 1));
			}
		}

	}
	return;
}
