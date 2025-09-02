#include "QuitCommand.hpp"
#include "Debug.hpp"
#include "iostream"
#include "Message.hpp"
#include <cmath>
// Default Constructor
QuitCommand::QuitCommand( void ): Command()
{
	debug("Default Constructor called");
}

QuitCommand::QuitCommand(const Message& msg) : Command(msg)
{}

// Destructor
QuitCommand::~QuitCommand()
{
	debug("Destructor called");
}

// Copy Constructor
QuitCommand::QuitCommand(const QuitCommand &copy): Command(copy)
{}

// Copy Assignment Operator
QuitCommand& QuitCommand::operator=( const QuitCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	QuitCommand::fromMessage(const Message& message)
{
	return new QuitCommand(message);
}

/*
https://modern.ircdocs.horse/#quit-message
example: QUIT :Gone to have lunch 
may relpies with ERROR message:
https://modern.ircdocs.horse/#error-message
does a broadcast of the quit:
example: :dan-!d@localhost QUIT :Quit: Bye for now!
*/
void	QuitCommand::execute(Server& server, Client& sender)
{
	if (!sender.isAuthenticated())
		inMessage_.setSource(sender);
	// do the exit & send notice to everyone
	//
	std::cerr << "Quit parameters size: " << (inMessage_.getParams().size())  << std::endl;
	if (inMessage_.getParams().size())
		return (server.quitClient(sender, inMessage_.getParams()[0]));
	server.quitClient(sender, "");
}

