#include "ErrAlreadyRegistered.hpp"
#include "Debug.hpp"

ErrAlreadyRegistered::ErrAlreadyRegistered(const Client& client)
: ErrReply(client, ERR_ALREADYREGISTERED)
{
	debug("ErrAlreadyRegistered default constructor called");
	params_.push_back("You may not reregister");
}

ErrAlreadyRegistered::~ErrAlreadyRegistered()
{
	debug("ErrAlreadyRegistered destructor called");
}

