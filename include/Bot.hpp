#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot {
public:
	Bot(std::string host, unsigned short port, std::string password,
		std::string nickname, std::string username, std::string realname);
	virtual ~Bot();

	bool connect();
	void login();
	void disconnect();
	void run();
	void sendRaw(const std::string& msg) const;

protected:
	//TODO onPrivMsg... etc
	void onPing(const std::string& token);
	

private:

	int				socket_;
	bool			running_;
	std::string		host_;
	unsigned short	port_;
	std::string		password_;
	std::string		nickname_;
	std::string		username_;
	std::string		realname_;
	Bot& operator=(const Bot& other);
	Bot(const Bot& other);
	void sendPass() const;
	void sendNick() const;
	void sendUser() const;
};

#endif // BOT_HPP
