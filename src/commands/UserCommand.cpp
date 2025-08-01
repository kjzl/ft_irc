#include "UserCommand.hpp"
#include "Debug.hpp"
#include "ErrAlreadyRegistered.hpp"

UserCommand::UserCommand(const std::string& username, const std::string& realname) : username_(username), realname_(realname)
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

Command* UserCommand::fromMessage(const Message& message, const Client& sender)
{
	(void)sender;
	// dont think we need to check whether params 1 and 2 are "0" and "*" as theyre unused anyway
	return new UserCommand(message.getParams()[0], message.getParams()[3]);
}

void UserCommand::execute(Server& server, Client& sender)
{
	if (sender.isAuthenticated())
	{
		debug("UserCommand execute called, but client is already authenticated");
		throw ErrAlreadyRegistered(sender);
	}
	// TODO check whether username and realname are valid or duplicated etc.
	sender.setUsername(username_);
	sender.setRealname(realname_);
	// TODO is this the right place to set the authenticated flag?
	sender.setAuthenticated(true);
	// TODO do we need to send a reply?
	(void)server;
}
