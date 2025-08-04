#ifndef MESSAGE_TYPE
# define MESSAGE_TYPE

# include <string>

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
	RPL_WELCOME = 1,
	// https://modern.ircdocs.horse/#rplyourhost-002
	RPL_YOURHOST = 2,
	// https://modern.ircdocs.horse/#errunknownerror-400
	ERR_UNKNOWNERROR = 400,
	// https://modern.ircdocs.horse/#errnosuchnick-401
	ERR_NOSUCHNICK = 401,
	// https://modern.ircdocs.horse/#errnotregistered-451
	ERR_NOTREGISTERED = 451,
	// https://modern.ircdocs.horse/#errneedmoreparams-461
	ERR_NEEDMOREPARAMS = 461,
	// https://modern.ircdocs.horse/#erralreadyregistered-462
	ERR_ALREADYREGISTERED = 462,
	ERR_NONICKNAMEGIVEN = 431
};

MessageType parseCommandType(const std::string& type);
std::ostream& operator<<(std::ostream& os, const MessageType& type);

#endif
