#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdio>
#include <string>
#include <vector>
#include "CaseMappedString.hpp"
#include "MessageType.hpp"

class Message;
class Server;

class   Client
{
	private:
		int					registrationLevel_;
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

		const std::string		&getNickname() const;
		const std::string		&getUsername() const;
		const std::string		&getRealname() const;
		const std::string		&getRawMessage() const;

		void	incrementRegistrationLevel(void);
		int		getRegistrationLevel(void) const;
		int		getSocket()			const;

		void	setNickname(const std::string &nickname);
		void	setUsername(const std::string &username);
		void	setRealname(const std::string &realname);
		void	setRawMessage(const std::string &rawMessage);
		void	setSocket(int socket);

		bool	isAuthenticated()	const;
		void	appendRawMessage(const char partialMessage[BUFSIZ], size_t length);
		void	clearMessage();
		int		safeSend(const std::string &string);
		void	sendMessage(Message toSend);
		void	sendErrorMessage(MessageType type, const Server& server, std::vector<std::string>& args);
		void 	sendErrorMessage(MessageType type, const Server& server, std::string args[], int size);

};

#endif

