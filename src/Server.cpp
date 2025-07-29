#include "../include/Server.hpp"
#include "../include/Debug.hpp"
#include <netinet/in.h>

// Default Constructor
Server::Server(void): port_(6667)
{
	debug("Default Constructor called");
}

// Parameterized Constructor
Server::Server(int port, std::string password): port_(port), serverSocket_(-1)
{
	debug("Parameterized Constructor called");
	pollFds_.reserve(5);
}

// Destructor
Server::~Server()
{
	debug("Destructor called");
}

// Copy Constructor
Server::Server(const Server& other): port_(other.port_)
{
	*this = other;
}

// Copy Assignment Operator
Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		//Todo
	}
	return *this;
}

int	Server::serverSocketCreate(void)
{
	struct	sockadr_in sa;

	sa.sin_family
}

int	Server::serverInit(void)
{
	serverSocket_ = serverSocketCreate();
	if (serverSocket_ == -1)
		return (-1);
}

