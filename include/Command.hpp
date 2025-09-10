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

Command* convertMessageToCommand(const Message& message);

#endif
