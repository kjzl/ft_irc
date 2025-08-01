#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <exception>
#include "MessageType.hpp"
#include "Client.hpp"

class Message {
public:
	Message(const Message& other);
	Message(MessageType type, const std::vector<std::string>& params);
	Message(MessageType type, const std::vector<std::string>& params, const std::string& source);
	Message(MessageType type, const std::vector<std::string>& params,  const Client& source);
	Message(MessageType type, const std::string& arg1);
	Message(MessageType type, const std::string& arg1, const std::string& arg2);
	Message(MessageType type, const std::string& arg1, const Client& source);
	Message(MessageType type, const std::string& arg1, const std::string& arg2, const Client& source);
	virtual ~Message();
	static Message parseIncomingMessage(const std::string& raw);

	// Optional note of where the message came from
	const std::string* getSource() const;
	// The specific command this message represents.
	MessageType getType() const;
	std::string getTypeAsString() const;
	// If it exists, data relevant to this specific command.
	const std::vector<std::string>& getParams() const;

private:
	std::string* source_;
	MessageType type_;
	std::vector<std::string> params_;

	std::string	toString() const;

	class UnknownMessageTypeException : std::exception {
	private:
		std::string type_;

	public:
		UnknownMessageTypeException(const std::string& type);
		~UnknownMessageTypeException() throw();
		const char* what() const throw();
	};

	class WrongMessageFormatException : std::exception {
	private:
		std::string message_;

	public:
		WrongMessageFormatException(const std::string& message);
		~WrongMessageFormatException() throw();
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream& os, const Message& message);

#endif // MESSAGE_HPP
