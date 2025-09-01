#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include <ctime>

class Message;
class Client;


class   Channel
{
	private:
		std::string					name_;
		std::map<std::string, int>	members_;  // we could also use the unique fd ?
		std::set<std::string>		whiteList_; //(quicker search) if is not empty, is invite only channel
		std::set<std::string>		operators_;
		std::string					topic_;
		time_t						creationTime_;


		std::string					password_;
		int							userLimit_;
		bool						isInviteOnly_;
		bool						isTopicProtected_; //If this mode is enabled, users must have channel privileges such as halfop or operator status in order to change the topic of a channel

	public:
		Channel();
		// Channel(std::vector<std::string> members, std::set<std::string> whiteList, std::set<std::string> operators, std::string topic, std::string password, int userLimit);
		Channel(const std::string& name, const Client& sender);
		Channel(const Channel &other);
		Channel &operator =(const Channel &other);
		virtual ~Channel();

		// Getters => necessary or only Utils?? 
		const	std::string					&getName() const;
		const	std::map<std::string, int>	&getMembers() const;
		const	std::set<std::string>		&getWhiteList() const;
		const	std::set<std::string>		&getOperators() const;
		const	std::string 				&getTopic() const;
		const	std::string 				&getPassword() const;
		int 								getUserLimit() const;
		const	time_t						&getCreationTime() const;

		// Setters => necessary or only Utils and full constructor?? 
		void setTopic(const std::string &topic);
		void setPassword(const std::string &password);
		void setUserLimit(int limit);

		void broadcastMsg(const Client &sender, const Message &message) const;

		void addMember(const Client* client);
		void removeMember(const std::string &nickname);
		bool isMember(const std::string &nickname) const;
		
		void addToWhiteList(const std::string &nickname);
		void removeFromWhiteList(const std::string &nickname);
		bool isWhiteListed(const std::string &nickname) const;
		
		void addOperator(const std::string &nickname);
		void removeOperator(const std::string &nickname);
		bool isOperator(const std::string &nickname) const;
		
		void setInviteOnly(bool value);
		void setTopicProtected(bool value);
		bool checkKey(const std::string& key) const;
		bool isInviteOnly() const;
		bool isTopicProtected() const;
};

#endif

