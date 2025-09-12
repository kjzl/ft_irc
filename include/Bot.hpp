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

	// Accessors
	const std::string& getNickname() const { return nickname_; }
	const std::string& getUsername() const { return username_; }
	const std::string& getRealname() const { return realname_; }

protected:
	// High-level send helpers
	void sendPrivmsg(const std::string& target, const std::string& text);
	void sendNotice(const std::string& target, const std::string& text);
	void sendJoin(const std::string& channel);
	void sendPart(const std::string& channel, const std::string& reason = "");

	// Incoming message dispatching and virtual handlers to override
	virtual void handleMessage(const class Message& msg);
	virtual void onPing(const std::string& token);
	virtual void onPrivmsg(const class Message& msg);
	virtual void onNotice(const class Message& msg);
	virtual void onNumeric(const class Message& msg);
	virtual void onInvite(const class Message& msg);
	virtual void onJoin(const class Message& msg);
	// Called when a graceful disconnect is scheduled; subclasses can flush state
	virtual void onShutdown() {}

private:
	// Connection state
	int             socket_;
	bool            running_;
	bool            connecting_;
	bool            pendingClose_;
	int             closeAttempts_;

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

	// Non-copyable: owning a socket is not safely copyable (declared only)
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
