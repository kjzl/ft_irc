# Sockets

## Sources

- https://beej.us/guide/bgnet/html/

#### create a socket


	struct	sockaddr_in	sa = {0};

	sa.sin_family = AF_INET;
	sa.sin_port = htons(getPort());
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
the above struct is the standart struct for IPv4.
using [INADDR_ANY](https://stackoverflow.com/questions/53808966/binding-with-inaddr-any-vs-inaddr-loopback-in-development) means anyone on the network on the machine, which is fine for me.
using INADDR_LOOPBACK would mean you can only connect from this very mashine it's running on.
```C++
	setServerSocket(socket(sa.sin_family, SOCK_STREAM, 0));
```
socket works like this:
int socket(int domain, int type, int protocol);
and I am leaving 0 as the protocol, as it is [reccomended](https://www.ibm.com/docs/en/zvm/7.2.0?topic=rsf-socket) and defaults to TCP/IP assigning the default protocol for the domain.
(it would be IPPROTO_TCP as I am using a stream socket, but it's better to leave that for TCP/IP)


#### poll

[beej](https://beej.us/guide/bgnet/html/#poll)
```C++
struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // on return, bitmap of events that occurred
};
```
our clients should be listened for POLLIN | POLLHUP

how to check if an event happened?
```C++
			if (((pollFds_[pollIndex].revents & POLLIN) == 1))
```
the return events will be set by poll
