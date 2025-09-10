#include "../include/MessageType.hpp"

const std::map<MessageType, IrcErrorInfo>& getErrorMap()
{
	static std::map<MessageType, IrcErrorInfo> errorMap;

	if (errorMap.empty())
	{
 //GENERIC
		errorMap[ERROR]					= IrcErrorInfo("ERROR", "");
		errorMap[ERR_UNKNOWNCOMMAND]	= IrcErrorInfo("421", "Unknown command");
//NICK
		errorMap[ERR_NONICKNAMEGIVEN] 	= IrcErrorInfo("431", "No nickname given");
		errorMap[ERR_ERRONEUSNICKNAME]	= IrcErrorInfo("432", "Erroneous nickname");
		errorMap[ERR_NICKNAMEINUSE]		= IrcErrorInfo("433", "Nickname is already in use");
        errorMap[ERR_NEEDMOREPARAMS]	= IrcErrorInfo("461", "Not enough parameters");
        errorMap[ERR_ALREADYREGISTERED]	= IrcErrorInfo("462", "You may not reregister");
//PASS
        errorMap[ERR_PASSWDMISMATCH]	= IrcErrorInfo("464", "Password incorrect");
//PRIVMSG
		errorMap[ERR_NOSUCHNICK]		= IrcErrorInfo("401", "No such nick/channel");
		errorMap[ERR_NOSUCHCHANNEL]		= IrcErrorInfo("403", "No such channel");
		errorMap[ERR_CANNOTSENDTOCHAN]	= IrcErrorInfo("404", "Cannot send to channel");
		errorMap[ERR_NORECIPIENT]		= IrcErrorInfo("411", "No recipient given (PRIVMSG)"); // beware to never use this anywhere where outside of PRIVMSG, else we change it so this is empty here
		errorMap[ERR_NOTEXTTOSEND]		= IrcErrorInfo("412", "No text to send");
//JOIN
		errorMap[ERR_BADCHANNELKEY]		= IrcErrorInfo("475", "Cannot join channel (+k)");
		errorMap[ERR_INVITEONLYCHAN]	= IrcErrorInfo("473", "Cannot join channel (+i)");
		errorMap[ERR_CHANNELISFULL]		= IrcErrorInfo("471", "Channel is full (+l)");
// QUIT
		errorMap[QUIT]					= IrcErrorInfo("QUIT", "Quit");
// MODE
		errorMap[ERR_USERSDONTMATCH]	= IrcErrorInfo("502", "Cant change mode for other users");
		errorMap[ERR_UMODEUNKNOWNFLAG]	= IrcErrorInfo("501", "Unknown MODE flag");
		errorMap[ERR_UNKNOWNMODE]			= IrcErrorInfo("472","is unknown mode char to me");  // "<client> <modechar> :is unknown mode char to me"
		errorMap[ERR_CHANOPRIVSNEEDED]		= IrcErrorInfo("482", "You're not channel operator"); //"<client> <channel> :You're not channel operator"
// WELCOME

		errorMap[RPL_WELCOME]			= IrcErrorInfo("1","");
		errorMap[RPL_YOURHOST]			= IrcErrorInfo("2","");
		errorMap[RPL_CREATED]			= IrcErrorInfo("3",""); // <client> :This server was created <datetime>" eg.: ':server 003 <nick> :This server was created Thu Aug 7 18:41:14 2025'
		errorMap[RPL_MYINFO]			= IrcErrorInfo("4",""); // <client> <servername> <version> <available user modes> <available channel modes> [<channel modes with a parameter>]"

// errorMap[RPL]
		errorMap[RPL_CHANNELMODEIS]		= IrcErrorInfo("324", "");
		errorMap[RPL_CREATIONTIME]		= IrcErrorInfo("329", ""); //RPL_CREATIONTIME: :server 329 <nick> #channel 1723124876

		errorMap[RPL_UMODEIS]			= IrcErrorInfo("221", ""); // "<client> <user modes>"
		//
		errorMap[RPL_TOPIC]				= IrcErrorInfo("332", "");
		errorMap[RPL_NAMREPLY]			= IrcErrorInfo("353", "");
		errorMap[RPL_ENDOFNAMES]		= IrcErrorInfo("366", "End of /NAMES list");
		errorMap[RPL_INVITING]			= IrcErrorInfo("341", "");
// INVITE
		errorMap[ERR_USERNOTINCHANNEL]	= IrcErrorInfo("441", "They aren't on that channel");
		errorMap[ERR_NOTONCHANNEL]		= IrcErrorInfo("442", "You're not on that channel");
		errorMap[ERR_CHANOPRIVSNEEDED]	= IrcErrorInfo("482", "You're not channel operator");
		errorMap[ERR_USERONCHANNEL]		= IrcErrorInfo("443", "is already on channel");
// TOPIC
		errorMap[RPL_NOTOPIC]			= IrcErrorInfo("331", "No topic is set");
		errorMap[RPL_TOPIC]				= IrcErrorInfo("332", "");
		errorMap[RPL_TOPICWHOTIME]		= IrcErrorInfo("333", "");
		// ...
	}
	return errorMap;
}

// // input string must be uppercase
// MessageType parseCommandType(const std::string& type)
// {
// 	static std::map<std::string, MessageType> command_map;

// 	if (command_map.empty())
//     {
//         command_map["NICK"] = NICK;
//         command_map["USER"] = USER;
//         command_map["PING"] = PING;
//         command_map["PONG"] = PONG;
//         command_map["QUIT"] = QUIT;
//     }
// 	std::map<std::string, MessageType>::const_iterator it = command_map.find(type);
//     if (it != command_map.end())
//         return it->second;
//     return UNKNOWN;
// }

// std::ostream& operator<<(std::ostream& os, const MessageType& type)
// {
// 	switch (type)
// 	{
// 		case NICK:
// 			os << "NICK";
// 			break;
// 		case USER:
// 			os << "USER";
// 			break;
// 		case PING:
// 			os << "PING";
// 			break;
// 		case PONG:
// 			os << "PONG";
// 			break;
// 		case QUIT:
// 			os << "QUIT";
// 			break;
// 		case UNKNOWN:
// 			os << "UNKNOWN";
// 			break;
// 		default:
// 			// Output the enum value as a 3-digit number (001-999)
// 			os.width(3);
// 			os.fill('0');
// 			os << static_cast<int>(type);
// 			break;
// 	}
// 	return os;
// }
