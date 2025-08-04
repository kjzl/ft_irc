#ifndef ERRALREADYREGISTERED_HPP
#define ERRALREADYREGISTERED_HPP

#include "ErrReply.hpp"
#include "Client.hpp"

class ErrAlreadyRegistered : ErrReply {
public:
	ErrAlreadyRegistered(const Client& client);
	virtual ~ErrAlreadyRegistered() throw();
};

#endif // ERRALREADYREGISTERED_HPP
