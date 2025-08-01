#include "ErrNotRegistered.hpp"
#include "Debug.hpp"

ErrNotRegistered::ErrNotRegistered(const Client& client)
: ErrReply(client, ERR_NOTREGISTERED)
{
	debug("ErrNotRegistered default constructor called");
	params_.push_back("You have not registered");
}

ErrNotRegistered::~ErrNotRegistered() throw()
{
	debug("ErrNotRegistered destructor called");

}

