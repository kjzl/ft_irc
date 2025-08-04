#ifndef IRCERROR_HPP
#define IRCERROR_HPP

#include <string>
#include <map>

enum IrcError
{
	ERR_NONICKNAMEGIVEN,
	ERR_ERRONEUSNICKNAME,
	ERR_NICKNAMEINUSE,
	ERR_NEEDMOREPARAMS,
    ERR_ALREADYREGISTERED,
    ERR_PASSWDMISMATCH
};

struct IrcErrorInfo
{
	std::string code;
	std::string message;

	IrcErrorInfo() {}
	IrcErrorInfo(const std::string& c, const std::string& m) : code(c), message(m) {}
};

const std::map<IrcError, IrcErrorInfo>& getErrorMap();

#endif

