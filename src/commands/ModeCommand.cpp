#include "ModeCommand.hpp"
#include "Debug.hpp"
#include <cstdlib>
#include <vector>
#include "ircUtils.hpp"
// Default Constructor
ModeCommand::ModeCommand( void ): Command()
{
	debug("Default Constructor called");
}

ModeCommand::ModeCommand(const Message& msg) : Command(msg)
{}

// Destructor
ModeCommand::~ModeCommand()
{
	debug("Destructor called");
}

// Copy Constructor
ModeCommand::ModeCommand(const ModeCommand &copy): Command(copy)
{}

// Copy Assignment Operator
ModeCommand& ModeCommand::operator=( const ModeCommand &assign )
{
	if (this != &assign)
	{
		Command::operator=(assign);
	}
	return *this;
}

Command*	ModeCommand::fromMessage(const Message& message)
{
	return new ModeCommand(message);
}

/*
If <target> is a nickname that does not exist on the network,
the ERR_NOSUCHNICK (401) numeric is returned. If <target> is a different nick
than the user who sent the command, the ERR_USERSDONTMATCH (502) numeric is returned.
If <modestring> is not given, the RPL_UMODEIS (221) numeric is sent back
containing the current modes of the target user.
If <modestring> is given, the supplied modes will be applied, and a MODE
message will be sent to the user containing the changed modes. If one or more
modes sent are not implemented on the server, the server MUST apply the modes
that are implemented, and then send the ERR_UMODEUNKNOWNFLAG (501) in reply along with the MODE message.
option we are not implementing:
 o: Give/take channel operator privilege -can only be used on Channel
 i: Invisible User Mode
*/
void	ModeCommand::userMode(Server& server, Client& sender)
{
	std::vector<std::string> parameters = inMessage_.getParams();
	std::string	senderNick = sender.getNickname();
	CaseMappedString caseMappedNick(parameters[0]);
	if (!server.clientNickExists(caseMappedNick)) // TODO: what is this check ?
		return (sender.sendErrorMessage(ERR_NOSUCHNICK, sender.getNickname(), parameters[0]));
	if (senderNick != parameters[0])
		return (sender.sendErrorMessage(ERR_USERSDONTMATCH, sender.getNickname()));
	if (parameters.size() < 2)
	{
		parameters.push_back(senderNick);
		//RPL_UMODEIS
		std::map<std::string, Channel> allChannels = server.getChannels();
		for (std::map<std::string, Channel>::iterator channelIt = allChannels.begin(); channelIt != allChannels.end(); channelIt++)
		{
			if (channelIt->second.isOperator(senderNick))
			{
				parameters.push_back("+o");
				break;
			}
		}
		return (sender.sendErrorMessage(RPL_UMODEIS, parameters));
	}
	// as we don't implement any Client flags, if we get to this point:
	return (sender.sendErrorMessage(ERR_UMODEUNKNOWNFLAG, NULL, 0));
}

void ModeCommand::processChannelModes(Client &sender, const std::string& modestring, 
						 const std::vector<std::string>& parameters,
						 Channel* channel)
{
	bool addMode = true;
	size_t	paramIndex = 2;
	std::string	senderNick = sender.getNickname();
	for (std::string::const_iterator cIt = modestring.begin(); cIt != modestring.end(); cIt++) {
		switch (*cIt) {
			case '+':
				addMode = true;
				break;
			case '-':
				addMode = false;
				break;
			case 'i': // Invite-only flag
				channel->setInviteOnly(addMode);
				break;
			case 't': // Topic protection flag
				channel->setTopicProtected(addMode);
				break;
			case 'k': // Channel key (password)
				if (addMode) {
					if (paramIndex < parameters.size())
					{
						channel->setPassword(parameters[paramIndex++]);
					}
					else // ERR_NEEDMOREPARAMS (461)
						return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
				}
				else
					channel->setPassword("");
				break;
				
			case 'l': // User limit
				if (addMode) {
					if (paramIndex < parameters.size()) {
						char * endptr;
						channel->setUserLimit(std::strtol(parameters[paramIndex++].c_str(), &endptr, 10));
					}
					else // ERR_NEEDMOREPARAMS (461)
						return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
				} else {
					channel->setUserLimit(0); // Disable user limit
				}
				break;
				
			case 'o': // Channel operator status
				if (paramIndex < parameters.size()) {
					if (addMode) {
						channel->addOperator(parameters[paramIndex++]);
					} else {
						channel->removeOperator(parameters[paramIndex++]);
					}
					paramIndex++;
				}
				else // ERR_NEEDMOREPARAMS (461)
					return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, sender.getNickname(), inMessage_.getType()));
				break;
				
			default: // unkwown modes
				sender.sendErrorMessage(ERR_UNKNOWNMODE, sender.getNickname(), inMessage_.getType(), std::string(1, *cIt));
				return;
		}
	}
}

