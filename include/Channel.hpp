#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <set>

class   Channel
{
	private:
		std::vector<std::string>	members_;  //quicker iteration
		std::set<std::string>		whiteList_; //quicker search
		std::set<std::string>		operators_;
		std::string					topic_;
		std::string					password_;
		int							user_limit_;

	public:
		Channel();
		Channel(Channel &other);
		Channel &operator =(const Channel &other);
		virtual ~Channel();

		// action members functions here below ??
		// or just getter/setters and the actions are performed by the server ???
};

#endif

