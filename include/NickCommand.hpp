#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "Command.hpp"

// https://modern.ircdocs.horse/#Nick-message

class NickCommand : Command {
public:
	NickCommand(const Message& inMessage);
	NickCommand(const NickCommand& other);
	NickCommand& operator=(const NickCommand& other);
	virtual ~NickCommand();

	static Command* fromMessage(const Message& message, const Client& sender);
	void execute(Server& server, Client& sender);
private:
	Message inMessage_;
	bool	checkNickFormat(std::string nickname);
};

#endif // NickCOMMAND_HPP
