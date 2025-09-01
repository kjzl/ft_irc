#ifndef WHOCOMMAND_HPP
#define WHOCOMMAND_HPP

#include "JoinCommand.hpp"
#include "Channel.hpp"

class WhoCommand : public JoinCommand {
public:
	WhoCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
};
#endif
