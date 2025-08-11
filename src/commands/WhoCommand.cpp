#include "../../include/WhoCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"

#include <sstream>

WhoCommand::WhoCommand(const Message& msg) : JoinCommand(msg)
{}

Command* WhoCommand::fromMessage(const Message& message)
{
	return new WhoCommand(message);
}
/*
    https://modern.ircdocs.horse/#join-message

    ERR_NOTREGISTERED (451)		=> done
	ERR_NEEDMOREPARAMS (461)	=> done
    ERR_NOSUCHCHANNEL (403) 	=> done
    RPL_NAMREPLY (353)			=> done
    RPL_ENDOFNAMES (366)		=> done
*/
void WhoCommand::execute(Server& server, Client& sender)
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
	Channel *channel = (server.mapChannel(channelName)); 
	// 403
	if (channelName[0] != '#' || !channel)
	{
		std::string arr[] = {sender.getNickname(), channelName};
		return(sender.sendErrorMessage(ERR_NOSUCHCHANNEL, arr, 2));
	}
	sendValidationMessages_353_366(sender, *channel);
}
