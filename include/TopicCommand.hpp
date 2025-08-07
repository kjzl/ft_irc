
#ifndef TOPICCOMMAND_HPP
#define TOPICCOMMAND_HPP

#include "Command.hpp"
#include "Channel.hpp"

class TopicCommand : public Command{
	public:
		virtual ~TopicCommand();

		TopicCommand(const TopicCommand &copy);
		TopicCommand& operator=( const TopicCommand &assign );

		TopicCommand(const Message& msg);
		void			execute(Server& server, Client& sender);
		static Command*	fromMessage(const Message& message);
	private:
		TopicCommand( void );
};

#endif
