#include "../include/Bot.hpp"
#include "../include/Debug.hpp"
#include "../include/IrcUtils.hpp"
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "../include/Message.hpp"

// Define static members
bool Bot::stopRequested_ = false;

Bot::Bot(std::string host, unsigned short port, std::string password,
         std::string nickname, std::string username, std::string realname)
  : socket_(-1), running_(false), connecting_(false), pendingClose_(false), closeAttempts_(0),
      host_(host), port_(port), password_(password), nickname_(nickname),
      username_(username), realname_(realname) {
  debug("Bot constructor called");
}

Bot::~Bot() {
  debug("Bot destructor called");
  if (socket_ != -1) {
    messageQueueManager_.discard(socket_);
    close(socket_);
    socket_ = -1;
  }
}

void Bot::sendRaw(const std::string &msg) {
  if (this->socket_ == -1)
    throw std::runtime_error("[Bot] not connected");
  // Queue the message for non-blocking send via manager (CRLF appended)
  messageQueueManager_.send(this->socket_, msg + "\r\n");
}

void Bot::sendPass() { sendRaw("PASS " + password_); }
void Bot::sendNick() { sendRaw("NICK " + nickname_); }
void Bot::sendUser() { sendRaw("USER " + username_ + " 0 * :" + realname_); }

void Bot::sendPrivmsg(const std::string &target, const std::string &text) {
  sendRaw("PRIVMSG " + target + " :" + text);
}

void Bot::sendNotice(const std::string &target, const std::string &text) {
  sendRaw("NOTICE " + target + " :" + text);
}

void Bot::sendJoin(const std::string &channel) { sendRaw("JOIN " + channel); }

void Bot::sendPart(const std::string &channel, const std::string &reason) {
  if (reason.empty())
    sendRaw("PART " + channel);
  else
    sendRaw("PART " + channel + " :" + reason);
}

bool Bot::connect() {
  struct addrinfo hints;
  struct addrinfo *res = 0, *p = 0;
  int fd = -1;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP

  std::string port_str = toString(port_);
  if (getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &res) != 0) {
    std::cout << "getaddrinfo() failed" << std::endl;
    return false;
  }

  for (p = res; p != 0; p = p->ai_next) {
    fd = socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol);
    if (fd == -1)
      continue; // try next address
    if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
      connecting_ = false;
      break; // success
    }
    if (errno == EINPROGRESS) {
      // Non-blocking connect in progress
      connecting_ = true;
      break;
    }
    // connect failed for this addr
    close(fd);
    fd = -1;
  }

  freeaddrinfo(res);

  if (fd == -1) {
    std::cout << "connect() failed: " << (errno) << std::endl;
    return false;
  }

  this->socket_ = fd;
  std::cout << "Connected socket " << fd
            << (connecting_ ? " (in progress)" : "") << std::endl;
  return true;
}

void Bot::disconnect() {
  // Defer actual close until send queue is empty and we get POLLOUT once
  if (socket_ != -1 && !pendingClose_) {
    try {
      sendRaw("QUIT");
    } catch (const std::exception &) {
      // ignore queuing errors
    }
    schedulePendingClose();
  }
  running_ = false; // signal main loop to finish gracefully
}

void Bot::login() {
  if (!password_.empty())
    sendPass();
  sendNick();
  sendUser();
  // TODO register that we expect the server to respond in a certain way
}

// Reply to server PINGs to keep the connection alive
// void Bot::onPing(const std::string &token) {
//   try {
//     if (token.empty())
//       sendRaw("PONG");
//     else
//       sendRaw("PONG " + token); // preserve leading ':' if present
//   } catch (const std::exception &e) {
//     std::cerr << "[Bot] Failed to send PONG: " << e.what() << std::endl;
//     this->running_ = false;
//   }
// }

