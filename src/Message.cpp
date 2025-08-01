#include "../include/Message.hpp"
#include "../include/ircUtils.hpp"
#include <cctype>
#include <sstream>
#include "Client.hpp"

Message::Message(const std::string &msg)
: source_(NULL), type_(UNKNOWN), params_()
{
	parseIncomingMessage(msg);
}

Message::Message(MessageType type, const std::vector<std::string>& params)
	: source_(NULL), type_(type), params_(params)
{
}

Message::Message(MessageType type, const std::vector<std::string>& params, const std::string& source)
	: source_(source.empty() ? NULL : new std::string(source)), type_(type), params_(params)
{
}

Message::Message(MessageType type, const std::vector<std::string>& params, const Client& source)
	: source_(source.getNickname().empty() ? NULL : new std::string(source.getNickname())), type_(type), params_(params)
{
}

Message::Message(MessageType type, const std::string& arg1)
	: source_(NULL), type_(type), params_(1, arg1)
{
}

Message::Message(MessageType type, const std::string& arg1, const std::string& arg2)
	: source_(NULL), type_(type), params_()
{
	params_.push_back(arg1);
	params_.push_back(arg2);
}

Message::Message(MessageType type, const std::string& arg1, const Client& source)
	: source_(source.getNickname().empty() ? NULL : new std::string(source.getNickname())), type_(type), params_()
{
	params_.push_back(arg1);
}

Message::Message(MessageType type, const std::string& arg1, const std::string& arg2, const Client& source)
	: source_(source.getNickname().empty() ? NULL : new std::string(source.getNickname())), type_(type), params_()
{
	params_.push_back(arg1);
	params_.push_back(arg2);
}

Message::Message(const Message& other)
	: source_(other.source_ ? new std::string(*other.source_) : NULL),
	  type_(other.type_),
	  params_(other.params_)
{
}

std::ostream& operator<<(std::ostream& os, const Message& message)
{
	os << message.toString();
	return os;
}

std::string	Message::toString() const
{
	std::string msg;
	if (source_)
		msg += ":" + *source_ + " ";
	msg += getTypeAsString();
	if (getParams().empty())
		return msg;
	else
		msg += " ";
	for (size_t i = 0; i < getParams().size() - 1; i++)
		msg += getParams()[i] + " ";
	msg += ":" + getParams().back() + "\r\n";
	return (msg);
}

Message::~Message()
{
	delete source_;
}

// input message must not end with crlf
void Message::parseIncomingMessage(const std::string& msg)
{
	std::istringstream			iss(msg);
	std::string					token;
	bool 						lastParam = false;

	iss >> token;
	if (token.empty())
		throw WrongMessageFormatException("Message must not be empty.");
	// client messages cant have a source and we do not support tags
	if (token[0] == ':' || token[0] == '@')
		throw WrongMessageFormatException("Message must not include a source or tag.");
	type_ = parseCommandType(token);
	if (type_ == UNKNOWN)
		throw UnknownMessageTypeException(token);
	while (iss >> token)
	{
		if (lastParam)
			params_.back() += " " + token;
		else if (token[0] == ':')
		{
			lastParam = true;
			params_.push_back(token.substr(1));
		}
		else
			params_.push_back(token);
	}
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
