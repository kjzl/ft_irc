#include "IrcError.hpp"

const std::map<IrcError, IrcErrorInfo>& getErrorMap()
{
	static std::map<IrcError, IrcErrorInfo> errorMap;

	if (errorMap.empty())
	{
		errorMap[ERR_NONICKNAMEGIVEN]  = IrcErrorInfo("431", ":No nickname given");
		errorMap[ERR_ERRONEUSNICKNAME] = IrcErrorInfo("432", ":Erroneous nickname");
		errorMap[ERR_NICKNAMEINUSE]    = IrcErrorInfo("433", ":Nickname is already in use");
		// ...
	}

	return errorMap;
}