#ifndef UNKNOWNCOMMAND_HPP
#define UNKNOWNCOMMAND_HPP

#include "Command.hpp"
#include "Channel.hpp"

class UnknownCommand : public Command
{
public:
	UnknownCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
};
#endif
