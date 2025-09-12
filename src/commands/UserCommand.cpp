#include "../../include/commands/UserCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"
#include <vector>

UserCommand::UserCommand(const Message& msg) : Command(msg)
{}

Command* UserCommand::fromMessage(const Message& message)
{
	return new UserCommand(message);
}

/*
    https://modern.ircdocs.horse/#user-message
    ERR_NEEDMOREPARAMS (461)
    ERR_ALREADYREGISTERED (462)
*/
void UserCommand::execute(Server& server, Client& sender)
{
	(void) server;
	std::vector<std::string> inParams = inMessage_.getParams();
	
	// 461
	if (inParams.size() < 4)
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
	// 462
	if (sender.isAuthenticated())
		return (sender.sendErrorMessage(ERR_ALREADYREGISTERED, sender.getNickname()));
	// Sucess !
	debug("now setting user");
	sender.setUsername(inParams[0]);
	sender.setRealname(inParams[3]);
	if (sender.isAuthenticated())
		sender.welcome(server);
	return;
}
