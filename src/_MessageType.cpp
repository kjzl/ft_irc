#include <map>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include "MessageType.hpp"

// input string must be uppercase
MessageType parseCommandType(const std::string& type)
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
	std::map<std::string, MessageType>::const_iterator it = command_map.find(type);
    if (it != command_map.end())
        return it->second;
    return UNKNOWN;
}

std::ostream& operator<<(std::ostream& os, const MessageType& type)
{
	switch (type)
	{
		case NICK:
			os << "NICK";
			break;
		case USER:
			os << "USER";
			break;
		case PING:
			os << "PING";
			break;
		case PONG:
			os << "PONG";
			break;
		case QUIT:
			os << "QUIT";
			break;
		case UNKNOWN:
			os << "UNKNOWN";
			break;
		default:
			// Output the enum value as a 3-digit number (001-999)
			os.width(3);
			os.fill('0');
			os << static_cast<int>(type);
			break;
	}
	return os;
}

