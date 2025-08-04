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
$$int socket(int domain, int type, int protocol);$$
and I am leaving 0 as the protocol, as it is [reccomended](https://www.ibm.com/docs/en/zvm/7.2.0?topic=rsf-socket) and defaults to TCP/IP assigning the default protocol for the domain.
(it would be IPPROTO_TCP as I am using a stream socket, but it's better to leave that for TCP/IP)


BUUUT
that is the old way. let's go with the new, ipv6 compatible one.

[guide](https://beej.us/guide/bgnet/html/#getaddrinfoman)

#### bind a socket

sometimes, when you quit the server and then restart, you get a bind error:
==== STARTING SERVER ====
[Server] bind error: Address already in use

That happens not because of this program, but because of how TCP works.
There is something calles [TIME_WAIT](https://superuser.com/questions/173535/what-are-close-wait-and-time-wait-states). It ensures that enough time has passed for delayed packages to arrive and be discarded.

[short explanation of what happens on TIME_WAIT and CLOSE_WAIT](https://blog.csdn.net/fareast_mzh/article/details/146475207)

so i do this:
```c++
	int optval = 1;
	setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
```
SOL_SOCKET for general socket options
SO_REUSEADDR is the important flag that lets us circumvent TIME_WAIT
optval is the pointer to a value i want to set it to
after that is the length of the value.

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

#### send and receive


send and receive are like write and read. write and read only take an fd though.
and since send and receive and be partial, i need to always take that into account, that is why there is the safeSend funtion we did.
