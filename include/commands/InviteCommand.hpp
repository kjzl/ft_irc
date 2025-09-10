
#ifndef INVITECOMMAND_HPP
#define INVITECOMMAND_HPP

#include "../Command.hpp"
#include "../Channel.hpp"

class InviteCommand : public Command{
	public:
		virtual ~InviteCommand();

		InviteCommand(const InviteCommand &copy);
		InviteCommand& operator=( const InviteCommand &assign );

		InviteCommand(const Message& msg);
		void			execute(Server& server, Client& sender);
		static Command*	fromMessage(const Message& message);
	private:
		InviteCommand( void );
};

#endif
