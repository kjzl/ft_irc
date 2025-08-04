#include "../../include/NickCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/IrcError.hpp"

NickCommand::NickCommand(const Message& msg) : Command(msg)
{}

Command* NickCommand::fromMessage(const Message& message)
{
	return new NickCommand(message);
}

/*
The NICK command is used to give the client a nickname or change the previous one.
If the server receives a NICK command from a client where the desired nickname is already in use on the network, it should issue an ERR_NICKNAMEINUSE numeric and ignore the NICK command.
If the server does not accept the new nickname supplied by the client as valid (for instance, due to containing invalid characters), it should issue an ERR_ERRONEUSNICKNAME numeric and ignore the NICK command. Servers MUST allow at least all alphanumerical characters, square and curly brackets ([]{}), backslashes (\), and pipe (|) characters in nicknames, and MAY disallow digits as the first character. Servers MAY allow extra characters, as long as they do not introduce ambiguity in other commands, including:
no leading # character or other character advertized in CHANTYPES
no leading colon (:)
no ASCII space
If the server does not receive the <nickname> parameter with the NICK command, it should issue an ERR_NONICKNAMEGIVEN numeric and ignore the NICK command.
The NICK message may be sent from the server to clients to acknowledge their NICK command was successful, and to inform other clients about the change of nickname. In these cases, the <source> of the message will be the old nickname [ [ "!" user ] "@" host ] of the user who is changing their nickname.
Numeric Replies:
ERR_NONICKNAMEGIVEN (431)
ERR_ERRONEUSNICKNAME (432)
ERR_NICKNAMEINUSE (433)
ERR_NICKCOLLISION (436) // not existant for US
Command Example:
  NICK Wiz                  ; Requesting the new nick "Wiz".*/
void NickCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();
	
	// 431
	if (inParams.empty())
	{
		std::string arr[] = {sender.getNickname()};
		std::vector<std::string> outParams(arr, arr + 1);
		return (sender.sendErrorMessage(ERR_NONICKNAMEGIVEN, server, outParams));
	}
	// checkRegistrationLevel(1) => kick_client or nothing ?
	// if (sender.getRegistrationLevel() == 0)  // TODO: DISABLE FOR TESTING while PASS isn't implemented...
	// 	return;
	// 432
	if (checkNickFormat(inParams[0]))
	{
		std::string arr[] = {sender.getNickname(), inParams[0]};
		std::vector<std::string> outParams(arr, arr + 2);
		return (sender.sendErrorMessage(ERR_ERRONEUSNICKNAME, server, outParams));
	}
	// 433
	CaseMappedString tmp(inParams[0]);
	if (server.nickCollision(tmp))
	{
		std::string arr[] = {sender.getNickname(), inParams[0]};
		std::vector<std::string> outParams(arr, arr + 2);
		return (sender.sendErrorMessage(ERR_NICKNAMEINUSE, server, outParams));
	}
	// set Nickname on sucess !
	sender.setNickname(inParams[0]);
	sender.incrementRegistrationLevel();
	return;
}
bool	NickCommand::checkNickFormat(std::string nickname)
{
	if (nickname[0] == '#' // TODO: handles all CHANTYPES from Server ??
	|| nickname[0] == ':') 
		return (true);
	if (nickname.find(" ") != std::string::npos)
		return (true);
	return (false);
}
