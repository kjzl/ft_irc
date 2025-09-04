#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>

class Message;
class Client;
class MessageQueueManager;

class Channel {
  private:
	MessageQueueManager		  &mqr_;
	std::string				   name_;
	// we could also use the unique fd ?
	std::map<std::string, int> members_;
	//(quicker search) if is not empty, is invite only channel
	std::set<std::string>	   whiteList_;
	std::set<std::string>	   operators_;
	std::string				   topic_;

	std::string password_;
	int			userLimit_;
	bool		isInviteOnly_;
	// If this mode is enabled, users must have channel
	// privileges such as halfop or operator status in
	// order to change the topic of a channel
	bool		isTopicProtected_;

  public:
	// Channel(std::vector<std::string> members, std::set<std::string>
	// whiteList, std::set<std::string> operators, std::string topic,
	// std::string password, int userLimit);
	Channel(const std::string &name, const Client &sender,
			MessageQueueManager &queueManager);
	Channel(const Channel &other);
	Channel &operator=(const Channel &other);
	virtual ~Channel();

		// Getters => necessary or only Utils?? 
		const	std::string					&getName() const;
		const	std::map<std::string, int>	&getMembers() const;
		const	std::set<std::string>		&getWhiteList() const;
		const	std::set<std::string>		&getOperators() const;
		const	std::string 				&getTopic() const;
		const	std::string 				&getPassword() const;
		int 								getUserLimit() const;

		// Setters => necessary or only Utils and full constructor?? 
		void setTopic(const std::string &topic);
		void setPassword(const std::string &password);
		void setUserLimit(int limit);

	// Broadcast to all members except the given sender nickname
	void broadcastMsg(const std::string &senderNickname,
					  const Message		&message) const;
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
