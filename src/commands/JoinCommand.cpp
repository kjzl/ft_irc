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
    ERR_NOSUCHCHANNEL (403) 	=> done
    ERR_TOOMANYCHANNELS (405)	=> we ignore that...
    ERR_BADCHANNELKEY (475)		=> done
    x ERR_BANNEDFROMCHAN (474)	=> we don't ban
    ERR_CHANNELISFULL (471)		=> TODO: we ignore that... our limit is MAX_INT ? ==> nop
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
		std::getline(stream_key, key, ',');
		// 403
		if (channelName[0] != '#')
		{
			std::string arr[] = {sender.getNickname(), channelName};
			sender.sendErrorMessage(ERR_NOSUCHCHANNEL, arr, 2);
			continue;
		}
		Channel *channel = (server.mapChannel(channelName)); 
		// Creating a new channel
		if (!channel)
		{
			server.getChannels()[channelName] = Channel(channelName, sender);
			channel = (server.mapChannel(channelName));
			sendValidationMessages(sender, *channel);
			continue;
		}
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
		sendValidationMessages(sender, *channel);
	}
}

void JoinCommand::sendValidationMessages(Client& sender, Channel& channel)
{
	sender.sendCmdValidation(inMessage_, channel);
	debug("channel name : " + channel.getName());
	// RPL_TOPIC (332)
	if (channel.getTopic().length())
	{
		std::string params[] = {sender.getNickname(), channel.getName(), channel.getTopic()};
		sender.sendErrorMessage(RPL_TOPIC, params, 3);
	}
	// RPL_NAMREPLY (353) & RPL_ENDOFNAMES (366)
	std::string memberList(":");
	for (std::map<std::string, int>::const_iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it)
	{
		if (it->first != sender.getNickname())
		{
			if (channel.getOperators().find(it->first) != channel.getOperators().end())
				memberList += "@";
			memberList += it->first + " ";
		}
	}
	if (memberList.size() > 1)
	{
		std::string params[] = {sender.getNickname(), "=", channel.getName(), memberList};
		sender.sendErrorMessage(RPL_NAMREPLY, params, 4);

		std::string params2[] = {sender.getNickname(), channel.getName()};
		sender.sendErrorMessage(RPL_ENDOFNAMES, params2, 2);
	}
}
