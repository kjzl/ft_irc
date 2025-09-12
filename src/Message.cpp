#include "../include/Message.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include <cctype>
#include <sstream>

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

std::ostream &operator<<(std::ostream &os, const Message &message) {
  os << message.toString();
  return os;
}

//: nickname!username@host(or IP) type ... ... :back
std::string Message::toString() const {
  std::string msg;
  const std::vector<std::string> &params = getParams();
  // source prefix
  if (hasSource_) {
    msg += ":";
    if (!nickname_.empty())
      msg += nickname_ + "!" + username_ + "@";
    msg += hostname_ + " ";
  }
  msg += type_;
  if (params.empty())
    return msg;
  else
    msg += " ";
  for (size_t i = 0; i < params.size() - 1; i++)
    msg += params[i] + " ";
  const std::string &lastParam = params.back();
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
	std::istringstream iss(msg);
	std::string token;
	bool lastParam = false;

	// Reset defaults
	hasSource_ = false;
	nickname_.clear();
	username_.clear();
	// hostname_ kept default
	params_.clear();

	// Optional prefix
	if (iss.peek() == ':') {
		iss.get(); // consume ':'
		std::string prefix;
		std::getline(iss, prefix, ' ');
		// prefix may be of forms: nick!user@host or server.name
		hasSource_ = true;
		hostname_.clear();
		std::string::size_type excl = prefix.find('!');
		std::string::size_type at = prefix.find('@');
		if (excl != std::string::npos && at != std::string::npos && excl < at) {
			nickname_ = prefix.substr(0, excl);
			username_ = prefix.substr(excl + 1, at - excl - 1);
			hostname_ = prefix.substr(at + 1);
		} else {
			// server-only prefix
			nickname_.clear();
			username_.clear();
			hostname_ = prefix;
		}
		// eat any extra spaces
		while (iss.peek() == ' ') iss.get();
	}

	// Command
	iss >> token;
	type_ = token;

	// Params
	while (iss >> token)
	{
		if (lastParam)
			params_.back() += " " + token;
		else if (!token.empty() && token[0] == ':')
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

// void Message::setSource(const std::string nickname, const std::string username)
// {
// 	hasSource_ = true;
// 	nickname_ = nickname;
// 	username_ = username;
// }

void Message::setSource(const Client &client)
{
	hasSource_ = true;
	nickname_ = client.getNickname();
	username_ = client.getUsername();
	hostname_ = client.getIP();
}

std::vector<std::string> &Message::getParams() { return params_; }

const std::vector<std::string> &Message::getParams() const { return params_; }

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
