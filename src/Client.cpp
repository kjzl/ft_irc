#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Debug.hpp"
#include "../include/Message.hpp"
#include "../include/MessageQueueManager.hpp"
#include "../include/MessageType.hpp"
#include "../include/Server.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>

Client::Client(MessageQueueManager &queueManager)
	: mqr_(queueManager), registrationLevel_(0), socket_(-1), nickname_(""),
	  username_("*"), realname_(""), rawMessage_("") {}

Client::Client(const Client &other) : mqr_(other.mqr_)
{
	*this = other;
}

Client &Client::operator=(const Client &other)
{
    if (this != &other) {
		clearMessage();
        this->registrationLevel_ = other.registrationLevel_;
        this->nickname_ = other.nickname_;
        this->username_ = other.username_;
        this->realname_ = other.realname_;
		this->rawMessage_ = other.rawMessage_;
        this->socket_ = other.socket_;
		this->IP_ = other.IP_;
    }
    return *this;
}

Client::~Client()
{}

bool	Client::operator==(const std::string nickname)
{
	if (getNickname() == nickname)
		return (true);
	return (false);
}

bool	Client::operator==(const Client &client)
{
	if (getSocket() == client.getSocket())
		return (true);
	return (false);
}

// Getters

bool Client::isAuthenticated() const
{
    return (registrationLevel_ == 3);
}

const std::string &Client::getNickname() const
{
    return nickname_;
}

const std::string &Client::getUsername() const
{
    return username_;
}

const std::string &Client::getRealname() const
{
    return realname_;
}

int Client::getSocket() const
{
    return socket_;
}

const std::string &Client::getRawMessage() const
{
    return rawMessage_;
}

const std::string &Client::getIP() const
{
    return IP_;
}

void Client::clearMessage()
{
	rawMessage_.clear();
}


void Client::incrementRegistrationLevel(void)
{
    ++registrationLevel_;
}

int Client::getRegistrationLevel(void) const
{
    return (registrationLevel_);
}

// Setters
void Client::setNickname(const std::string &nickname)
{
    nickname_ = nickname;
}

void Client::setUsername(const std::string &username)
{
    username_ = username;
}

void Client::setRealname(const std::string &realname)
{
    realname_ = realname;
}

void Client::setSocket(int socket)
{
    socket_ = socket;
}


void Client::setRawMessage(const std::string &rawMessage)
{
    rawMessage_ = rawMessage;
}

void Client::setIP(const std::string &IP)
{
    IP_ = IP;
}

void Client::appendRawMessage(const char partialMessage[BUFSIZ], size_t length)
{
	rawMessage_ += std::string(partialMessage, length);
}

void Client::sendMessage(Message toSend) const {
	mqr_.send(this->getSocket(), toSend.toString());
}

bool	Client::sendMessageTo(Message msg, const std::string recipientNickname, Server &server) const
{
	msg.setSource(*this);
	std::vector<Client> clients = server.getClients();
	std::vector<Client>::iterator clientIt = std::find(clients.begin(), clients.end(), recipientNickname);
	if (clientIt != clients.end())
	{
		clientIt->sendMessage(msg);
		return (true);
	}
	return (false);
}

void Client::sendErrorMessage(MessageType type, std::vector<std::string>& args) const
{
	static std::map<MessageType, IrcErrorInfo> ErrorMap = getErrorMap();
    IrcErrorInfo info = ErrorMap.find(type)->second;
	if (!info.message.empty())
		args.push_back(info.message);
    Message outMessage(info.code,  args);
	outMessage.setSource();
	debug("sending error MSG: " + outMessage.toString());
    sendMessage(outMessage);
}

// void Client::sendErrorMessage(MessageType type, std::string args[], int size) const
// {
// 	std::vector<std::string> outParams(args, args + size);
// 	sendErrorMessage(type, outParams);
// }

void Client::sendErrorMessage(MessageType type, std::string arg1) const
{
	std::vector<std::string> outParams;
	outParams.push_back(arg1);
	sendErrorMessage(type, outParams);
}

void Client::sendErrorMessage(MessageType type, std::string arg1, std::string arg2) const
{
	std::vector<std::string> outParams;
	outParams.push_back(arg1);
	outParams.push_back(arg2);
	sendErrorMessage(type, outParams);
}

void Client::sendErrorMessage(MessageType type, std::string arg1, std::string arg2, std::string arg3) const
{
	std::vector<std::string> outParams;
	outParams.push_back(arg1);
	outParams.push_back(arg2);
	outParams.push_back(arg3);
	sendErrorMessage(type, outParams);
}

void Client::sendErrorMessage(MessageType type, std::string arg1, std::string arg2, std::string arg3, std::string arg4) const
{
	std::vector<std::string> outParams;
	outParams.push_back(arg1);
	outParams.push_back(arg2);
	outParams.push_back(arg3);
	outParams.push_back(arg4);
	sendErrorMessage(type, outParams);
}

void Client::sendCmdValidation(const Message inMessage) const {
	Message outMessage(inMessage);
	outMessage.setSource(*this);
	sendMessage(outMessage);
}

void Client::sendCmdValidation(const Message inMessage, const Channel &channel) const
{
	sendCmdValidation(inMessage);
	Message outMessage(inMessage);
	outMessage.setSource(*this);
	channel.broadcastMsg(*this, outMessage);
}
