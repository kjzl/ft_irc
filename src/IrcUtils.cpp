#include "IrcUtils.hpp"
#include <sys/socket.h>

// sends the entire string with send() even when more than one send() call is needed
// throws and error if send fails
void	safeSend(int socket, const std::string &string)
{
	int sendBytes;
	int	total_sent = 0;
	int	left_size = string.size();

	while (left_size)
	{
		sendBytes = send(socket, string.substr(total_sent, left_size).c_str(), left_size, 0);
		if (sendBytes == -1)
			throw std::runtime_error("[Server] send error with client: " + toString(socket));
		total_sent += sendBytes;
		left_size -= sendBytes;
	}
}
