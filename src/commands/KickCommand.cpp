#include "KickCommand.hpp"
#include "Debug.hpp"
// Default Constructor
KickCommand::KickCommand( void ): Command()
{
	debug("Default Constructor called");
}

KickCommand::KickCommand(const Message& msg) : Command(msg)
{}

// Destructor
KickCommand::~KickCommand()
{
	debug("Destructor called");
}

// Copy Constructor
KickCommand::KickCommand(const KickCommand &copy): Command(copy)
{}

// Copy Assignment Operator
KickCommand& KickCommand::operator=( const KickCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	KickCommand::fromMessage(const Message& message)
{
	return new KickCommand(message);
}

/*
https://modern.ircdocs.horse/#kick-message
ERR_NEEDMOREPARAMS (461)
ERR_NOSUCHCHANNEL (403)
ERR_CHANOPRIVSNEEDED (482)
ERR_USERNOTINCHANNEL (441)
ERR_NOTONCHANNEL (442)
*/
void	KickCommand::execute(Server& server, Client& sender)
{
	(void) server;
	(void) sender;
}

