
#ifndef QUITCOMMAND_HPP
#define QUITCOMMAND_HPP

#include "../Command.hpp"
#include "../Channel.hpp"

class QuitCommand : public Command{
	public:
		virtual ~QuitCommand();

		QuitCommand(const QuitCommand &copy);
		QuitCommand& operator=( const QuitCommand &assign );

		QuitCommand(const Message& msg);
		void			execute(Server& server, Client& sender);
		static Command*	fromMessage(const Message& message);
	private:
		QuitCommand( void );
};

#endif
