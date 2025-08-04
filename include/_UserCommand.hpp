#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "Command.hpp"

// https://modern.ircdocs.horse/#user-message

class UserCommand : Command {
public:
	UserCommand(const std::string& username, const std::string& realname);
	UserCommand(const UserCommand& other);
	UserCommand& operator=(const UserCommand& other);
	virtual ~UserCommand();

	static Command* fromMessage(const Message& message);
	void execute(Server& server, Client& sender);
private:
	std::string username_;
	std::string realname_;
};

#endif // USERCOMMAND_HPP