// Non-Blocking poll loop
void Bot::run() {
  try {
    if (this->socket_ == -1)
      throw std::runtime_error("[Bot] run() called without a connected socket");
    // Ensure we have signal handlers installed once.
    installSignalHandlers();
    running_ = true;
    rxBuffer_.clear();
    // keep vector allocation outside the loop
    std::vector<struct pollfd> polled;
    while (running_ || pendingClose_ ||
           (socket_ != -1 && messageQueueManager_.hasBacklog(socket_))) {
      // If Ctrl+C or SIGQUIT was pressed, request a graceful shutdown.
      if (stopRequested_) {
        running_ = false;
        if (!pendingClose_ && socket_ != -1)
          schedulePendingClose();
      }
      buildPollVector(polled);
      int rc = poll(&polled[0], polled.size(), 1000);
      if (rc == -1) {
        if (errno == EINTR)
          continue;
        std::string err = std::string("poll failed: ") + toString(errno);
        closeErroneousSocket(err.c_str());
      }
      if (rc > 0) {
        const struct pollfd &rp = polled[0];
        handlePollErrors(rp);
        if (!handleConnectReady(rp))
          break;
        if (pendingClose_)
          if (tryCloseIfPending())
            break;
        messageQueueManager_.drainQueuesForPolled(polled);
        if ((rp.revents & POLLIN) && running_) {
          readFromSocket();
          processRxLines();
        }
      }
      // Even on timeout (rc == 0), progress graceful shutdown if backlog is drained.
      if (pendingClose_)
        if (tryCloseIfPending())
          break;
      handleDeadFds();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error in Bot run loop: " << e.what() << std::endl;
  }
  if (socket_ != -1) {
  messageQueueManager_.discard(socket_);
  close(socket_);
    socket_ = -1;
  }
}

void Bot::buildPollVector(std::vector<struct pollfd> &polled) const {
  struct pollfd pfd;
  pfd.fd = socket_;
  pfd.revents = 0;
  pfd.events = 0;
  if (running_ && !pendingClose_ && !connecting_) {
    pfd.events = POLLIN;
  }
  if (connecting_) {
    pfd.events = static_cast<short>(pfd.events | POLLOUT);
  }
  polled.clear();
  polled.push_back(pfd);
  messageQueueManager_.mergePollfds(polled);
}

bool Bot::handleConnectReady(const struct pollfd &rp) {
  if (!connecting_)
    return true;
  // If the socket was already closed due to an earlier error in this
  // iteration, stop connecting now.
  if (socket_ == -1) {
    connecting_ = false;
    return false;
  }
  if (!(rp.revents & (POLLOUT | POLLERR | POLLHUP | POLLNVAL)))
    return true;
  int soerr;
  soerr = 0;
  socklen_t slen;
  slen = sizeof(soerr);
  if (getsockopt(socket_, SOL_SOCKET, SO_ERROR, &soerr, &slen) == -1)
    soerr = errno;
  if (soerr != 0) {
    connecting_ = false;
    std::string reason = std::string("connect failed: ") + toString(soerr);
    closeErroneousSocket(reason.c_str());
    return false;
  }
  connecting_ = false;
  return true;
}

void Bot::handlePollErrors(const struct pollfd &rp) {
  if (rp.revents & (POLLHUP | POLLERR | POLLNVAL)) {
    closeErroneousSocket("poll error/hup/nval");
  }
}

void Bot::readFromSocket() {
  char chunk[4096];
  ssize_t n = recv(this->socket_, chunk, sizeof(chunk), MSG_DONTWAIT);
  if (n == 0) {
    closeErroneousSocket("peer closed (EOF)");
    return;
  }
  if (n < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
      std::string reason = std::string("recv failed: ") + toString(errno);
      closeErroneousSocket(reason.c_str());
      return;
    }
    return;
  }
  rxBuffer_.append(chunk, static_cast<std::size_t>(n));
}

