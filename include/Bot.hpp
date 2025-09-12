#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <poll.h>
#include "MessageQueueManager.hpp"
#include <csignal>

class Bot {
public:
	Bot(std::string host, unsigned short port, std::string password,
		std::string nickname, std::string username, std::string realname);
	virtual ~Bot();

	// Establish a non-blocking connection; returns true if a socket was
	// created and either connected or connection is in progress.
	bool connect();
	void login();
	// Initiate graceful disconnect: queue QUIT, stop reading, and wait for
	// POLLOUT + empty send queue before closing inside run().
	void disconnect();
	// Single poll-loop for both RX and TX progression.
	void run();
	// Queue a raw IRC line (CRLF appended) for sending.
	void sendRaw(const std::string& msg);

protected:
	// TODO onPrivMsg... etc
	//void onPing(const std::string& token);

private:
	// Connection state
	int             socket_;
	bool            running_;
	bool            connecting_;
	bool            pendingClose_;

	// Config
	std::string     host_;
	unsigned short  port_;
	std::string     password_;
	std::string     nickname_;
	std::string     username_;
	std::string     realname_;

	// Outbound non-blocking send manager
	MessageQueueManager messageQueueManager_;

	// RX line buffer
	std::string     rxBuffer_;

	Bot& operator=(const Bot& other);
	Bot(const Bot& other);
	void sendPass();
	void sendNick();
	void sendUser();

	// Helpers
	void schedulePendingClose();
	void buildPollVector(std::vector<struct pollfd> &polled) const;
	bool handleConnectReady(const struct pollfd &rp);
	void handlePollErrors(const struct pollfd &rp);
	void readFromSocket();
	void processRxLines();
	bool tryCloseIfPending();
	void handleDeadFds();
	void closeErroneousSocket(const char *reason);

	static void signalHandler(int signum);
	static void installSignalHandlers();
	static bool stopRequested_;
};

#endif // BOT_HPP
