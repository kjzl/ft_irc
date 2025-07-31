#ifndef ERRREPLY_HPP
#define ERRREPLY_HPP

#include <exception>
#include "Message.hpp"
#include "Client.hpp"

class ErrReply : std::exception {
public:
	ErrReply(const Client& client, MessageType errType);
	virtual ~ErrReply();
	const char* what() const throw();
	Message toMessage() const;

protected:
	std::vector<std::string> params_;
	const MessageType errType_;
};

#endif // ERRREPLY_HPP
