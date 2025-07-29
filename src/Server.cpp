#include "../include/Server.hpp"
#include "../include/Debug.hpp"
#include "../include/ircUtils.hpp"
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
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
#include <sys/fcntl.h>

//TODO: addPollFd clients: allways POLLIN | POLLHUP

void	Server::signalHandler(int signum)
{
	static_cast<void>(signum);
	running = false;
	std::cout << BLUE << "Signal received" << RESET << std::endl;
}

// Default Constructor
Server::Server(void): port_(6667)
{
	debug("Default Constructor called");
}

// Parameterized Constructor
Server::Server(int port, std::string password): port_(port), password_(password), serverSocket_(-1)
{
	debug("Parameterized Constructor called");
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
	serverInit();
}

// Destructor
Server::~Server()
{
	debug("Destructor called");
	serverShutdown();
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

// accepts a connection from client and adds it to pollFds_
void	Server::acceptConnection( void )
{
	int clientFd;

	clientFd = accept(getServerSocket(), NULL, NULL);
	if (clientFd == -1)
		throw std::runtime_error("[Server] accept error");
	addPollFd(clientFd, POLLIN | POLLHUP, 0);
	debug("server accepted new connection");
	std::string	clientFdString = intToStr(clientFd);
	std::string welcomeMessage = "Welcome to this server, you are the " + clientFdString + "th connection";
	if (-1 == send(clientFd, welcomeMessage.c_str(), welcomeMessage.size(), 0))
		throw std::runtime_error("[Server] send error with client: " + clientFdString);
}

// this function should check if its a POLLHUP or POLLIN 
// then interpret the message
// and execute it
void	Server::readFromSocket(struct pollfd request)
{
	(void) request;
	//TODO:
}

//main loop to be in while running
//checks if any fd is ready and prints waiting messages inbetween
//if ready, checks polls through and directs them toward acceptConnection or readFromSocket
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
		int	rdyPollsChecked = 0;
		for (size_t pollIndex = 0; pollIndex < pollFds_.size() && rdyPollsChecked < rdyPollsCount; pollIndex++)
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

//creates the one listening socket the server has to start out with
void	Server::createListeningSocket(void)
{
	struct addrinfo	hints, *res = {0};
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = AI_PASSIVE; //put in my ip for me
	std::string port_str = intToStr(port_);

	int err = (getaddrinfo(NULL, port_str.c_str(), &hints, &res));
	if (err != 0)
	{
		freeaddrinfo(res);
		throw std::runtime_error(std::string(gai_strerror(err)));
	}
	//create Socket
	serverSocket_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (serverSocket_ == -1)
	{
		freeaddrinfo(res);
		throw std::runtime_error("[Server] socket error");
	}
	//bind socket
	if (-1 == bind(getServerSocket(), res->ai_addr, res->ai_addrlen))
	{
		freeaddrinfo(res);
		throw std::runtime_error("[Server] bind error");
	}
	freeaddrinfo(res);
	// mark as passive socket listening to incoming connections from clients
	if (-1 == listen(getServerSocket(), BACKLOG))
		throw std::runtime_error("[Server] listen error");
}

// makes first listeing socket and adds it to pollFds_
void	Server::serverInit(void)
{
	pollFds_.reserve(5);

	this->createListeningSocket();
	this->addPollFd(getServerSocket(), POLLIN, 0);
}

//cleanup
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
