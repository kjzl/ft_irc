#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class   Client
{
	private:
		bool        is_authenticated_;
		std::string nickname_;
		std::string username_;
		std::string realname_;
		int			socket_;


	public:
		Client();
		Client(const Client &other);
		Client &operator =(const Client &other);
		virtual ~Client();

		bool isAuthenticated() const;
		const std::string &getNickname() const;
		const std::string &getUsername() const;
		const std::string &getRealname() const;
		int getSocket() const;

		void setAuthenticated(bool authenticated);
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
		void setSocket(int socket); 
};

#endif

