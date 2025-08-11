
#ifndef KICKCOMMAND_HPP
#define KICKCOMMAND_HPP

#include "Command.hpp"
#include "Channel.hpp"

class KickCommand : public Command{
	public:
		virtual ~KickCommand();

		KickCommand(const KickCommand &copy);
		KickCommand& operator=( const KickCommand &assign );

		KickCommand(const Message& msg);
		void			execute(Server& server, Client& sender);
		static Command*	fromMessage(const Message& message);
	private:
		KickCommand( void );
};

#endif
