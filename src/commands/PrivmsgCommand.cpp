#include "../../include/PrivmsgCommand.hpp"
#include "../../include/Debug.hpp"
#include "../../include/MessageType.hpp"

PrivmsgCommand::PrivmsgCommand(const Message& msg) : Command(msg)
{}

Command* PrivmsgCommand::fromMessage(const Message& message)
{
	return new PrivmsgCommand(message);
}

/*
    https://modern.ircdocs.horse/#privmsg-message
    ERR_NOSUCHNICK (401)
    ERR_NOSUCHSERVER (402)
    ERR_CANNOTSENDTOCHAN (404)
    ERR_TOOMANYTARGETS (407)
    ERR_NORECIPIENT (411)
    ERR_NOTEXTTOSEND (412)
    ERR_NOTOPLEVEL (413)
    ERR_WILDTOPLEVEL (414)
    RPL_AWAY (301)

*/
void PrivmsgCommand::execute(Server& server, Client& sender)
{
	std::vector<std::string> inParams = inMessage_.getParams();
	
	// Not Authenticated ==> ignore it...
	if (sender.getRegistrationLevel() < 3)
		return;
	// TODO: handle all errors...
	// Success !
	Message outMessage = inMessage_;
	outMessage.setSource(&sender.getNickname()); //TODO: whyyyyy a pointer ???
	std::stringstream stream(inParams[0]);
	std::string token;
	while (std::getline(stream, token, ','))
	{
		outMessage.getParams()[0] = token;
		if (token[0] == '#')
		{
			// map to channel
			// broadcast to channel
		}
		else
		{
			// map to the receiver , Muehsam...
			Client* receiver = NULL;
			for (std::vector<Client>::iterator it = server.getClients().begin(); it != server.getClients().end(); ++it)
			{
				if (it->getNickname() == token)
				{
					receiver = &(*it);
					break;
				}
			}
			//send the message()
			receiver->sendMessage(outMessage);
		}
	}
	return;
}
