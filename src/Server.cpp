#include "../include/Server.hpp"
#include "../include/Debug.hpp"

// Default Constructor
Server::Server( void ): port_(6667)
{
	debug("Default Constructor called");
}

// Parameterized Constructor
Server::Server( int port, std::string password ): port_(port)
{
	debug("Parameterized Constructor called");
}

// Destructor
Server::~Server()
{
	debug("Destructor called");
}

// Copy Constructor
Server::Server(const Server& rhs): port_(rhs.port_)
{
	*this = rhs;
}

// Copy Assignment Operator
Server& Server::operator=( const Server& rhs )
{
	if (this != &rhs)
	{
		//Todo
	}
	return *this;
}