/*
If <target> is a channel that does not exist on the network, the
ERR_NOSUCHCHANNEL (403) numeric is returned.
If <modestring> is not given, the RPL_CHANNELMODEIS (324) numeric is returned.
Servers MAY choose to hide sensitive information such as channel keys when
sending the current modes. Servers SHOULD also return the RPL_CREATIONTIME
(329) numeric following RPL_CHANNELMODEIS.
If <modestring> is given, the user sending the command MUST have appropriate
channel privileges on the target channel to change the modes given. If a user
does not have appropriate privileges to change modes on the target channel,
the server MUST NOT process the message, and ERR_CHANOPRIVSNEEDED (482)
numeric is returned. If the user has permission to change modes on the target,
the supplied modes will be applied based on the type of the mode (see below).
For type A, B, and C modes, arguments will be sequentially obtained from <mode
arguments>. If a type B or C mode does not have a parameter when being set,
the server MUST ignore that mode. If a type A mode has been sent without an
argument, the contents of the list MUST be sent to the user, unless it
contains sensitive information the user is not allowed to access. When the
server is done processing the modes, a MODE command is sent to all members of
the channel containing the mode changes. Servers MAY choose to hide sensitive
information when sending the mode changes.
 */
void	ModeCommand::channelMode(Server& server, Client& sender)
{
	std::vector<std::string>	parameters = inMessage_.getParams();
	std::string					channelName = parameters[0];
	std::string					nickname = sender.getNickname();
	Channel						*channel = (server.mapChannel(channelName));
	if (!channel)
	{
		sender.sendErrorMessage(ERR_NOSUCHCHANNEL, parameters);
	}
	if (parameters.size() == 1)
	{
	//RPL_CHANNELMODEIS
		std::string	modetypes = "+";
		if (channel->isInviteOnly())
			modetypes += "i";
		if (channel->isTopicProtected())
			modetypes += "t";
		std::string channelPassword = channel->getPassword();
		if (!channelPassword.empty())
			modetypes += "k";
		int	channelLimit = channel->getUserLimit();
		if (channelLimit)
			modetypes += "l";
		if (modetypes.size() > 1)
		{
			parameters.push_back(modetypes);
			if (!channel->getPassword().empty())
				parameters.push_back(channelPassword);
			if (channelLimit)
				parameters.push_back(toString(channelLimit));
		}
		sender.sendErrorMessage(RPL_CHANNELMODEIS, parameters);

		//RPL_CREATIONTIME TODO: SHOULD == MUST ...
	}
	else
	{
		if (!channel->isOperator(nickname))
			sender.sendErrorMessage(ERR_CHANOPRIVSNEEDED, sender.getNickname(), channelName);
		// set Channel Modes, may ERR_NEEDMOREPARAMS
		processChannelModes(sender, parameters[1], parameters, channel);
	}
}

/*
https://modern.ircdocs.horse/#mode-message
 Parameters: <target> [<modestring> [<mode arguments>...]]
 target is nickname of self of #channel
 modestring are  [-+] followed by [itkol]
*/
void	ModeCommand::execute(Server& server, Client& sender)
{
	if (!sender.isAuthenticated())
		return ;
	std::vector<std::string> parameters = inMessage_.getParams();
	std::string	senderNick = sender.getNickname();
	if (parameters.size() < 1)
		return (sender.sendErrorMessage(ERR_NEEDMOREPARAMS, senderNick, inMessage_.getType()));
	if (parameters[0][0] == '#')
		return (channelMode(server, sender));
	return (userMode(server, sender));
}

