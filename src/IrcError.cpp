#include "IrcError.hpp"

const std::map<IrcError, IrcErrorInfo>& getErrorMap()
{
	static std::map<IrcError, IrcErrorInfo> errorMap;

	if (errorMap.empty())
	{
		errorMap[ERR_NONICKNAMEGIVEN]  = IrcErrorInfo("431", ":No nickname given");
		errorMap[ERR_ERRONEUSNICKNAME] = IrcErrorInfo("432", ":Erroneous nickname");
		errorMap[ERR_NICKNAMEINUSE]    = IrcErrorInfo("433", ":Nickname is already in use");
        errorMap[ERR_NEEDMOREPARAMS]   = IrcErrorInfo("461", ":Not enough parameters");
        errorMap[ERR_ALREADYREGISTERED]= IrcErrorInfo("462", ":You may not reregister");
        errorMap[ERR_PASSWDMISMATCH]   = IrcErrorInfo("464", ":Password incorrect");
		// ...
	}

	return errorMap;
}
