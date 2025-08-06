#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <set>

class Message;
class Client;


class   Channel
{
	private:
		std::string					name_;
		std::vector<const Client*>	members_;  //(quicker iteration) // we could also use the unique fd ?
		std::set<const Client*>		whiteList_; //(quicker search) if is not empty, is invite only channel
		std::set<const Client*>		operators_;
		std::string					topic_;
		std::string					password_;
		int							userLimit_;
		bool						isInviteOnly_;

	public:
		Channel();
		// Channel(std::vector<const Client*> members, std::set<std::string> whiteList, std::set<std::string> operators, std::string topic, std::string password, int userLimit);
		Channel(const std::string& name, const Client& sender);
		Channel(const Channel &other);
		Channel &operator =(const Channel &other);
		virtual ~Channel();

		// Getters => necessary or only Utils?? 
		const	std::vector<const Client*> &getMembers() const;
		const	std::set<const Client*> &getWhiteList() const;
		const	std::set<const Client*> &getOperators() const;
		const	std::string &getTopic() const;
		const	std::string &getPassword() const;
		int getUserLimit() const;

		// Setters => necessary or only Utils and full constructor?? 
		void setTopic(const std::string &topic);
		void setPassword(const std::string &password);
		void setUserLimit(int limit);

		void broadcastMsg(const Client &sender, const Message &message);
		
		void addMember(const Client *client);
		void removeMember(const Client *client);
		bool isMember(const Client *client) const;
		
		void addOperator(const Client *client);
		void removeOperator(const Client *client);
		bool isOperator(const Client *client) const;
		
		void addToWhiteList(const Client *client);
		void removeFromWhiteList(const Client *client);
		bool isInWhiteList(const Client *client) const;

		bool checkKey(const std::string& key) const;
		bool isInviteOnly() const;
		bool isWhiteListed(const Client& sender) const;
};

#endif

