#include "../include/Message.hpp"
#include "../include/ircUtils.hpp"
#include <cctype>
#include <sstream>

Message::Message(MessageType type, const std::vector<std::string>& params)
	: source_(NULL), type_(type), params_(params)
{
}

Message::Message(const std::string& source, MessageType type, const std::vector<std::string>& params)
: source_(source.empty() ? NULL : new std::string(source)), type_(type), params_(params)
{
}

Message::Message(const Message& other)
	: source_(other.source_ ? new std::string(*other.source_) : NULL),
	  type_(other.type_),
	  params_(other.params_)
{
}

std::ostream& operator<<(std::ostream& os, const Message& message)
{
	os << message.getTypeAsString() << " ";
	if (message.getParams().empty())
		return os;
	for (size_t i = 0; i < message.getParams().size() - 1; i++)
		os << message.getParams()[i] << " ";
	os << ":" << message.getParams().back();
	return os;
}

std::string	Message::toString() const
{
	std::string msg;
	msg = getTypeAsString() + " ";
	if (getParams().empty())
		return msg;
	for (size_t i = 0; i < getParams().size() - 1; i++)
		msg += getParams()[i] + " ";
	msg += ":" + getParams().back();
	return (msg);
}

Message::~Message()
{
	delete source_;
}

// input message must not end with crlf
Message Message::parseIncomingMessage(const std::string& msg)
{
	std::istringstream			iss(msg);
	std::string					token;
	MessageType					type;
	std::vector<std::string>	params;
	bool 						lastParam = false;

	iss >> token;
	if (token.empty())
		throw WrongMessageFormatException("Message must not be empty.");
	// client messages cant have a source and we do not support tags
	if (token[0] == ':' || token[0] == '@')
		throw WrongMessageFormatException("Message must not include a source or tag.");
	type = parseCommandType(token);
	if (type == UNKNOWN)
		throw UnknownMessageTypeException(token);
	while (iss >> token)
	{
		if (lastParam)
			params.back() += " " + token;
		else if (token[0] == ':')
		{
			lastParam = true;
			params.push_back(token.substr(1));
		}
		else
			params.push_back(token);
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

std::string Message::getTypeAsString() const
{
	return ::toString(type_);
}

const std::vector<std::string>& Message::getParams() const
{
	return params_;
}

Message::UnknownMessageTypeException::UnknownMessageTypeException(const std::string& type)
	: type_("\"" + type + "\" is not a valid message/command type.")
{
}

Message::UnknownMessageTypeException::~UnknownMessageTypeException() throw()
{
}

const char * Message::UnknownMessageTypeException::what() const throw()
{
	return type_.c_str();
}

Message::WrongMessageFormatException::WrongMessageFormatException(const std::string& message)
	: message_(message)
{
}

const char * Message::WrongMessageFormatException::what() const throw()
{
	return message_.c_str();
}

Message::WrongMessageFormatException::~WrongMessageFormatException() throw()
{
}