void Bot::processRxLines() {
  std::size_t pos;
  while ((pos = rxBuffer_.find('\n')) != std::string::npos) {
    std::string line = rxBuffer_.substr(0, pos);
    rxBuffer_.erase(0, pos + 1);
    if (line.empty())
      continue;
    if (!line.empty() && line[line.size() - 1] == '\r') {
      line.erase(line.size() - 1);
    }
    std::cout << RED << "<<< " << RESET << line << std::endl;
    // Parse incoming and dispatch
    try {
      Message msg(line);
      handleMessage(msg);
    } catch (const std::exception &e) {
      std::cerr << "[Bot] Failed to parse/handle line: " << e.what() << std::endl;
    }
  }
}

bool Bot::tryCloseIfPending() {
  if (!pendingClose_)
    return false;

  // Close as soon as our send backlog for this socket is drained and either the
  // socket is ready (with POLLOUT or an error) or the poll had a timeout.
  if (!messageQueueManager_.hasBacklog(socket_) || closeAttempts_ > 5) {
  messageQueueManager_.discard(socket_);
  close(socket_);
    socket_ = -1;
    running_ = false;
    pendingClose_ = false;
    std::cout << "Disconnected" << std::endl;
  closeAttempts_ = 0;
    return true;
  }
  closeAttempts_++;
  return false;
}

void Bot::closeErroneousSocket(const char *reason) {
  if (socket_ != -1) {
  messageQueueManager_.discard(socket_);
    close(socket_);
    socket_ = -1;
  }
  pendingClose_ = false;
  running_ = false;
  std::cerr << "[Bot] socket closed due to an error: " << reason << std::endl;
}

void Bot::handleDeadFds() {
  if (!messageQueueManager_.hasDeadFds())
    return;
  std::vector<int> dead = messageQueueManager_.takeDeadFds();
  for (std::size_t i = 0; i < dead.size(); ++i) {
    if (dead[i] != socket_)
      continue;
    closeErroneousSocket("send error or backlog limit reached");
    break;
  }
}

// static
void Bot::signalHandler(int signum) {
  (void)signum;
  stopRequested_ = true;
}

// static
void Bot::installSignalHandlers() {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
}

void Bot::schedulePendingClose() {
  if (socket_ == -1)
    return;
  if (!pendingClose_) {
    onShutdown();
    pendingClose_ = true;
    closeAttempts_ = 0;
  }
}

// Default message handling: basic PING/PONG and dispatch of PRIVMSG/NOTICE or numerics
void Bot::handleMessage(const Message &msg) {
  const std::string type = msg.getType();
  if (type == "PING") {
    const std::vector<std::string> &p = msg.getParams();
    onPing(p.empty() ? std::string() : p[0]);
    return;
  }
  // Numerics are 3 digits
  if (type.size() == 3 && std::isdigit(type[0]) && std::isdigit(type[1]) && std::isdigit(type[2])) {
    onNumeric(msg);
    return;
  }
  if (type == "PRIVMSG") {
    onPrivmsg(msg);
    return;
  }
  if (type == "NOTICE") {
    onNotice(msg);
    return;
  }
  if (type == "INVITE") {
    onInvite(msg);
    return;
  }
  if (type == "JOIN") {
    onJoin(msg);
    return;
  }
}

void Bot::onPing(const std::string &token) {
  try {
    if (token.empty())
      sendRaw("PONG");
    else
      sendRaw("PONG :" + token);
  } catch (...) {
  }
}

void Bot::onPrivmsg(const Message &msg) { (void)msg; }
void Bot::onNotice(const Message &msg) { (void)msg; }
void Bot::onNumeric(const Message &msg) { (void)msg; }
void Bot::onInvite(const Message &msg) {
  // INVITE <nick> <channel>
  const std::vector<std::string> &p = msg.getParams();
  if (p.size() >= 2) {
    const std::string &nick = p[0];
    const std::string &channel = p[1];
    if (nick == getNickname()) {
      sendJoin(channel);
    }
  }
}
void Bot::onJoin(const Message &msg) { (void)msg; }
