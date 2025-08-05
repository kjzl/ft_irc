#include "Server.hpp"
#include "Channel.hpp"

std::map<std::string, Channel>	&getChannelMap()
{
	static std::map<std::string, Channel> channelMap;
	if (channelMap.empty())
	{
		channelMap["entrance"] = Channel();
	}
	return (channelMap);
}
