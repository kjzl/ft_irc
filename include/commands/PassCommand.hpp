#ifndef PASSCOMMAND_HPP
#define PASSCOMMAND_HPP

#include "../Command.hpp"

class PassCommand : public Command {
public:
	PassCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
};

#endif
