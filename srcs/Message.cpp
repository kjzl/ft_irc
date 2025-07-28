#include "Message.hpp"

// this message must not end with crlf
static Message parseMessage(const std::string& msg)
{
	std::string::const_iterator	it = msg.cbegin();
	std::string::const_iterator	end = msg.cend();
	std::string					tmp;
	Nickname*					nick;
	MessageType					type;
	std::vector<std::string>	params;

	if (*it == '@') {
		// parse tags and move iterator along
	}
	if (*it == ':') {
		// TODO nickname class?
		it++;
		while (it != end && !isspace(*it))
		{
			tmp += *it;
		}
		nick = new Nickname(tmp);
		tmp = "";
	}
	while (it != end && !isspace(*it))
	{
		tmp += *it;
	}
	type = parseClientMessageType(tmp);
	if (type == MessageType::UNKNOWN) {
		// TODO UnknownMessageTypeException
	}
	while (it != end)
	{
		tmp = "";
		if (*it == ':')
		{
			it++;
			while (it != end)
			{
				tmp += *it;
			}
		}
		else
		{
			while (!isspace(*it) && it != end)
			{
				tmp += *it;
			}
		}
		params.push_back(tmp);
	}

}
