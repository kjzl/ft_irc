#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <functional>
#include <map>
#include "Message.hpp"
#include "Server.hpp"

class Command {
public:
	virtual ~Command() = 0;
	virtual void execute(Server& server) = 0;
};

struct CommandFactory {
	int minArgs;
	bool	requiresAuth;
	Command* (*createCommand)(const Message&);
};

Command* convertMessageToCommand(const Message& message);

#endif // COMMAND_HPP
