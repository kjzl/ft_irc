#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <exception>
// #include "MessageType.hpp"

class Client;

class Message {
public:
  Message();
  Message(const Message &other);
  Message(std::string type, const std::vector<std::string> &params);
  Message(std::string type, const Client &source,
          const std::vector<std::string> &params);
  Message(std::string type, const std::string &arg1);
  Message(std::string type, const std::string &arg1, const std::string &arg2);
  Message(std::string type, const std::string &arg1, const Client &source);
  Message(std::string type, const std::string &arg1, const std::string &arg2,
          const Client &source);
  Message(const std::string &msg);
  virtual ~Message();
  std::string toString() const;

	// Optional note of where the message came from
	const std::string	getNickname() const;
	const std::string	getUsername() const;
	// void	setSource(const std::string nickname, const std::string username);
	void	setSource();
	void	setSource(const Client &client);
	// The specific command this message represents.
	std::string getType() const;
	// If it exists, data relevant to this specific command.
	std::vector<std::string>&	getParams();
	const std::vector<std::string>	&getParams() const;

	class WrongMessageFormatException : public std::exception {
	private:
		std::string message_;

	public:
		WrongMessageFormatException(const std::string& message);
		~WrongMessageFormatException() throw();
		const char* what() const throw();
	};
private:
	bool 						hasSource_;
	std::string					nickname_;
	std::string					username_;
	std::string			hostname_;
	std::string					type_;
	std::vector<std::string>	params_;

	void parseIncomingMessage(const std::string& raw);
};

std::ostream& operator<<(std::ostream& os, const Message& message);

#endif
