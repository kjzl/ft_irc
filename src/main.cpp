#include "../include/Server.hpp"
#include <iostream>

int main (int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	// if (argc != 3)
	// {
	// 	std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
	// 	return (1);
	// }
	Server	serv(6667, "password");
	serv.waitForRequests();
	
	return 0;
}
