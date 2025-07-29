#ifndef SERVER_HPP
# define SERVER_HPP

#include <map>
#include <vector>
#include <string>

#define	BACKLOG 10
#define	TIMEOUT	2500 // = 2.5 seconds waiting for events

class	Client;
class	Channel;

class Server {
	public:
		Server( void );
		virtual ~Server();
		Server( int port, std::string password );

		Server(const Server& other);
		Server& operator=( const Server& other );
		void	waitForRequests(void);

	private:
		// Getters and setters
		int			getPort( void ) const;
		int			getServerSocket( void ) const;
		void		setServerSocket( int serverSocketFd );
		void	addPollFd(const int fd, const short events, const short revents);

		void		createListeningSocket(void);
		void		serverInit(void);
		void	serverShutdown(void);
		void	acceptConnection();
		static void	signalHandler(int signum);

		const int						port_;
		int								serverSocket_;
		static bool						running;
		std::vector<struct pollfd>		pollFds_;
		std::map<std::string, Client>	clients_;
		std::map<std::string, Channel>	channels_;
};

#endif // !SERVER_HPP

