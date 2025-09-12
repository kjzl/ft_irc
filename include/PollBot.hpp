#ifndef POLLBOT_HPP
#define POLLBOT_HPP

#include "Bot.hpp"
#include <map>

struct ChannelPoll {
  bool open;
  std::string question;
  std::vector<std::string> options; // index 1..N for user-facing
  std::map<std::string, size_t> votesByNick; // nickname -> option index (1..N)
  std::vector<size_t> tally; // size==options.size()+1, tally[i] for option i

  ChannelPoll() : open(false) {}
};

// Simple poll bot: start/end poll per-channel and accept private votes
class PollBot : public Bot {
public:
  PollBot(std::string host, unsigned short port, std::string password,
          std::string nickname, std::string username, std::string realname);
  virtual ~PollBot();

protected:
  // Overrides
  virtual void onPrivmsg(const class Message &msg);
  virtual void onNumeric(const class Message &msg);
  virtual void onNotice(const class Message &msg);
  virtual void onShutdown();
  virtual void onJoin(const class Message &msg);

private:
  // state: one poll per channel
  std::map<std::string, ChannelPoll> polls_;

  // commands
  void handleChannelCommand(const std::string &channel, const std::string &fromNick, const std::string &text);
  void handlePrivateCommand(const std::string &fromNick, const std::string &text);

  // poll ops
  void cmdStartPoll(const std::string &channel, const std::string &fromNick, const std::string &rest);
  void cmdClosePoll(const std::string &channel, const std::string &fromNick);
  void cmdVote(const std::string &fromNick, const std::string &channel, size_t optionId);

  void announcePoll(const std::string &channel, const ChannelPoll &poll);
  void announceResults(const std::string &channel, const ChannelPoll &poll);
};

#endif // POLLBOT_HPP
