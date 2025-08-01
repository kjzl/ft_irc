#ifndef ERRNOTREGISTERED_HPP
#define ERRNOTREGISTERED_HPP

#include "ErrReply.hpp"

class ErrNotRegistered : ErrReply {
public:
	ErrNotRegistered(const Client& client);
	virtual ~ErrNotRegistered() throw();
};

#endif // ERRNOTREGISTERED_HPP
