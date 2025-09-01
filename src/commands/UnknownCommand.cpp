#include "../../include/UnknownCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"

#include <sstream>

UnknownCommand::UnknownCommand(const Message& msg): Command(msg)
{}

Command* UnknownCommand::fromMessage(const Message& message)
{
	return new UnknownCommand(message);
}

void UnknownCommand::execute(Server& server, Client& sender)
{
	(void)server;
	sender.sendErrorMessage(ERR_UNKNOWNCOMMAND, sender.getNickname(), inMessage_.getType());
	return;
}
