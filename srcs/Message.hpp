#include <string>
#include <vector>
#include "MessageType.hpp"
#include "Nickname.hpp"

class Message {
	private:
		// tags; TODO
		// Optional note of where the message came from, starting with (':', 0x3A).
		const Nickname* source;
		// The specific command this message represents.
		const MessageType type;
		// If it exists, data relevant to this specific command.
		const std::vector<std::string> params;

	protected:
		Message(const std::string& source, MessageType type, std::vector<std::string> params);

	public:
		static Message parseMessage(const std::string& msg);

	// UnknownMessageTypeException
};
