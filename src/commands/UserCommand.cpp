#include "UserCommand.hpp"
#include "Debug.hpp"

UserCommand::UserCommand(std::string username, std::string realname) : username_(username), realname_(realname)
{
	debug("UserCommand default constructor called");
}

UserCommand::UserCommand(const UserCommand& other) {
	debug("UserCommand copy constructor called");
	*this = other;
}

UserCommand& UserCommand::operator=(const UserCommand& other) {
	debug("UserCommand assignment operator called");
	if (this != &other) {

	}
	return *this;
}

UserCommand::~UserCommand() {
	debug("UserCommand destructor called");

}

