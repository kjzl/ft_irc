#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "Command.hpp"

class UserCommand : Command {
public:
	UserCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
};

#endif
