#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>

class	Client;
class	Channel;

class Server {
	public:
		Server( void );
		virtual ~Server();
		Server( int port, std::string password );

		Server(const Server& other);
		Server& operator=( const Server& other );

		// Getters and setters
		int	serverInit(void);
		int	serverSocketCreate(void);
	private:
		const int					port_;
		int							serverSocket_;
		static bool					signal_;
		std::vector<struct pollfd>	pollFds_;
		std::vector<Client>			clients_;
		std::vector<Channel>		channels_;
};

#endif // !SERVER_HPP

