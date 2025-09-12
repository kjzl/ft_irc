#include "../include/PollBot.hpp"
#include <cstring>
#include <exception>
#include <iostream>
#include <errno.h>


int bot_main(int argc, char *argv[]) {
	// TODO optional args: <host> <port> <password> <nickname> <username> <realname> [auto-join channels...]
	(void)argc;
	(void)argv;
	try {
	PollBot	bot("127.0.0.1", 6667, "password", "PollBot", "PollUser", "Poll Bot");
	if (bot.connect()) {
		bot.login();
		bot.run();
		bot.disconnect();
	}
	} catch (std::exception &e) {
		std::cerr << e.what() << ": " << (errno) << std::endl;
	}

	return 0;
}
