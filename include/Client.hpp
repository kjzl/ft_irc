#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "CaseMappedString.hpp"
#include "Message.hpp"
#include "MessageType.hpp"
#include <cstdio>
#include <string>
#include <vector>

class Server;
class Channel;
class MessageQueueManager;

class   Client
{
	private:
		MessageQueueManager	&mqr_;
		int					registrationLevel_;
		int					socket_;
		CaseMappedString	nickname_;
		std::string			username_;
		std::string			realname_;
		std::string			rawMessage_;
		std::string			IP_;

  public:
	Client(MessageQueueManager &queueManager);
	Client(const Client &other);
	Client &operator=(const Client &other);
	virtual ~Client();

	bool operator==(const std::string nickname);
	bool operator==(const Client &other);

		const std::string		&getNickname() const;
		const std::string		&getUsername() const;
		const std::string		&getRealname() const;
		const std::string		&getRawMessage() const;
		const std::string		&getIP() const;

		void	incrementRegistrationLevel(void);
		int		getRegistrationLevel(void) const;
		int		getSocket()			const;

		void	setNickname(const std::string &nickname);
		void	setUsername(const std::string &username);
		void	setRealname(const std::string &realname);
		void	setRawMessage(const std::string &rawMessage);
		void	setSocket(int socket);
		void	setIP(const std::string &IP);

		bool	isAuthenticated()	const;
		void	appendRawMessage(const char partialMessage[BUFSIZ], size_t length);
		void	clearMessage();
		void	sendMessage(Message toSend) const;
		void 	sendCmdValidation(const Message inMessage) const;
		void	sendCmdValidation(const Message inMessage, const Channel &channel) const;
		// sends a message to recipient, returns false if recipientNickname not found and could not send, else true
		bool	sendMessageTo(Message msg, const std::string recipientNickname, Server &server) const;
		void	sendErrorMessage(MessageType type, std::vector<std::string>& args) const;
		void	sendErrorMessage(MessageType type, std::string arg1) const;
		void	sendErrorMessage(MessageType type, std::string arg1, std::string arg2) const;
		void	sendErrorMessage(MessageType type, std::string arg1, std::string arg2, std::string arg3) const;
		void	sendErrorMessage(MessageType type, std::string arg1, std::string arg2, std::string arg3, std::string arg4) const;
		//send to fd stuff
		void	sendToFd(const std::string &string, int fd) const;
		void	sendMessageToFd(Message msg, int fd) const;
};

#endif
