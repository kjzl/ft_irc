#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <string>

class Server {
	public:
		Server( void );
		virtual ~Server();
		Server( int port, std::string password );

		Server(const Server &copy);
		Server& operator=( const Server &assign );

		// Getters and setters
	private:
		const int					port_;
		static bool					signal_;
		std::vector<struct pollfd>	pollfds_;
};

#endif // !SERVER_HPP

