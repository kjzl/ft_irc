#include "../include/Bot.hpp"
#include <cstring>
#include <exception>
#include <iostream>
#include <errno.h>


int bot_main(int argc, char *argv[]) {
	// TODO optional args: <host> <port> <password> <nickname> <username> <realname> [auto-join channels...]
	(void)argc;
	(void)argv;
	try {
	Bot	bot("localhost", 6667, "password", "BotNick", "BotUser", "Bot Real Name");
	if (bot.connect()) {
		bot.login();
		bot.run();
		bot.disconnect();
	}
	} catch (std::exception &e) {
		std::cerr << e.what() << ": " << strerror(errno) << std::endl;
	}

	return 0;
}
