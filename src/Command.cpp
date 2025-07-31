#include "Command.hpp"
#include "ErrNeedMoreParams.hpp"

void fillCommandFactories(std::map<MessageType, CommandFactory>& commandFactories)
{
	commandFactories[USER] = {1, 1, /*TODO*/0};
}

static Command* convertMessageToCommand(const Message& message, const Client& sender)
{
	static std::map<MessageType, CommandFactory> commandFactories;

	if (commandFactories.empty())
		fillCommandFactories(commandFactories);

	CommandFactory factory = commandFactories[message.getType()];

	if (factory.minArgs <= message.getParams().size())
		return factory.createCommand(message);
	else
		throw ErrNeedMoreParams(sender)
}
