#include "Message.hpp"
#include <cctype>

Message::Message(MessageType type, std::vector<std::string> params)
	: source_(NULL), type_(type), params_(params)
{
}

Message::Message(const std::string& source, MessageType type, std::vector<std::string> params)
: source_(source.empty() ? NULL : new std::string(source)), type_(type), params_(params)
{
}

Message::Message(const Message& other)
	: source_(other.source_ ? new std::string(*other.source_) : NULL),
	  type_(other.type_),
	  params_(other.params_)
{
}

Message::~Message()
{
	delete source_;
}

// input message must not end with crlf
Message Message::parseMessage(const std::string& msg)
{
	std::string::const_iterator	it = msg.cbegin();
	std::string::const_iterator	end = msg.cend();
	std::string					tmp;
	MessageType					type;
	std::vector<std::string>	params;

	// client messages cant have a source and we do not support tags
	if (*it == ':' || *it == '@') {
		throw WrongMessageFormatException("Client must not send a source or tags.");
	}
	while (it != end && *it != ' ')
		tmp += *it++;
	type = parseClientMessageType(tmp);
	if (type == MessageType::UNKNOWN) {
		throw UnknownMessageTypeException(tmp);
	}
	while (true)
	{
		// skip whitespace
		while (it != end && *it == ' ')
			++it;
		if (it == end)
			break;
		tmp.clear();
		if (*it == ':')
		{
			while (++it != end)
				tmp += *it;
		}
		else
		{
			while (*it != ' ' && it != end)
				tmp += *it++;
		}
		params.push_back(tmp);
	}
	return Message(type, params);
}

const std::string* Message::getSource() const
{
	return source_;
}

MessageType Message::getType() const
{
	return type_;
}

const std::vector<std::string>& Message::getParams() const
{
	return params_;
}

Message::UnknownMessageTypeException::UnknownMessageTypeException(const std::string& type)
	: type("\"" + type + "\" is not a valid message/command type.")
{
}

const char * Message::UnknownMessageTypeException::what() const noexcept
{
	return type.c_str();
}

Message::WrongMessageFormatException::WrongMessageFormatException(const std::string& message)
	: message(message)
{
}

const char * Message::WrongMessageFormatException::what() const noexcept
{
	return message.c_str();
}
