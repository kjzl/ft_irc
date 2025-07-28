#include "../include/Server.hpp"
#include "../include/Debug.hpp"

// Default Constructor
Server::Server( void )
{
	debug("Default Constructor called");
}

// Parameterized Constructor
// Server::Server( std::string value )
// {
// 	debug("Parameterized Constructor called");
// }

// Destructor
Server::~Server()
{
	debug("Destructor called");
}

// Copy Constructor
Server::Server(const Server &copy)
{
	*this = copy;
}

// Copy Assignment Operator
Server& Server::operator=( const Server &assign )
{
	if (this != &assign)
	{
		//Todo
	}
	return *this;
}

