#include <string>
#include <vector>
#include <exception>
#include "MessageType.hpp"

class Message {
public:
	Message(const Message& other);
	virtual ~Message();
	static Message parseMessage(const std::string& raw);

	// Optional note of where the message came from
	const std::string* getSource() const;
	// The specific command this message represents.
	MessageType getType() const;
	// If it exists, data relevant to this specific command.
	const std::vector<std::string>& getParams() const;

private:
	const std::string* source_;
	const MessageType type_;
	const std::vector<std::string> params_;

	Message(MessageType type, std::vector<std::string> params);
	Message(const std::string& source, MessageType type, std::vector<std::string> params);

	class UnknownMessageTypeException : public std::exception {
	private:
		std::string type_;

	public:
		UnknownMessageTypeException(const std::string& type);
		const char* what() const noexcept;
	};

	class WrongMessageFormatException : public std::exception {
	private:
		std::string message_;

	public:
		WrongMessageFormatException(const std::string& message);
		const char* what() const noexcept;
	};
};
