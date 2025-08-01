#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <functional>
#include <map>
#include "Message.hpp"
#include "Server.hpp"

class Command {
public:
	virtual ~Command();
	virtual void	execute(Server& server, Client& sender) = 0;
};

enum AuthRequirement {
	REQUIRES_UNAUTH,
	NO_AUTH_REQUIRED,
	REQUIRES_AUTH
};

struct CommandFactory {
	int		minArgs;
	AuthRequirement	auth;
	Command* (*createCommand)(const Message&, const Client&);
};

Command* convertMessageToCommand(const Message& message, const Client& sender);
void executeIncomingCommandMessage(Server& server, Client& sender, const std::string& rawMessage);
void	sendMessage(Server& server, int fd, Message toSend);

#endif // COMMAND_HPP
