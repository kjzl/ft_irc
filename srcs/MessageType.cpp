#include <map>
#include <cctype>
#include <stdexcept>
#include "StringHelpers.hpp"
#include "MessageType.hpp"

// input string must be uppercase
MessageType parseClientMessageType(const std::string& type)
{
	static std::map<std::string, MessageType> command_map;
	if (command_map.empty())
    {
        command_map["NICK"] = NICK;
        command_map["USER"] = USER;
        command_map["PING"] = PING;
        command_map["PONG"] = PONG;
        command_map["QUIT"] = QUIT;
    }

    if (!str_is_upper(type))
		throw std::logic_error("Input to parseClientMessageType must be uppercase.");

	std::map<std::string, MessageType>::const_iterator it = command_map.find(type);
    if (it != command_map.end())
        return it->second;
    return UNKNOWN;
}
