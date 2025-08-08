#include "../include/Server.hpp"
#include <cstring>
#include <exception>
#include <iostream>
#include <errno.h>

//TODO:
//make main start with args
//check canonical form
//PRIVMSG to channel response looks wierd: PRIVMSG #wool to all


int main (int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	// if (argc != 3)
	// {
	// 	std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
	// 	return (1);
	// }
	try {
	Server	serv(6667, "password");
	serv.waitForRequests();
	serv.serverShutdown();
	} catch (std::exception &e) {
		std::cerr << e.what() << ": " << strerror(errno) << std::endl;
	}
	
	return 0;
}
