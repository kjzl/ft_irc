#include "../include/Command.hpp"
#include "../include/Debug.hpp"
#include "../include/ircUtils.hpp"
#include "../include/PassCommand.hpp"
#include "../include/NickCommand.hpp"
#include "../include/UserCommand.hpp"
#include "../include/PrivmsgCommand.hpp"
#include "Message.hpp"


Command::Command(const Message& msg) : inMessage_(msg)
{
	debug("new command created :)");
}


typedef Command* (*CommandFactory)(const Message&);

static void fillCommandMap(std::map<std::string, CommandFactory> &commandMap)
{
	commandMap["PASS"] = 	&PassCommand::fromMessage;
	commandMap["NICK"] = 	&NickCommand::fromMessage;
	commandMap["USER"] = 	&UserCommand::fromMessage;
	commandMap["PRIVMSG"] = &PrivmsgCommand::fromMessage;
	//...
}

Command* convertMessageToCommand(const Message& message)
{
	static std::map<std::string, CommandFactory> commandMap;
	if (commandMap.empty())
		fillCommandMap(commandMap);
	std::string type = message.getType();
	std::map<std::string, CommandFactory>::iterator it = commandMap.find(type);
	if (it == commandMap.end())		
		return NULL;
	Command* cmd = it->second(message);
	return cmd;
}

// void executeIncomingCommandMessage(Server& server, Client& sender, const std::string& rawMessage)
// {
// 	Message message(rawMessage);
// 	debug("Parsed message: " + message.getType() + " with params: " + toString(message.getParams().size()));
// 	Command* cmd = convertMessageToCommand(message);
// 	if (!cmd)
// 		return; // TODO: send ERROR_MSG 421
// 	cmd->execute(server, sender);
// 	delete cmd;
// }

Command::~Command()
{
}
