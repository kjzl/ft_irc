#ifndef JOINCOMMAND_HPP
#define JOINCOMMAND_HPP

#include "Command.hpp"
#include "Channel.hpp"

class JoinCommand : Command {
public:
	JoinCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
};

#endif
