#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "Command.hpp"

class UserCommand : Command {
public:
	UserCommand(std::string username, std::string realname);
	UserCommand(const UserCommand& other);
	UserCommand& operator=(const UserCommand& other);
	virtual ~UserCommand();

	UserCommand(const Message& message);
	void execute(Server& server);
private:
	std::string username_;
	std::string realname_;
};

#endif // USERCOMMAND_HPP
