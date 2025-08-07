#ifndef MODECOMMAND_HPP
#define MODECOMMAND_HPP

#include "Command.hpp"
#include "Channel.hpp"

class ModeCommand : public Command{
	public:
		virtual ~ModeCommand();

		ModeCommand(const ModeCommand &copy);
		ModeCommand& operator=( const ModeCommand &assign );

		ModeCommand(const Message& msg);
		void			execute(Server& server, Client& sender);
		static Command*	fromMessage(const Message& message);
	private:
		ModeCommand( void );
		void	userMode(Server& server, Client& sender);
};

#endif
