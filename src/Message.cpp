#include "../include/Message.hpp"
#include "../include/ircUtils.hpp"
#include "Server.hpp"
#include <cctype>
#include <sstream>
#include "Client.hpp"

Message::Message()
{}

Message::Message(const std::string &msg)
: hasSource_(false), nickname_(""), username_(""), hostname_(HOSTNAME), type_(""), params_()
{
	parseIncomingMessage(msg);
}

Message::Message(std::string type, const std::vector<std::string>& params)
	: hasSource_(false), nickname_(""), username_(""), hostname_(HOSTNAME), type_(type), params_(params)
{
}


Message::Message(std::string type, const std::string& nickname, const std::string& username, const std::vector<std::string>& params)
	: hasSource_(true), nickname_(nickname), username_(username), hostname_(HOSTNAME), type_(type), params_(params)
{
}

Message::Message(std::string type, const Client& source, const std::vector<std::string>& params)
	: hasSource_(true), nickname_(source.getNickname()), username_(source.getUsername()), hostname_(HOSTNAME), type_(type), params_(params)
{
}

Message::Message(std::string type, const std::string& arg1)
	: hasSource_(false), nickname_(""), username_(""), hostname_(HOSTNAME), type_(type), params_(1, arg1)
{
}

Message::Message(std::string type, const std::string& arg1, const std::string& arg2)
	: hasSource_(false), nickname_(""), username_(""), hostname_(HOSTNAME), type_(type), params_()
{
	params_.push_back(arg1);
	params_.push_back(arg2);
}

Message::Message(std::string type, const std::string& arg1, const Client& source)
	: hasSource_(true), nickname_(source.getNickname()), username_(source.getUsername()), hostname_(HOSTNAME), type_(type), params_()
{
	params_.push_back(arg1);
}

Message::Message(std::string type, const std::string& arg1, const std::string& arg2, const Client& source)
	: hasSource_(true), nickname_(source.getNickname()), username_(source.getUsername()), hostname_(HOSTNAME), type_(type), params_()
{
	params_.push_back(arg1);
	params_.push_back(arg2);
}

Message::Message(const Message& other)
	:	hasSource_(other.hasSource_),
		nickname_(other.nickname_),
		username_(other.username_),
		hostname_(other.hostname_),
		type_(other.type_),
		params_(other.params_)
{
}

std::ostream& operator<<(std::ostream& os, Message& message)
{
	os << message.toString();
	return os;
}

//:nickname!username@host(or IP) type ... ... :back
std::string	Message::toString()
{
	std::string msg;
	std::string	lastParam = getParams().back();
	if (hasSource_)
	{
		msg += ":";
		if (!nickname_.empty())
			msg += nickname_ + "!" + username_ + "@" ;
		msg += hostname_ + " ";
	}
	msg += type_;
	if (getParams().empty())
		return msg;
	else
		msg += " ";
	for (size_t i = 0; i < getParams().size() - 1; i++)
		msg += getParams()[i] + " ";
	if (lastParam.find(" ") != std::string::npos)
		msg += ":";
	msg += lastParam + "\r\n";
	return (msg);
}

Message::~Message()
{}

// input message must not end with crlf
void Message::parseIncomingMessage(const std::string& msg)
{
	std::istringstream			iss(msg);
	std::string					token;
	bool 						lastParam = false;

	iss >> token;
	// client messages cant have a source and we do not support tags
	//TODO: this will stop running the server if one typed a Message beginning with @. that is not intended. Instead make a reply that works
	if (token[0] == ':' || token[0] == '@')
		throw WrongMessageFormatException("Message must not include a source or tag.");
	type_ = token;
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

const std::string Message::getUsername() const
{
	return username_;
}

const std::string Message::getNickname() const
{
	return nickname_;
}

std::string Message::getType() const
{
	return type_;
}

// only sets servername as source
void Message::setSource()
{
	hasSource_ = true;
}

void Message::setSource(const std::string nickname, const std::string username)
{
	hasSource_ = true;
	nickname_ = nickname;
	username_ = username;
}

std::vector<std::string>& Message::getParams()
{
	return params_;
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
