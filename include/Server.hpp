#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>
#include <ctime>

#include "Client.hpp"

#define	BACKLOG			10
#define	TIMEOUT			4000 // = /1000 to seconds waiting for events
#define	HOSTNAME		"AspenWood"
#define	VERSION			"AspenIrc-0.0"
#define	AVAILABLEUSERMODES		""
#define	AVAILABLECHANNELMODES	"it"
#define	AVAILABLECHANNELMODESWITHPARAMETER "k,o,l"

class	Channel;

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
		bool		clientNickExists(CaseMappedString& toCheck);
		void		broadcastMsg(const Message &message) const;
		void		broadcastErrorMessage(MessageType type, std::string args[], int size) const;
		void		broadcastErrorMessage(MessageType type, std::vector<std::string>& args) const;
		void		quitClient(const Client &quitter, std::vector<std::string> &messageParams);
		const char	*getTimeCreatedHumanReadable() const;
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
		void	addPollFd(const int fd, const short events, const short revents);

		void	createListeningSocket(void);
		void	serverInit(void);
		void	acceptConnection();
		void	processPollIn(struct pollfd request, int pollIndex);
		void	removeClient(int pollIndexToRemove);
		static void	signalHandler(int signum);
		void	makeMessage(Client &client);
		void	executeIncomingCommandMessage(Client& sender, const std::string& rawMessage);
		Message	buildErrorMessage(MessageType type, std::vector<std::string> messageParams) const;

		
		const std::string				name_;				
		const int						port_;
		const std::string				password_;
		int								serverSocket_;
		static bool						running_;
		std::vector<struct pollfd>		pollFds_;
		std::vector<Client>				clients_;
		std::map<std::string, Channel>	channels_;
		const time_t					timeCreated_;
};

#endif // !SERVER_HPP

