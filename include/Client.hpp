#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class   Client
{
	private:
		bool        is_authenticated_;
		std::string nickname_; //necessary ?? ==> or we use a map{nickname:client} in the server class directly?
		std::string username_;
		// ??       socket_;
		

	public:
		Client();
		Client(Client &other);
		Client &operator =(const Client &other);
		virtual ~Client();

		std::string getNickName(void) const; //necessary ?
		void        setNickName(const std::string &name);
		std::string getUserName(void) const; //necessary?
		void        setUserName(const std::string &name);
		void        check_authentication(???); // ?? 
};

#endif

