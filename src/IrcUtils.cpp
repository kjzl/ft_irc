#include "../include/IrcUtils.hpp"
#include <sys/socket.h>
#include <stdexcept>

// sends the entire string with send() even when more than one send() call is needed
// throws and error if send fails
void safeSend(int fd, const std::string &msg)
{
	int sendBytes;
	int	total_sent = 0;
	int	left_size = msg.size();

	while (left_size)
	{
		sendBytes = send(fd, msg.substr(total_sent, left_size).c_str(), left_size, 0);
		if (sendBytes == -1)
        {
			throw std::runtime_error("[Server] send error with client: " + toString(fd));
        }
		total_sent += sendBytes;
		left_size -= sendBytes;
	}
}
