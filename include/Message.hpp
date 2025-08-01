#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <exception>
#include "MessageType.hpp"

class Message {
public:
	Message(const Message& other);
	Message(MessageType type, const std::vector<std::string>& params);
	Message(const std::string& source, MessageType type, const std::vector<std::string>& params);
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
	const std::string* source_;
	const MessageType type_;
	const std::vector<std::string> params_;

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

#endif // MESSAGE_HPP
