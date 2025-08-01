#include "../include/Command.hpp"
#include "../include/Debug.hpp"
#include "../include/ErrNeedMoreParams.hpp"
#include "../include/ErrNotRegistered.hpp"
#include "../include/ErrAlreadyRegistered.hpp"
#include "../include/UserCommand.hpp"
#include "../include/ircUtils.hpp"

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
}

Command* convertMessageToCommand(const Message& message, const Client& sender)
{
	static std::map<MessageType, CommandFactory> commandFactories;

	if (commandFactories.empty())
		fillCommandFactories(commandFactories);

	CommandFactory factory = commandFactories[message.getType()];

	if (factory.auth == REQUIRES_AUTH && !sender.isAuthenticated())
	{
		debug("Command requires authentication but client is not authenticated");
		throw ErrNotRegistered(sender);
	}
	else if (factory.auth == REQUIRES_UNAUTH && sender.isAuthenticated())
	{
		debug("Command requires unauthenticated client but client is authenticated");
		throw ErrAlreadyRegistered(sender);
	}
	if (static_cast<size_t>(factory.minArgs) <= message.getParams().size())
		return factory.createCommand(message, sender);
	else
		throw ErrNeedMoreParams(sender, message.getTypeAsString());
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
	} catch (const Message::UnknownMessageTypeException& e) {
		debug("Exception caught: " + std::string(e.what()));
		std::vector<std::string> params;
		params.push_back(sender.getNickname());
		params.push_back(e.what());
		Message errorMessage(ERR_UNKNOWNERROR, params);
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
