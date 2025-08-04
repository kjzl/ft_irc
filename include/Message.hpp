#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <exception>
// #include "MessageType.hpp"

class Client;

class Message {
public:
	Message(const Message& other);
	Message(std::string type, const std::vector<std::string>& params);
	Message(std::string type, const std::string& source, const std::vector<std::string>& params);
	Message(std::string type, const Client& source, const std::vector<std::string>& params);
	Message(std::string type, const std::string& arg1);
	Message(std::string type, const std::string& arg1, const std::string& arg2);
	Message(std::string type, const std::string& arg1, const Client& source);
	Message(std::string type, const std::string& arg1, const std::string& arg2, const Client& source);
	Message(const std::string &msg);
	virtual ~Message();
	std::string	toString() const;

	// Optional note of where the message came from
	const std::string* getSource() const;
	// The specific command this message represents.
	std::string getType() const;
	// If it exists, data relevant to this specific command.
	const std::vector<std::string>& getParams() const;

	class WrongMessageFormatException : public std::exception {
	private:
		std::string message_;

	public:
		WrongMessageFormatException(const std::string& message);
		~WrongMessageFormatException() throw();
		const char* what() const throw();
	};
private:
	std::string* 				source_; //TODO: why a pointer ????
	std::string					type_;
	std::vector<std::string>	params_;

	void parseIncomingMessage(const std::string& raw);

};

std::ostream& operator<<(std::ostream& os, const Message& message);

#endif // MESSAGE_HPP
