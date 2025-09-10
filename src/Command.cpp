#include "../include/Message.hpp"
#include "../include/Command.hpp"
#include "../include/Debug.hpp"
#include "../include/commands/PassCommand.hpp"
#include "../include/commands/NickCommand.hpp"
#include "../include/commands/UserCommand.hpp"
#include "../include/commands/PrivmsgCommand.hpp"
#include "../include/commands/JoinCommand.hpp"
#include "../include/commands/KickCommand.hpp"
#include "../include/commands/QuitCommand.hpp"
#include "../include/commands/InviteCommand.hpp"
#include "../include/commands/TopicCommand.hpp"
#include "../include/commands/ModeCommand.hpp"
#include "../include/commands/WhoCommand.hpp"
#include "../include/commands/UnknownCommand.hpp"

// Default Constructor
Command::Command( void ): inMessage_()
{
	debug("Default Constructor called");
}

// Destructor
Command::~Command()
{
	debug("Destructor called");
}

// Copy Constructor
Command::Command(const Command &copy): inMessage_(copy.inMessage_)
{}

// Copy Assignment Operator
Command& Command::operator=( const Command &assign )
{
	if (this != &assign)
	{
		//cant assign to const message
		debug("TRIED TO ASSIGN TO CONST MESSAGE, that should not happen");
	}
	return *this;
}


Command::Command(const Message& msg) : inMessage_(msg)
{
	debug("new command created :)");
}


typedef Command* (*CommandFactory)(const Message&);

static void fillCommandMap(std::map<std::string, CommandFactory> &commandMap)
{
	commandMap["PASS"]		= &PassCommand::fromMessage;
	commandMap["NICK"]		= &NickCommand::fromMessage;
	commandMap["USER"]		= &UserCommand::fromMessage;
	commandMap["PRIVMSG"]	= &PrivmsgCommand::fromMessage;
	commandMap["NOTICE"]	= &PrivmsgCommand::fromMessage;
	commandMap["JOIN"]		= &JoinCommand::fromMessage;
	commandMap["KICK"]		= &KickCommand::fromMessage;
	commandMap["QUIT"]		= &QuitCommand::fromMessage;
	commandMap["INVITE"]	= &InviteCommand::fromMessage;
	commandMap["TOPIC"]		= &TopicCommand::fromMessage;
	commandMap["MODE"]		= &ModeCommand::fromMessage;
	commandMap["WHO"]		= &WhoCommand::fromMessage;
	commandMap["UNKNOWN"]	= &UnknownCommand::fromMessage;
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
		return (commandMap["UNKNOWN"](message));
	Command* cmd = it->second(message);
	return cmd;
}
