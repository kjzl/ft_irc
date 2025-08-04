#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "Command.hpp"

// https://modern.ircdocs.horse/#Nick-message

class NickCommand : Command {
public:
	NickCommand(const Message& msg);
	void			execute(Server& server, Client& sender);
	static Command*	fromMessage(const Message& message);
private:
	bool			checkNickFormat(std::string nickname);
};

#endif // NickCOMMAND_HPP
