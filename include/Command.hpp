#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <functional>
#include <map>
#include "Message.hpp"
#include "Server.hpp"

class Command {
public:
	virtual ~Command();
	virtual void execute(Server& server, Client& sender) = 0;
};

struct CommandFactory {
	int		minArgs;
	bool	requiresAuth;
	Command* (*createCommand)(const Message&, const Client&);
};

Command* convertMessageToCommand(const Message& message, const Client& sender);
void executeIncomingCommandMessage(Server& server, Client& sender, const std::string& rawMessage);

#endif // COMMAND_HPP
