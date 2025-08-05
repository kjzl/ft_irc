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
	RPL_AWAY = 301

*/
void PrivmsgCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();
	
	// Not Authenticated ==> ignore it...
	if (!sender.isAuthenticated())
		return;
	// TODO: handle all errors...
	
	// Success !
	Message outMessage = inMessage_;
	outMessage.setSource(sender.getNickname(), sender.getUsername());
	std::cerr << BLUE << "message: '" << outMessage << "'" << RESET << std::endl;
	std::stringstream stream(inParams[0]);
	std::string token;
	while (std::getline(stream, token, ','))
	{
		outMessage.getParams()[0] = token;
		if (token[0] == '#')
		{

			// {
			// 	// ERR_CANNOTSENDTOCHAN = 404, (not enough rights)
			// 	std::string arr[] = {sender.getNickname(), token};
			// 	return (sender.sendErrorMessage(ERR_CANNOTSENDTOCHAN, arr, 1));
			// }
		}
		else
		{
			if (!sender.sendMessageTo(outMessage, token, server))
			{
				// ERR_NOSUCHNICK (401)
				std::string arr[] = {sender.getNickname(), token};
				return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 1));
			}
		}

	}
	return;
}
