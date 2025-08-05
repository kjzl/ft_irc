#include "PassCommand.hpp"
#include "Debug.hpp"
#include "IrcError.hpp"

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
	{
		std::string arr[] = {sender.getNickname(), inMessage_.getType()};
		std::vector<std::string> outParams(arr, arr + 2);
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, outParams));
	}
	// 462
	if (sender.getRegistrationLevel() > 0)
	{
		std::string arr[] = {sender.getNickname()};
		std::vector<std::string> outParams(arr, arr + 1);
		return (sender.sendErrorMessage(ERR_ALREADYREGISTERED, outParams));
	}
	// 464
	if (inParams[0] != server.getPassword())
	{
		std::string arr[] = {sender.getNickname()};
		std::vector<std::string> outParams(arr, arr + 1);
		return (sender.sendErrorMessage(ERR_PASSWDMISMATCH, outParams));
	}
	// Sucess !
	sender.incrementRegistrationLevel();
	return;
}

