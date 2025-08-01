#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdio>
#include <string>
#include "CaseMappedString.hpp"

class   Client
{
	private:
		bool				is_authenticated_;
		int					socket_;
		CaseMappedString	nickname_;
		std::string			username_;
		std::string			realname_;
		std::string			rawMessage_;


	public:
		Client();
		Client(const Client &other);
		Client &operator =(const Client &other);
		virtual ~Client();

		bool isAuthenticated()	const;
		const CaseMappedString	&getNickname() const;
		const std::string		&getUsername() const;
		const std::string		&getRealname() const;
		const std::string		&getRawMessage() const;
		int getSocket()			const;

		void setAuthenticated(bool authenticated);
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
		void setRawMessage(const std::string &rawMessage);
		void setSocket(int socket);

		void appendRawMessage(const char partialMessage[BUFSIZ], size_t length);
		void clearMessage();
};

#endif

