#include "../include/Server.hpp"
#include "../include/Debug.hpp"
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

//TODO: addPollFd clients: allways POLLIN | POLLHUP

void	Server::signalHandler(int signum)
{
	static_cast<void>(signum);
	running = false;
}

// Default Constructor
Server::Server(void): port_(6667)
{
	debug("Default Constructor called");
}

// Parameterized Constructor
Server::Server(int port, std::string password): port_(port), serverSocket_(-1)
{
	debug("Parameterized Constructor called");
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
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

int	Server::getPort( void ) const
{
	return (port_);
}

int	Server::getServerSocket( void ) const
{
	return this->serverSocket_;
}

void	Server::addPollFd(const int fd, const short events, const short revents)
{
	struct pollfd newPollfd = {fd, events, revents};
	pollFds_.push_back(newPollfd);
}

void	Server::setServerSocket( int serverSocketFd )
{
	this->serverSocket_ = serverSocketFd;
}

void	Server::acceptConnection(int pollIndex)
{
	int clientFd;

	clientFd = accept(getServerSocket(), NULL, NULL);
	if (clientFd == -1)
		throw std::runtime_error("[Server] accept error");
	addPollFd(clientFd, POLLIN | POLLHUP, 0);
	debug("server accepted new connection");

}

// this function should check if its a POLLHUP or POLLIN 
// then interpret the message
// and execute it
void	Server::readFromSocket(struct pollfd request)
{
	//TODO:
}

void	Server::waitForRequests(void)
{
	while (running)
	{
		int rdyPollsCount = poll(&(pollFds_[0]), pollFds_.size(), TIMEOUT);
		if (rdyPollsCount == -1)
			throw std::runtime_error("[Server] poll error");
		else if (rdyPollsCount == 0)
		{
			std::cout << "[Server] Waiting for requests" << std::endl;
			continue;
		}
		int	rdyPollsChecked;
		for (int pollIndex = 0; pollIndex < pollFds_.size() && rdyPollsChecked < rdyPollsCount; pollIndex++)
		{ 
			if (((pollFds_[pollIndex].revents & (POLLIN | POLLHUP)) != 1))
				continue; // this socket is not the ready one
			++rdyPollsChecked;
			if (pollIndex == 0)
				acceptConnection();
			else
				readFromSocket(pollFds_[pollIndex]);
		}
	}
	std::cout << YEL << "[Server] Stopped listening for requests" << RESET << std::endl;
}

void	Server::createListeningSocket(void)
{
	struct	sockaddr_in	sa = {0};
	sa.sin_family = AF_INET;
	sa.sin_port = htons(getPort());
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	//create Socket
	serverSocket_ = socket(sa.sin_family, SOCK_STREAM, 0);
	if (serverSocket_ == -1)
		throw std::runtime_error("[Server] socket error");
	//bind socket
	if (-1 == bind(getServerSocket(), reinterpret_cast<struct sockaddr *>(&sa), sizeof sa))
		throw std::runtime_error("[Server] bind error");
	// mark as passive socket listening to incoming connections from clients
	if (-1 == listen(getServerSocket(), BACKLOG))
		throw std::runtime_error("[Server] listen error");
}

void	Server::serverInit(void)
{
	pollFds_.reserve(5);

	this->createListeningSocket();
	this->addPollFd(getServerSocket(), POLLIN, 0);
}

void	Server::serverShutdown(void)
{
	std::cout << BRED << "==== STARTING SERVER SHUTDOWN ====" << RESET << std::endl;
	//TODO:
	// loop through clients and close their fd
	if (serverSocket_ != -1)
	{
		if (-1 == close(serverSocket_))
			throw std::runtime_error(strerror(errno));
		std::cout << RED << "[Server] diconnected listening socket" << RESET << std::endl;
	}
	std::cout << GREEN << "[Server] Shutdown complete" << RESET << std::endl;
}
