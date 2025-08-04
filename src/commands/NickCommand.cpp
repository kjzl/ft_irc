#include "../../include/NickCommand.hpp"
#include "../../include/Debug.hpp"

NickCommand::NickCommand(const Message& inMessage): inMessage_(inMessage)
{
	debug("NickCommand default constructor called");
}

NickCommand::NickCommand(const NickCommand& other) {
	debug("NickCommand copy constructor called");
	*this = other;
}

NickCommand& NickCommand::operator=(const NickCommand& other) {
	debug("NickCommand assignment operator called");
	if (this != &other) {

	}
	return *this;
}

NickCommand::~NickCommand() {
	debug("NickCommand destructor called");
}

Command* NickCommand::fromMessage(const Message& message, const Client& sender)
{
	(void)sender;
	// dont think we need to check whether params 1 and 2 are "0" and "*" as theyre unused anyway
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
	std::vector<std::string> params = inMessage_.getParams();
	
	// checkParamCount() => 431
	if (params.empty())
	{
		Message outMessage(ERR_NONICKNAMEGIVEN, {sender.getNickname(), ":No nickname given"}, server.getName());
		return (sender.sendMessage(outMessage));
	}

	// checkRegistrationLevel(1) => kick_client or nothing ?
	if (sender.getRegistrationLevel() == 0)
	{
		return;
	}

	// check format => 432
	if (checkNickFormat(params[0]))
	{
		Message outMessage(ERR_ERRONEUSNICKNAME, {sender.getNickname(), ":Erroneus nickname"}, server.getName());
		return (sender.sendMessage(outMessage));
	}
	// check already in use => 433

	// set Nickname on sucess !
	sender.setNickname(params[0]);
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