#include "../../include/UserCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/IrcError.hpp"

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
	std::vector<std::string> inParams = inMessage_.getParams();
	
	// 461
	if (inParams.size() < 4)
	{
		std::string arr[] = {sender.getNickname(), inMessage_.getType()};
		std::vector<std::string> outParams(arr, arr + 2);
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, server, outParams));
	}
	// 462
	if (sender.getRegistrationLevel() == 3)
	{
		std::string arr[] = {sender.getNickname()};
		std::vector<std::string> outParams(arr, arr + 1);
		return (sender.sendErrorMessage(ERR_ALREADYREGISTERED, server, outParams));
	}
	// Sucess !
	sender.setUsername(inParams[0]);
	sender.setRealname(inParams[3]);
	sender.incrementRegistrationLevel();
	// TODO: implement that cleanly or just leave it like it is ?!...
	// std::string arr[] = {sender.getNickname()};
	// std::vector<std::string> outParams(arr, arr + 1);
	// sender.sendErrorMessage(RPL_WELCOME, server, outParams);
	// sender.sendErrorMessage(RPL_YOURHOST, server, outParams);
	// sender.sendErrorMessage(RPL_CREATED , server, outParams);
	// sender.sendErrorMessage(RPL_MYINFO , server, outParams);
	sender.sendMessage(Message("Welcome to the AspenWood modest IRC Chat :)"));
	sender.sendMessage(Message("You are now fully authenticated :D"));
	return;
}
