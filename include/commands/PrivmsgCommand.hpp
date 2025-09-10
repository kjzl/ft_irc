#ifndef PRIVMSGCOMMAND_HPP
#define PRIVMSGCOMMAND_HPP

#include "../Command.hpp"

class PrivmsgCommand : public Command {
public:
	PrivmsgCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
private:
		void	privmsgRecipient(std::string recipient, Server& server, Client& sender);
};

#endif
