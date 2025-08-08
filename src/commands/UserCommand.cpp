#include "../../include/UserCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"
#include <vector>

UserCommand::UserCommand(const Message& msg) : Command(msg)
{}

Command* UserCommand::fromMessage(const Message& message)
{
	return new UserCommand(message);
}

void	UserCommand::welcome(const Server &server, const Client &sender)
{
	(void) server;
	std::string	nickname = sender.getNickname();
	std::string	welcome = std::string("Welcome to the ") + HOSTNAME + " Network, " + nickname;
	std::string yourhost = std::string("Your host is ") + HOSTNAME + ", running version" + VERSION;
	std::string created = std::string("This server was created ") + server.getTimeCreatedHumanReadable();
	std::string myInfo = std::string() + HOSTNAME + " " + VERSION + " " AVAILABLEUSERMODES + " " + AVAILABLECHANNELMODES + " " + AVAILABLECHANNELMODESWITHPARAMETER; 
	std::vector<std::string> vec;
	vec.reserve(2);
	vec.push_back(nickname);
	sender.sendErrorMessage(RPL_WELCOME, vec);
	vec[1] = yourhost;
	sender.sendErrorMessage(RPL_YOURHOST, vec);
	vec[1] = created;
	sender.sendErrorMessage(RPL_CREATED , vec);
	vec[1] = myInfo;
	sender.sendErrorMessage(RPL_MYINFO , vec);
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
	
	if (sender.getRegistrationLevel() < 2) // user needs to give pass or nick first
	{
		debug("registration level too low for user command");
		return ;
	}
	// 461
	if (inParams.size() < 4)
	{
		std::string arr[] = {sender.getNickname(), inMessage_.getType()};
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, arr, 2));
	}
	// 462
	if (sender.isAuthenticated())
	{
		std::string arr[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_ALREADYREGISTERED, arr, 1));
	}
	// Sucess !
	else if (sender.getRegistrationLevel() == 2)
	{
	debug("now setting user, incrementing registration level");
	sender.setUsername(inParams[0]);
	sender.setRealname(inParams[3]);
	sender.incrementRegistrationLevel();
	welcome(server, sender);
	}
	return;
}
