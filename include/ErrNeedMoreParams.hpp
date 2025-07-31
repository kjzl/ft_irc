#ifndef ERRNEEDMOREPARAMS_HPP
#define ERRNEEDMOREPARAMS_HPP

#include "ErrReply.hpp"

class ErrNeedMoreParams : public ErrReply {
public:
	ErrNeedMoreParams(std::string client, std::string command);
	virtual ~ErrNeedMoreParams();
};

#endif // ERRNEEDMOREPARAMS_HPP
