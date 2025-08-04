#include "../include/Command.hpp"
#include "../include/Debug.hpp"
#include "../include/ErrNeedMoreParams.hpp"
#include "../include/ErrNotRegistered.hpp"
#include "../include/ErrAlreadyRegistered.hpp"
#include "../include/UserCommand.hpp"
#include "../include/ircUtils.hpp"
#include "../include/NickCommand.hpp"

static CommandFactory factory(int minArgs, AuthRequirement auth, Command* (*createCommand)(const Message&, const Client&))
{
	CommandFactory commandFactory;
	commandFactory.minArgs = minArgs;
	commandFactory.auth = auth;
	commandFactory.createCommand = createCommand;
	return commandFactory;
}

static void fillCommandFactories(std::map<MessageType, CommandFactory>& commandFactories)
{
	commandFactories[USER] = factory(4, REQUIRES_UNAUTH, UserCommand::fromMessage);
	commandFactories[NICK] = factory(1, NO_AUTH_REQUIRED, NickCommand::fromMessage);
}

Command* convertMessageToCommand(const Message& message, const Client& sender)
{
	static std::map<MessageType, CommandFactory> commandFactories;

	if (commandFactories.empty())
		fillCommandFactories(commandFactories);

	std::map<MessageType, CommandFactory>::iterator it = commandFactories.find(message.getType());
	if (it == commandFactories.end())
		throw std::logic_error("No command factory found for message type " + message.getTypeAsString());

	CommandFactory factory = it->second;
	return factory.createCommand(message, sender);
}

void executeIncomingCommandMessage(Server& server, Client& sender, const std::string& rawMessage)
{
	try {
		// Message message = Message::parseIncomingMessage(rawMessage);
		Message message(rawMessage);
		debug("Parsed message: " + message.getTypeAsString() + " with params: " + toString(message.getParams().size()));

		Command* command = convertMessageToCommand(message, sender);
		if (command)
		{
			command->execute(server, sender);
			delete command;
		}
	} catch (const ErrReply& errReply) {
		debug("Error reply: " + std::string(errReply.what()));
		Message errorMessage = errReply.toMessage();
		sender.sendMessage(errorMessage);
	} catch (const std::exception& e) {
		debug("Exception caught: " + std::string(e.what()));
		std::vector<std::string> params;
		params.push_back(sender.getNickname());
		params.push_back(e.what());
		Message errorMessage(ERR_UNKNOWNERROR, params);
		sender.sendMessage(errorMessage);
	}
}

Command::~Command()
{
}
