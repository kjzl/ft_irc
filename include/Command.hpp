#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Message.hpp"
#include "Server.hpp"

class Command
{
	public:
		Command(const Command &copy);
		Command& operator=( const Command &assign );

		Command(const Message& msg);
		virtual ~Command();
		virtual void	execute(Server& server, Client& sender) = 0;
	protected:
		Message inMessage_;
		Command();
};

// enum AuthRequirement {
// 	REQUIRES_UNAUTH,
// 	NO_AUTH_REQUIRED,
// 	REQUIRES_AUTH
// };

// struct CommandFactory {
// 	int		minArgs;
// 	AuthRequirement	auth;
// 	Command* (*createCommand)(const Message&, const Client&);
// };

//TODO: Move those as member functions of the servers !
Command* convertMessageToCommand(const Message& message);
// void executeIncomingCommandMessage(Server& server, Client& sender, const std::string& rawMessage);

#endif // COMMAND_HPP
