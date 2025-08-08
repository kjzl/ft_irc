#include "../include/Server.hpp"
#include <cstring>
#include <exception>
#include <iostream>
#include <errno.h>
#include <sstream>

//TODO:
//make main start with args
//check canonical form


int main (int argc, char *argv[]) {
	if (argc != 3)
	{
		std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	int port;
	std::stringstream portStream(argv[1]);
	if (!(portStream >> port))
	{
		std::cerr << "Invalid port number: conversion failed" << std::endl;
		return (1);
	}
	char remaining;
	if (portStream >> remaining)
		std::cerr << "Invalid port number: extra characters after number" << std::endl;
	std::string password = argv[2];
	try {
	Server	serv(port, static_cast<std::string>(argv[2]));
	serv.waitForRequests();
	serv.serverShutdown();
	} catch (std::exception &e) {
		std::cerr << e.what() << ": " << strerror(errno) << std::endl;
	}
	
	return 0;
}
