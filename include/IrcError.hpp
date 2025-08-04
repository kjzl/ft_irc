#ifndef IRCERROR_HPP
#define IRCERROR_HPP

#include <string>
#include <map>

enum class IrcError
{
    ERR_NONICKNAMEGIVEN,
    ERR_ERRONEUSNICKNAME,
    ERR_NICKNAMEINUSE
};


struct IrcErrorInfo
{
    std::string code;
    std::string message;
};

const std::map<IrcError, IrcErrorInfo> ErrorMap =    // default error text
{
    {IrcError::ERR_NONICKNAMEGIVEN,     {"431", ":No nickname given"} },
    {IrcError::ERR_ERRONEUSNICKNAME,    {"432", ":Erroneous nickname"} },
    {IrcError::ERR_NICKNAMEINUSE,       {"433", ":Nickname is already in use" }}
    // ...
};

#endif
