#include "ErrNeedMoreParams.hpp"
#include "Debug.hpp"

ErrNeedMoreParams::ErrNeedMoreParams(const Client& client, const std::string& command)
	: ErrReply(client, ERR_NEEDMOREPARAMS)
{
	debug("ErrNeedMoreParams default constructor called");
	params_.push_back(command);
	params_.push_back("Not enough parameters");
}

ErrNeedMoreParams::~ErrNeedMoreParams() throw()
{
	debug("ErrNeedMoreParams destructor called");
}

