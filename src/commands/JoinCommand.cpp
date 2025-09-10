#include "JoinCommand.hpp"
#include "Debug.hpp"
#include "MessageType.hpp"

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
    ERR_CHANNELISFULL (471)		=> done
    ERR_INVITEONLYCHAN (473)	=> done
    ERR_BADCHANMASK (476)		=> ???
    RPL_TOPIC (332)				=> done
    RPL_TOPICWHOTIME (333)		=> we don't do that (no history in our modest server)
    RPL_NAMREPLY (353)			=> done
    RPL_ENDOFNAMES (366)		=> done
*/
void JoinCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();
	// 451
	if (!sender.isAuthenticated())
		return (sender.sendErrorMessage(ERR_NOTREGISTERED, sender.getNickname()));
	// 461
	if (inParams.size() < 1)
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
	
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
			sender.sendErrorMessage(ERR_NOSUCHCHANNEL, sender.getNickname(), channelName);
			continue;
		}
		Channel *channel = (server.mapChannel(channelName)); 
		// Creating a new channel
		if (!channel) {
			// Insert without requiring default constructor
			std::map<std::string, Channel> &chs = server.getChannels();
			std::pair<std::map<std::string, Channel>::iterator, bool> ins =
				chs.insert(std::make_pair(
					channelName,
					Channel(channelName, sender, server.getMessageQueueManager())));
			channel = &ins.first->second;
			sendValidationMessages(sender, *channel);
			continue;
		}
		// ERR_BADCHANNELKEY (475)
		if(!channel->checkKey(key))
		{
			sender.sendErrorMessage(ERR_BADCHANNELKEY, sender.getNickname(), channelName);
			continue;
		}
		// ERR_INVITEONLYCHAN (473)
		if (channel->isInviteOnly() && !channel->isWhiteListed(sender.getNickname()))
		{
			sender.sendErrorMessage(ERR_INVITEONLYCHAN, sender.getNickname(), channelName);
			continue;
		}
		// ERR_CHANNELISFULL (471)
		if (channel->getUserLimit() && (int)channel->getMembers().size() >= channel->getUserLimit())
		{
			sender.sendErrorMessage(ERR_CHANNELISFULL, sender.getNickname(), channelName);
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
		sender.sendErrorMessage(RPL_TOPIC, sender.getNickname(), channel.getName(), channel.getTopic());
	sendValidationMessages_353_366(sender, channel);
}

void JoinCommand::sendValidationMessages_353_366(Client& sender, Channel& channel)
{
	// RPL_NAMREPLY (353) & RPL_ENDOFNAMES (366)
	std::string memberList;
	for (std::map<std::string, int>::const_iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it)
	{
		if (it->first != sender.getNickname())
		{
			if (channel.getOperators().find(it->first) != channel.getOperators().end())
				memberList += "@";
			memberList += it->first + " ";
		}
	}
	sender.sendErrorMessage(RPL_NAMREPLY, sender.getNickname(), "=", channel.getName(), memberList);
	sender.sendErrorMessage(RPL_ENDOFNAMES, sender.getNickname(), channel.getName());
}
