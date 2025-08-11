#ifndef JOINCOMMAND_HPP
#define JOINCOMMAND_HPP

#include "Command.hpp"
#include "Channel.hpp"

class JoinCommand : public Command {
public:
	JoinCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
protected:
	void sendValidationMessages(Client& sender, Channel& channel);
	void sendValidationMessages_353_366(Client& sender, Channel& channel);
};
#endif
