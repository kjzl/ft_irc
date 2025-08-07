#include "ModeCommand.hpp"
#include "Debug.hpp"
#include <vector>
// Default Constructor
ModeCommand::ModeCommand( void ): Command()
{
	debug("Default Constructor called");
}

ModeCommand::ModeCommand(const Message& msg) : Command(msg)
{}

// Destructor
ModeCommand::~ModeCommand()
{
	debug("Destructor called");
}

// Copy Constructor
ModeCommand::ModeCommand(const ModeCommand &copy): Command(copy)
{}

// Copy Assignment Operator
ModeCommand& ModeCommand::operator=( const ModeCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	ModeCommand::fromMessage(const Message& message)
{
	return new ModeCommand(message);
}

/*
If <target> is a nickname that does not exist on the network,
the ERR_NOSUCHNICK (401) numeric is returned. If <target> is a different nick
than the user who sent the command, the ERR_USERSDONTMATCH (502) numeric is returned.
If <modestring> is not given, the RPL_UMODEIS (221) numeric is sent back
containing the current modes of the target user.
If <modestring> is given, the supplied modes will be applied, and a MODE
message will be sent to the user containing the changed modes. If one or more
modes sent are not implemented on the server, the server MUST apply the modes
that are implemented, and then send the ERR_UMODEUNKNOWNFLAG (501) in reply along with the MODE message.
*/
void	ModeCommand::userMode(Server& server, Client& sender)
{
	std::vector<std::string> parameters = inMessage_.getParams();
	std::string	senderNick = sender.getNickname();
	CaseMappedString caseMappedNick(parameters[0]);
	if (parameters.size() < 1 || !server.clientNickExists(caseMappedNick))
	{
		std::string arr[] = {sender.getNickname(), parameters[0]};
		return (sender.sendErrorMessage(ERR_NOSUCHNICK, arr, 2));
	}
	if (senderNick != parameters[0])
	{
		std::string arr[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_USERSDONTMATCH, arr, 1));
	}
}

/*
https://modern.ircdocs.horse/#mode-message
 Parameters: <target> [<modestring> [<mode arguments>...]]
*/
void	ModeCommand::execute(Server& server, Client& sender)
{
	if (!sender.isAuthenticated())
		return ;
	(void) server;
	(void) sender;

}

