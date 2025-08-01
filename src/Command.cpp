#include "Command.hpp"
#include "Debug.hpp"
#include "ErrNeedMoreParams.hpp"
#include "ErrNotRegistered.hpp"
#include "UserCommand.hpp"

static CommandFactory factory(int minArgs, bool requiresAuth, Command* (*createCommand)(const Message&, const Client&))
{
	CommandFactory commandFactory;
	commandFactory.minArgs = minArgs;
	commandFactory.requiresAuth = requiresAuth;
	commandFactory.createCommand = createCommand;
	return commandFactory;
}

static void fillCommandFactories(std::map<MessageType, CommandFactory>& commandFactories)
{
	commandFactories[USER] = factory(4, false, UserCommand::fromMessage);
}

Command* convertMessageToCommand(const Message& message, const Client& sender)
{
	static std::map<MessageType, CommandFactory> commandFactories;

	if (commandFactories.empty())
		fillCommandFactories(commandFactories);

	CommandFactory factory = commandFactories[message.getType()];

	if (factory.requiresAuth && !sender.isAuthenticated())
	{
		debug("Command requires authentication but client is not authenticated");
		throw ErrNotRegistered(sender);
	}
	if (static_cast<size_t>(factory.minArgs) <= message.getParams().size())
		return factory.createCommand(message, sender);
	else
		throw ErrNeedMoreParams(sender, message.getTypeAsString());
}

void executeIncomingCommandMessage(Server& server, Client& sender, const std::string& rawMessage)
{
	try {
		Message message = Message::parseIncomingMessage(rawMessage);
		debug("Parsed message: " + message.getTypeAsString() + " with params: " + std::to_string(message.getParams().size()));

		Command* command = convertMessageToCommand(message, sender);
		if (command)
		{
			command->execute(server, sender);
			delete command;
		}
	} catch (const ErrReply& errReply) {
		debug("Error reply: " + std::string(errReply.what()));
		Message errorMessage = errReply.toMessage();
		// TODO send errorMessage to the client
	} catch (const std::exception& e) {
		debug("Exception caught: " + std::string(e.what()));
		std::vector<std::string> params;
		params.push_back(sender.getNickname());
		params.push_back(e.what());
		Message errorMessage(ERR_UNKNOWNERROR, params);
		// TODO send errorMessage to the client
	}
}

Command::~Command()
{
}
