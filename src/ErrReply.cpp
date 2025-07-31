#include "ErrReply.hpp"
#include "Debug.hpp"

ErrReply::ErrReply(const Client& client, MessageType errType) : errType_(errType)
{
	debug("ErrReply default constructor called");
	params_.push_back(client.getNickname());
}

ErrReply::~ErrReply()
{
	debug("ErrReply destructor called");

}

