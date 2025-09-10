#include "../../include/commands/PassCommand.hpp"
#include "../../include/MessageType.hpp"

PassCommand::PassCommand(const Message& msg) : Command(msg)
{}

Command* PassCommand::fromMessage(const Message& message)
{
	return new PassCommand(message);
}

/*
    https://modern.ircdocs.horse/#pass-message
	ERR_NEEDMOREPARAMS (461)
    ERR_ALREADYREGISTERED (462)
    ERR_PASSWDMISMATCH (464)
*/
void PassCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();

	// 461
	if (inParams.size() == 0)
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
	// 462
	if (sender.getRegistrationLevel() > 0)
		return (sender.sendErrorMessage(ERR_ALREADYREGISTERED, sender.getNickname()));
	// 464
	if (inParams[0] != server.getPassword())
		return (sender.sendErrorMessage(ERR_PASSWDMISMATCH, sender.getNickname()));
	// Sucess !
	sender.incrementRegistrationLevel();
	return;
}

