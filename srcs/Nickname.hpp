#ifndef NICKNAME_HPP
# define NICKNAME_HPP

# include <string>

class Nickname {
	private:
		const std::string nick;
	public:
		// this should check nick for the disallowed chars and min/max length, etc.
		Nickname(const std::string& nick);
	
};

#endif
