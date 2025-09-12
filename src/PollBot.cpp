#include "../include/PollBot.hpp"
#include "../include/Message.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

PollBot::PollBot(std::string host, unsigned short port, std::string password,
                 std::string nickname, std::string username, std::string realname)
    : Bot(host, port, password, nickname, username, realname) {}

PollBot::~PollBot() {}

static bool isChannel(const std::string &target) {
  return !target.empty() && (target[0] == '#' || target[0] == '&');
}

void PollBot::onPrivmsg(const Message &msg) {
  const std::vector<std::string> &p = msg.getParams();
  if (p.size() < 2) return;
  const std::string &target = p[0];
  const std::string &text = p[1];
  const std::string fromNick = msg.getNickname();
  if (fromNick.empty()) return; // ignore server messages

  if (isChannel(target)) {
    handleChannelCommand(target, fromNick, text);
  } else {
    // private command for voting
    handlePrivateCommand(fromNick, text);
  }
}

void PollBot::onNotice(const Message &msg) { (void)msg; }

void PollBot::onNumeric(const Message &msg) {
  (void)msg;
}

// Syntax on channel:
//  !poll start <question> | <opt1> | <opt2> [| <optN>]
//  !poll close
void PollBot::handleChannelCommand(const std::string &channel, const std::string &fromNick, const std::string &text) {
  if (text.find("!poll ") != 0) return;
  std::string cmd = text.substr(6);
  if (cmd.compare(0, 5, "start") == 0) {
    std::string rest = cmd.size() > 6 ? cmd.substr(6) : std::string();
    // trim leading spaces
    rest.erase(0, rest.find_first_not_of(' '));
    cmdStartPoll(channel, fromNick, rest);
  } else if (cmd.compare(0, 5, "close") == 0) {
    cmdClosePoll(channel, fromNick);
  }
}

// Private: "vote <#channel> <id>"
void PollBot::handlePrivateCommand(const std::string &fromNick, const std::string &text) {
  std::string t = text;
  // lower copy to check prefix
  std::string low = t;
  std::transform(low.begin(), low.end(), low.begin(), ::tolower);
  if (low.compare(0, 5, "vote ") != 0) return;
  // parse channel and number
  std::istringstream ss(t.substr(5));
  std::string channel;
  std::string token;
  ss >> channel;
  if (channel.empty() || channel[0] != '#') {
    sendNotice(fromNick, "Usage: vote #channel <number>");
    return;
  }
  if (!(ss >> token)) {
    // no number provided
    sendNotice(fromNick, "Usage: vote #channel <number>");
    return;
  }
  bool allDigits = true;
  for (size_t i = 0; i < token.size(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(token[i]))) { allDigits = false; break; }
  }
  if (!allDigits) {
    sendNotice(fromNick, "Please send a valid option number.");
    return;
  }
  std::istringstream numss(token);
  size_t id = 0;
  numss >> id;
  if (id == 0) {
    sendNotice(fromNick, "Please send a valid option number.");
    return;
  }
  cmdVote(fromNick, channel, id);
}

void PollBot::cmdStartPoll(const std::string &channel, const std::string &fromNick, const std::string &rest) {
  (void)fromNick; // anyone may start
  ChannelPoll &poll = polls_[channel];
  if (poll.open) {
    sendNotice(channel, "There's already an open poll. Close it with !poll close.");
    return;
  }
  // rest expected format: <question> | <opt1> | <opt2> [| <optN>]
  // split by '|'
  std::vector<std::string> parts;
  std::string buf;
  std::istringstream iss(rest);
  while (std::getline(iss, buf, '|')) {
    // trim spaces
    size_t b = buf.find_first_not_of(' ');
    size_t e = buf.find_last_not_of(' ');
    if (b == std::string::npos) parts.push_back("");
    else parts.push_back(buf.substr(b, e - b + 1));
  }
  if (parts.size() < 3) {
    sendNotice(channel, "Usage: !poll start <question> | <opt1> | <opt2> [| <optN>]");
    return;
  }
  poll.open = true;
  poll.question = parts[0];
  poll.options.assign(parts.begin() + 1, parts.end());
  poll.votesByNick.clear();
  poll.tally.assign(poll.options.size() + 1, 0); // 1-based
  announcePoll(channel, poll);
}

void PollBot::cmdClosePoll(const std::string &channel, const std::string &fromNick) {
  (void)fromNick;
  std::map<std::string, ChannelPoll>::iterator it = polls_.find(channel);
  if (it == polls_.end() || !it->second.open) {
    sendNotice(channel, "No open poll in this channel.");
    return;
  }
  it->second.open = false;
  announceResults(channel, it->second);
}

void PollBot::cmdVote(const std::string &fromNick, const std::string &channel, size_t id) {
  std::map<std::string, ChannelPoll>::iterator it = polls_.find(channel);
  if (it == polls_.end() || !it->second.open) {
    sendNotice(fromNick, "No open poll in that channel.");
    return;
  }
  ChannelPoll &poll = it->second;
  if (id == 0 || id > poll.options.size()) {
    sendNotice(fromNick, "Please send a valid option number.");
    return;
  }
  poll.votesByNick[fromNick] = id;
  // recompute tally
  std::fill(poll.tally.begin(), poll.tally.end(), 0);
  for (std::map<std::string, size_t>::const_iterator vn = poll.votesByNick.begin(); vn != poll.votesByNick.end(); ++vn) {
    if (vn->second < poll.tally.size()) poll.tally[vn->second]++;
  }
  sendNotice(fromNick, "Your vote has been recorded.");
}

void PollBot::announcePoll(const std::string &channel, const ChannelPoll &poll) {
  sendPrivmsg(channel, "New poll: " + poll.question);
  for (size_t i = 0; i < poll.options.size(); ++i) {
    std::ostringstream os; os << (i+1) << ") " << poll.options[i];
    sendPrivmsg(channel, os.str());
  }
  sendPrivmsg(channel, "Vote by sending me a private message: vote #channel <number>");
}

void PollBot::announceResults(const std::string &channel, const ChannelPoll &poll) {
  sendPrivmsg(channel, "Poll closed. Results:");
  for (size_t i = 0; i < poll.options.size(); ++i) {
    std::ostringstream os; os << (i+1) << ") " << poll.options[i] << " - " << poll.tally[i+1] << " vote(s)";
    sendPrivmsg(channel, os.str());
  }
}

void PollBot::onShutdown() {
  // Close any remaining open polls and announce results
  for (std::map<std::string, ChannelPoll>::iterator it = polls_.begin(); it != polls_.end(); ++it) {
    if (it->second.open) {
      it->second.open = false;
      announceResults(it->first, it->second);
    }
  }
}

void PollBot::onJoin(const Message &msg) {
  // Only react when this bot itself joins a channel.
  const std::string self = getNickname();
  if (msg.getNickname() != self)
    return; // only when the bot joins
  const std::vector<std::string> &p = msg.getParams();
  if (p.empty()) return;
  const std::string &channel = p[0];
  sendPrivmsg(channel, "Hi! I can run simple polls.");
  sendPrivmsg(channel, "Start: !poll start <question> | <opt1> | <opt2> [| <optN>]");
  sendPrivmsg(channel, "Close: !poll close");
  sendPrivmsg(channel, "Vote (private msg to me): vote #channel <number>");
}
