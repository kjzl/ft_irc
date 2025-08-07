#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Debug.hpp"
#include "../include/ircUtils.hpp"
#include "../include/Command.hpp"
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
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
#include <vector>

bool Server::running_ = false;

void	Server::signalHandler(int signum)
{
	static_cast<void>(signum);
	running_ = false;
	debug("received a signal");
}

// Default Constructor
Server::Server(void): name_(HOSTNAME), port_(6667), password_("password")
{
	running_ = true;
	debug("Default Constructor called");
}

// Parameterized Constructor
Server::Server(int port, std::string password): name_(HOSTNAME), port_(port), password_(password), serverSocket_(-1)
{
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	debug("Parameterized Constructor called");
	std::cout << GREEN << "==== STARTING SERVER ====" << RESET << std::endl;
	running_ = true;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	serverInit();
}

// Destructor
Server::~Server()
{
	debug("Destructor called");
	// serverShutdown();
}

// Copy Constructor
Server::Server(const Server& other):
	name_(other.name_),
	port_(other.port_),
	password_(other.password_),
	serverSocket_(other.serverSocket_),
	pollFds_(other.pollFds_),
	clients_(other.clients_)
	// channels_(other.channels_)
{}

// Copy Assignment Operator
Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		serverSocket_ = other.serverSocket_;
		pollFds_ = other.pollFds_;
		clients_ = other.clients_;
		//channels_ = other.channels_;
	}
	return *this;
}

const std::string	&Server::getName( void ) const
{
	return (name_);
}

const std::string	&Server::getPassword( void ) const
{
	return (password_);
}

std::vector<Client>& Server::getClients( void )
{
	return clients_;
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
	addPollFd(clientFd, POLLIN, 0);
	debug("[Server] accepted new connection");
	std::string	clientFdString = toString(clientFd);
	Client newcomer;
	newcomer.setSocket(clientFd);
	clients_.push_back(newcomer);
}

Message	Server::buildErrorMessage(MessageType type, std::vector<std::string> messageParams) const
{
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
	IrcErrorInfo info = ErrorMap.find(type)->second;
	messageParams.push_back(info.message);
	Message message(info.code, messageParams);
	return (message);
}

// used to removeClient from server (in poll and clients) and broadcast the Quit message.
// if server needs to diconnect the client, modify messageParams to reflect reason
void	Server::quitClient(const Client &quitter, const std::vector<std::string> &messageParams)
{
	std::vector<Client>::iterator clIt = std::find(clients_.begin(), clients_.end(), quitter);
	size_t	clientIndex = clIt - clients_.begin();
	if (pollFds_[clientIndex + 1].fd != quitter.getSocket())
		throw std::logic_error("clientIndex in quitClient is not corespondent to the quitting client");
	removeClient(clientIndex + 1);
	Message msg = buildErrorMessage(QUIT, messageParams);
	for (std::map<std::string, Channel>::iterator cMapIter = channels_.begin(); cMapIter != channels_.end(); cMapIter++)
	{
		std::string	qNickname = quitter.getNickname();
		Channel quittersChannel = cMapIter->second;
		if (!quittersChannel.isMember(qNickname))
			continue;
		quittersChannel.broadcastMsg(quitter, msg);
		quittersChannel.removeMember(qNickname);
	}
}

// closes and delets an elements from pollIndex_ 
//the entry in pollFds_ corresponds to the same index -1 in clients_ for that particular client. they should have the same fd.
void	Server::removeClient(int pollIndexToRemove)
{
	struct pollfd pollToRemove = pollFds_[pollIndexToRemove];
	debug("removing Client");
	std::cout << "[Server] Client on fd " << pollToRemove.fd << " has disconnected." << std::endl;
	shutdown(pollToRemove.fd, SHUT_RD);
	if (-1 == close(pollToRemove.fd))
		throw std::runtime_error("close error");
	if (clients_[pollIndexToRemove - 1].getSocket() != pollFds_[pollIndexToRemove].fd)
		throw std::logic_error("pollfds and clients should be indexwise only -1 apart");
	clients_[pollIndexToRemove - 1] = clients_.back();
	clients_.pop_back();
	pollFds_[pollIndexToRemove] = pollFds_.back();
	pollFds_.pop_back();
}

void Server::executeIncomingCommandMessage(Client& sender, const std::string& rawMessage)
{
	Message message(rawMessage);
	debug("Parsed message: " + message.getType() + " with params: " + toString(message.getParams().size()));
	Command* cmd = convertMessageToCommand(message);
	if (!cmd)
	{
		std::string arr[] = {sender.getNickname(), message.getType()};
		sender.sendErrorMessage(ERR_UNKNOWNCOMMAND, arr, 2);
		return;
	}
	cmd->execute(*this, sender);
	delete cmd;
}

void Server::broadcastErrorMessage(MessageType type, std::vector<std::string>& args) const
{
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
    IrcErrorInfo info = ErrorMap.find(type)->second;
    args.push_back(info.message);
    Message outMessage(info.code,  args);
	broadcastMsg(outMessage);
}

void	Server::broadcastErrorMessage(MessageType type, std::string args[], int size) const
{
	std::vector<std::string> outParams(args, args + size);
    static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
    IrcErrorInfo info = ErrorMap.find(type)->second;
    outParams.push_back(info.message);
    Message outMessage(info.code, outParams);
	broadcastMsg(outMessage);
}

