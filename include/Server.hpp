#ifndef SERVER_HPP
# define SERVER_HPP

class Server {
	public:
		Server( void );
		virtual ~Server();

		Server(const Server &copy);
		Server& operator=( const Server &assign );

		// Getters and setters
	private:
};

#endif // !SERVER_HPP

