#ifndef MESSAGE_TYPE
# define MESSAGE_TYPE

# include <string>
# include <map>

enum MessageType {
	UNKNOWN,

	/*
	CLIENT MESSAGES
	*/

	// https://modern.ircdocs.horse/#nick-message
	NICK,
	// https://modern.ircdocs.horse/#user-message
	USER,
	// https://modern.ircdocs.horse/#ping-message
	PING,
	// https://modern.ircdocs.horse/#pong-message
	PONG,
	// https://modern.ircdocs.horse/#quit-message
	QUIT,

	/*
	NUMERICS
	*/

	// https://modern.ircdocs.horse/#rplwelcome-001
	RPL_WELCOME,
	// https://modern.ircdocs.horse/#rplyourhost-002
	RPL_YOURHOST,
	RPL_CREATED, // "<client> :This server was created <datetime>"
	RPL_MYINFO, // "<client> <servername> <version> <available user modes> <available channel modes> [<channel modes with a parameter>]"
	// https://modern.ircdocs.horse/#error-message
	ERROR,
	// https://modern.ircdocs.horse/#errunknownerror-400
	ERR_UNKNOWNERROR,
	// https://modern.ircdocs.horse/#errnotregistered-451
	ERR_NOTREGISTERED,
	// https://modern.ircdocs.horse/#errneedmoreparams-461
	ERR_NEEDMOREPARAMS,
	// https://modern.ircdocs.horse/#erralreadyregistered-462
	ERR_ALREADYREGISTERED,

	ERR_UNKNOWNCOMMAND,
	ERR_NONICKNAMEGIVEN,
	ERR_ERRONEUSNICKNAME,
	ERR_NICKNAMEINUSE,
    ERR_PASSWDMISMATCH,

	// https://modern.ircdocs.horse/#errnosuchnick-401
	ERR_NOSUCHNICK,
	ERR_NOSUCHCHANNEL,
	ERR_CANNOTSENDTOCHAN,
	ERR_NORECIPIENT,
	ERR_NOTEXTTOSEND,
	ERR_BADCHANNELKEY,
	ERR_INVITEONLYCHAN,
	RPL_TOPIC,
	RPL_NAMREPLY,
	RPL_ENDOFNAMES,
	RPL_INVITING,
    ERR_NOTONCHANNEL,
    ERR_USERONCHANNEL,
	//https://modern.ircdocs.horse/#rplumodeis-221
	RPL_UMODEIS,
	ERR_USERSDONTMATCH,
	ERR_UMODEUNKNOWNFLAG,
	ERR_UNKNOWNMODE,
	RPL_CHANNELMODEIS,
	RPL_CREATIONTIME,
	ERR_CHANOPRIVSNEEDED,
	ERR_USERNOTINCHANNEL,
	RPL_NOTOPIC,
	ERR_CHANNELISFULL
};

MessageType parseCommandType(const std::string& type);
std::ostream& operator<<(std::ostream& os, const MessageType& type);

struct IrcErrorInfo
{
	std::string code;
	std::string message;

	IrcErrorInfo() {}
	IrcErrorInfo(const std::string& c, const std::string& m) : code(c), message(m) {}
};

const std::map<MessageType, IrcErrorInfo>& getErrorMap();

#endif
