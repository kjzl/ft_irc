#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>

#include "Client.hpp"

#define	BACKLOG 10
#define	TIMEOUT	4000 // = /1000 to seconds waiting for events
#define	WELCOME_MESSAGE "Welcome to this server. You are the "
#define	HOSTNAME "AspenWood"

// class	Channel;

class Server {
	public:
		virtual ~Server();
		Server( int port, std::string password );

		Server(const Server& other);
		Server& operator=( const Server& other );
		// core
		void	waitForRequests(void);
		void	serverShutdown(void);
		// getters
		const std::string	&getName( void ) const;
		const std::string	&getPassword( void ) const;
		// ?
		bool	nickCollision(CaseMappedString& toCheck);
		// everything is exposed :
		std::vector<Client>&			getClients(void);
		std::map<std::string, Channel>&	getChannels(void);
		// Utils
		Channel* mapChannel(const std::string& channelName);

	private:
		Server( void );
		// Getters and setters
		int		getPort( void ) const;
		int		getServerSocket( void ) const;
		void	setServerSocket( int serverSocketFd );
		// const std::map<std::string, Channel>	&getChannelMap() const;
		void	addPollFd(const int fd, const short events, const short revents);

		void	createListeningSocket(void);
		void	serverInit(void);
		void	acceptConnection();
		void	processPollIn(struct pollfd request, int pollIndex);
		void	removeClient(int pollIndexToRemove);
		static void	signalHandler(int signum);
		void	makeMessage(Client &client);
		void	executeIncomingCommandMessage(Client& sender, const std::string& rawMessage);

		
		const std::string				name_;				
		const int						port_;
		const std::string				password_;
		int								serverSocket_;
		static bool						running_;
		std::vector<struct pollfd>		pollFds_;
		std::vector<Client>				clients_;
		// std::map<std::string, Channel>	channels_;
};

#endif // !SERVER_HPP