void	Server::broadcastMsg(const Message &message) const
{
	Client	tmp;
	for (std::vector<Client>::const_iterator clientIt = (clients_.begin()); clientIt != clients_.end(); clientIt++)
		tmp.sendMessageToFd(message, clientIt->getSocket());
}


bool	Server::clientNickExists(CaseMappedString& toCheck)
{
	for (size_t clientIndex = 0; clientIndex < clients_.size(); clientIndex++)
	{
		Client	compareTo = clients_[clientIndex];
		if (toCheck == compareTo.getNickname())
			return (1);
	}
	return (0);
}

//attempts to extract a full message from the clients sent input
//if it extraced a string, calls the parser and executes the command
void	Server::makeMessage(Client &client)
{
	std::string	command;
	std::string	raw_message = client.getRawMessage();
	size_t		position;

	while ((position = raw_message.find('\n')) != raw_message.npos)
	{
		if (position != 0 && raw_message[position - 1] == '\r')
			command = raw_message.substr(0, position - 1);
		else
			command = raw_message.substr(0, position);
		raw_message.erase(0, position + 1);
		client.setRawMessage(raw_message);
		std::cout << command << std::endl;
		executeIncomingCommandMessage(client, command);
		debug(raw_message);
	}
}

// this function should check if its a POLLHUP or POLLIN 
// then interpret the message
// and execute it
void	Server::processPollIn(struct pollfd request, int pollIndex)
{
	char	message[BUFSIZ];
	int		bytesRead;

	// if (request.revents & POLLHUP)
	// 	removeClient(pollIndex);
	// else
	// {
		bytesRead = recv(request.fd, message, BUFSIZ, MSG_DONTWAIT);
		if (bytesRead == 0)
			removeClient(pollIndex);
		else if (bytesRead == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			throw std::runtime_error("[Server] recv error");
		}
		else
		{
			std::cout << CYN << "[received a message from client: " << request.fd <<" ]" << RESET << std::endl;
			clients_[pollIndex - 1].appendRawMessage(message, bytesRead);
			makeMessage(clients_[pollIndex - 1]);
		}
	// }
}

//main loop to be in while running
//checks if any fd is ready and prints waiting messages inbetween
//if ready, checks polls through and directs them toward acceptConnection or readFromSocket
void	Server::waitForRequests(void)
{
	try {
		while (running_)
		{
			int	rdyPollsCount = 0;
			rdyPollsCount = poll(&(pollFds_[0]), pollFds_.size(), TIMEOUT);
			if (running_ && rdyPollsCount == -1)
				throw std::runtime_error("[Server] poll error");
			else if (rdyPollsCount == 0)
			{
				std::cout << "[Server] Waiting for requests" << std::endl;
				continue;
			}
			int	rdyPollsChecked = 0;
			for (size_t pollIndex = 0; running_ && pollIndex < pollFds_.size() && rdyPollsChecked < rdyPollsCount; pollIndex++)
			{ 
				if (((pollFds_[pollIndex].revents & (POLLIN | POLLHUP)) != 1))
					continue; // this socket is not the ready one
				++rdyPollsChecked;
				if (pollIndex == 0)
					acceptConnection();
				else
					processPollIn(pollFds_[pollIndex], pollIndex);
				std::cout << std::endl;
			}
		}
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << ": " << strerror(errno) << std::endl;
		serverShutdown();
	}
	std::cout << YEL << "[Server] Stopped listening for requests" << RESET << std::endl;
}

//creates the one listening socket the server has to start out with
void	Server::createListeningSocket(void)
{
	int	err = 0;
	struct addrinfo	hints = {0, 0, 0, 0, 0, 0, 0, 0};
	struct addrinfo	*res = {0};
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = AI_PASSIVE; //put in my ip for me
	std::string port_str = toString(port_);
	int optval = 1;

	err = (getaddrinfo(NULL, port_str.c_str(), &hints, &res));
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
	//reusing old socket, if still open, to circumvent TIME_WAIT
	setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	//bind socket
	if (-1 == bind(getServerSocket(), res->ai_addr, res->ai_addrlen))
	{
		close(serverSocket_);
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
	for (size_t pollIndex = 1; pollIndex < pollFds_.size(); pollIndex++)
	{
		shutdown(pollFds_[pollIndex].fd,  SHUT_RDWR);
		if (-1 == close(pollFds_[pollIndex].fd))
			throw std::runtime_error("[Server] close error on fd: " + toString(pollFds_[pollIndex].fd));
	}
	std::cout << "[Server] diconnected all clients sockets" << RESET << std::endl;
	if (serverSocket_ != -1)
	{
		shutdown(serverSocket_,  SHUT_RDWR);
		if (-1 == close(serverSocket_))
			throw std::runtime_error("[Server] close error on serverSocket");
		std::cout << "[Server] diconnected listening socket" << RESET << std::endl;
	}
	std::cout << GREEN << "[Server] Shutdown complete" << RESET << std::endl;
}

Channel* Server::mapChannel(const std::string& channelName) const
{
	std::map<std::string, Channel>::const_iterator it = channels_.find(channelName);
	if (it != channels_.end())
		return &(it->second);
	return NULL;
}

std::map<std::string, Channel>&	Server::getChannels(void)
{
	return (channels_);
}

