#include "../../include/JoinCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"

#include <sstream>

JoinCommand::JoinCommand(const Message& msg) : Command(msg)
{}

Command* JoinCommand::fromMessage(const Message& message)
{
	return new JoinCommand(message);
}

/*
    https://modern.ircdocs.horse/#join-message

    ERR_NOTREGISTERED (451)		=> done
	ERR_NEEDMOREPARAMS (461)	=> done
    ERR_NOSUCHCHANNEL (403) 	=> done - TODO: is this one for the name convention with # ???
    ERR_TOOMANYCHANNELS (405)	=> we ignore that...
    ERR_BADCHANNELKEY (475)		=> done
    ERR_BANNEDFROMCHAN (474)	=> TODO: do we want to implement that after KICK ?
    ERR_CHANNELISFULL (471)		=> we ignore that... our limit is MAX_INT
    ERR_INVITEONLYCHAN (473)	=> done
    ERR_BADCHANMASK (476)		=> ???
    RPL_TOPIC (332)				=> done
    RPL_TOPICWHOTIME (333)		=> we don't do that (no history in our modest server)
    RPL_NAMREPLY (353)			=> done
    RPL_ENDOFNAMES (366)		=> no necessarry IMO ?
*/
void JoinCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();
	// 451
	if (!sender.isAuthenticated())
	{
		std::string params[] = {sender.getNickname()};
		return (sender.sendErrorMessage(ERR_NOTREGISTERED, params, 1));
	}
	// 461
	if (inParams.size() < 1)
	{
		std::string arr[] = {sender.getNickname(), inMessage_.getType()};
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, arr, 2));
	}
	// Split the channels
	std::stringstream stream_channel(inParams[0]);
	std::stringstream stream_key;
	if (inParams.size() > 1)
		stream_key.str(inParams[1]);
	std::string channelName, key;
	while (std::getline(stream_channel, channelName, ','))
	{
		// 403
		if (channelName[0] != '#')
		{
			std::string arr[] = {sender.getNickname(), channelName};
			sender.sendErrorMessage(ERR_NOSUCHCHANNEL, arr, 2);
			continue;
		}
		Channel *channel = const_cast<Channel *>(server.mapChannel(channelName)); 
		// Creating a new channel
		if (!channel)
		{
			server.getChannels()[channelName] = Channel(channelName, sender);
			// TODO: send confimation ??
			continue;
		}
		std::getline(stream_key, key, ',');
		// ERR_BADCHANNELKEY (475)
		if(!channel->checkKey(key))
		{
			std::string arr[] = {sender.getNickname(), channelName};
			sender.sendErrorMessage(ERR_BADCHANNELKEY, arr, 2);
			continue;
		}
		// ERR_INVITEONLYCHAN (473)
		if (channel->isInviteOnly() && !channel->isWhiteListed(sender.getNickname()))
		{
			std::string arr[] = {sender.getNickname(), channelName};
			sender.sendErrorMessage(ERR_INVITEONLYCHAN, arr, 2);
			continue;
		}
		// Success with adding member !
		channel->addMember(&sender);
		// RPL_TOPIC (332)
		// TODO: implement
		// RPL_NAMREPLY (353)
		// TODO: implement
	}
}
