#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../include/Client.hpp"
#include <string>
#include <vector>
#include <set>

class Message{};


class   Channel
{
	private:
		std::vector<std::string>	members_;  //(quicker iteration) // we could also use the unique fd ?
		std::set<std::string>		whiteList_; //(quicker search)
		std::set<std::string>		operators_;
		std::string					topic_;
		std::string					password_;
		int							user_limit_;

	public:
		Channel();
		Channel(const Channel &other);
		Channel &operator =(const Channel &other);
		virtual ~Channel();
		const std::vector<std::string> &getMembers() const;
		const std::set<std::string> &getWhiteList() const;
		const std::set<std::string> &getOperators() const;
		const std::string &getTopic() const;
		const std::string &getPassword() const;
		int getUserLimit() const;

		// Getters => necessary or only Utils?? 
		const std::vector<std::string> &getMembers() const;
		const std::set<std::string> &getWhiteList() const;
		const std::set<std::string> &getOperators() const;
		const std::string &getTopic() const;
		const std::string &getPassword() const;
		int getUserLimit() const;

		// Setters => necessary or only Utils and full constructor?? 
		void setTopic(const std::string &topic);
		void setPassword(const std::string &password);
		void setUserLimit(int limit);

		// Action Utilities -- WIP BELOW => no cpp yet...
		void broadcastMsg(const Client &sender, Message &message);

		void addMember(const std::string &nickname);
		void removeMember(const std::string &nickname);
		bool isMember(const std::string &nickname) const;

		void addOperator(const std::string &nickname);
		void removeOperator(const std::string &nickname);
		bool isOperator(const std::string &nickname) const;

		void addToWhiteList(const std::string &nickname);
		void removeFromWhiteList(const std::string &nickname);
		bool isInWhiteList(const std::string &nickname) const;
};

#endif

