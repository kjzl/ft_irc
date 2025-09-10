#include "../include/MessageQueueManager.hpp"
#include "../include/Debug.hpp"
#include "../include/IrcUtils.hpp"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <poll.h>
#include <sys/socket.h>
#include <utility>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

// Comparator for lower_bound to compare pollfd against an int fd.
static bool pollfdLess(const struct pollfd &p, int value) {
  return p.fd < value;
}

std::pair<bool, std::size_t> MessageQueueManager::findIndexByFd_(int fd) const {
  std::vector<struct pollfd>::const_iterator it =
      std::lower_bound(pfds_.begin(), pfds_.end(), fd, pollfdLess);
  const bool found = (it != pfds_.end() && it->fd == fd);
  return std::make_pair(found, static_cast<std::size_t>(it - pfds_.begin()));
}

bool MessageQueueManager::insertMsgAtQueue_(std::size_t index,
                                            const std::string &msg) {
  queues_[index].pushBack(msg);
  if (queues_[index].totalBytes() > MAX_BACKLOG_SIZE) {
    debug("Warning: MessageQueue for fd " + toString(pfds_[index].fd) +
          " exceeds maximum backlog size.");
    markDeadAndRemove_(index);
    return false;
  }
  return true;
}

bool MessageQueueManager::insertAt_(std::size_t index, int fd,
                                    const std::string &msg) {
  struct pollfd p;
  p.fd = fd;
  p.events = POLLOUT;
  p.revents = 0;

  pfds_.insert(pfds_.begin() + static_cast<std::ptrdiff_t>(index), p);
  queues_.insert(queues_.begin() + static_cast<std::ptrdiff_t>(index),
                 MessageQueue());
  return insertMsgAtQueue_(index, msg);
}

void MessageQueueManager::removeAt_(std::size_t index) {
  pfds_.erase(pfds_.begin() + static_cast<std::ptrdiff_t>(index));
  queues_.erase(queues_.begin() + static_cast<std::ptrdiff_t>(index));
}

void MessageQueueManager::markDeadAndRemove_(std::size_t index) {
  const int fd = pfds_[index].fd;
  deadFds_.push_back(fd);
  removeAt_(index);
}

MessageQueueManager::MessageQueueManager() {
  debug("MessageQueueManager default constructor called");
}

MessageQueueManager::MessageQueueManager(const MessageQueueManager &other) {
  pfds_ = other.pfds_;
  queues_ = other.queues_;
  deadFds_ = other.deadFds_;
}

MessageQueueManager &
MessageQueueManager::operator=(const MessageQueueManager &other) {
  if (this != &other) {
    pfds_ = other.pfds_;
    queues_ = other.queues_;
    deadFds_ = other.deadFds_;
  }
  return *this;
}

MessageQueueManager::~MessageQueueManager() {
  debug("MessageQueueManager destructor called");
}

bool MessageQueueManager::isDead_(int fd) const {
  for (std::size_t i = 0; i < deadFds_.size(); ++i) {
    if (deadFds_[i] == fd)
      return true;
  }
  return false;
}

int MessageQueueManager::drainQueueForFd_(
    const std::pair<bool, std::size_t> fd_lookup) {
  if (!fd_lookup.first) {
    return 0; // no entry means no backlog and fd assumed alive
  }
  std::size_t idx = fd_lookup.second;
  int fd = pfds_[idx].fd;
  MessageQueue &queue = queues_[idx];

  while (!queue.empty()) {
    const std::string &front = queue.front();
    const std::size_t len = front.size();
    if (len == 0) {
      queue.popFront();
      continue;
    }

  const ssize_t n = ::send(fd, front.c_str(), len, MSG_NOSIGNAL | MSG_DONTWAIT);
    if (n > 0) {
      queue.removeBytesFromFront(static_cast<std::size_t>(n));
      continue; // try to drain more immediately
    }
    if (n == 0)
      return 1; // treat as would block now -> retry later
    // n < 0 => error
    if (errno == EINTR)
      continue; // Retry loop
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 1; // Would block now; some backlog remains
    // Fatal I/O error
    markDeadAndRemove_(idx);
    return -1; // fd dead
  }

  // If the entry still exists and its queue is empty, remove it
  if (idx < pfds_.size() && pfds_[idx].fd == fd && queue.empty()) {
    removeAt_(idx);
  }
  return 0; // no backlog left, fd alive
}

void MessageQueueManager::send(int fd, const std::string &msg) {
  // If fd is already marked dead, discard any work.
  if (isDead_(fd))
    return;

  if (!msg.empty()) {
    const std::pair<bool, std::size_t> res = findIndexByFd_(fd);
    bool exists = res.first;
    const std::size_t i = res.second;
    if (!exists) // dont care about the return values
      insertAt_(i, fd, msg);
    else
      insertMsgAtQueue_(i, msg);
  }
}

// Implementation that tries to send immediately without poll()
/*

bool MessageQueueManager::sendOnFdWithoutBacklog_(int fd,
                                                  const std::string &msg,
                                                  std::string &remainder) {
  remainder.clear();
  if (msg.empty()) {
    return true;
  }

  ssize_t n = ::send(fd, msg.c_str(), msg.size(), MSG_NOSIGNAL | MSG_DONTWAIT);
  if (n < 0 && errno == EINTR)
    n = ::send(fd, msg.c_str(), msg.size(), MSG_NOSIGNAL | MSG_DONTWAIT);

  if (n == static_cast<ssize_t>(msg.size())) {
    return true; // fully sent
  } else if (n > 0) {
    remainder = msg.substr(static_cast<std::size_t>(n));
    return true;
  } else if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
      remainder = msg; // not writable now
      return true;
    }
    // Fatal error, mark dead (not using markDead method because fd is not
    // even tracked at this point)
    deadFds_.push_back(fd);
    return false;
  } else {
    // n == 0 with len>0, queue full message
    remainder = msg;
    return true;
  }
}

///
/// @brief Queue data for non-blocking delivery to fd, attempting an
///        immediate one-shot write first.
///
/// Behavior:
/// - If fd was previously marked dead, the call is a no-op.
/// - Any existing backlog for fd is first drained (non-blocking).
/// - If backlog is fully drained, a single non-blocking send(2) of msg is
///   attempted (retrying once on EINTR). Unsent bytes, if any, are queued.
/// - If the socket would block, the full message is queued.
/// - On fatal errors, fd is recorded in deadFds_ and no data is queued.
///
/// This function does not call poll(2). Use mergePollfds() before your poll
/// call and drainQueuesForPolled() afterwards to progress queued writes.
///
/// @param fd  Connected socket file descriptor (preferably O_NONBLOCK).
/// @param msg Bytes to send (appends CRLF etc. should already be present).
///
void MessageQueueManager::send(int fd, const std::string &msg) {
  // If fd is already marked dead, discard any work.
  if (isDead_(fd))
    return;

  const std::pair<bool, std::size_t> fd_lookup = findIndexByFd_(fd);
  std::string toQueue;

  // Try to drain any existing backlog for this fd first.
  int drainStatus = drainQueueForFd_(fd_lookup);
  if (drainStatus == -1) {
    return; // fd became dead during draining, discard msg
  } else if (drainStatus == 0) {
    if (!sendOnFdWithoutBacklog_(fd, msg, toQueue))
      return; // fd became dead during sending, discard msg
  } else
    toQueue = msg; // Backlog remains; queue full msg if provided

  // potentially need to enqueue remaining data
  if (!toQueue.empty()) {
    // storage backing arrays might have changed, run find again
    const std::pair<bool, std::size_t> res = findIndexByFd_(fd);
    bool exists = res.first;
    const std::size_t i = res.second;
    if (!exists) // dont care about the return values
      insertAt_(i, fd, toQueue);
    else
      insertMsgAtQueue_(i, toQueue);
  }
}
*/

void MessageQueueManager::drainQueuesForPolled(
    const std::vector<struct pollfd> &polled) {
  if (pfds_.empty())
    return; // nothing to drain or no ready sockets to write to

  const std::size_t sizeBefore = pfds_.size();

  for (std::vector<struct pollfd>::const_iterator it = polled.begin();
       it != polled.end(); ++it) {
    const short re = it->revents;
    const std::pair<bool, std::size_t> fd_lookup = findIndexByFd_(it->fd);
    if (!fd_lookup.first) // polled fd is not tracked
      continue;
    const std::size_t idx = fd_lookup.second;

    // Handle error/close first
    if (re & (POLLERR | POLLHUP | POLLNVAL)) {
      markDeadAndRemove_(idx);
      continue;
    }

    // Writable: attempt to drain without blocking
    if (re & POLLOUT)
      drainQueueForFd_(fd_lookup); // return value can be ignored here
  }

  // try to shrink storage if entries were removed
  if (pfds_.size() < sizeBefore) {
    shrinkVecToFit(pfds_);
    shrinkVecToFit(queues_);
  }
  if (!pfds_.empty()) {
    debug("Backlog after draining (" + toString(pfds_.size()) +
          " fd's): " + toString(queues_[0].totalBytes()) + " bytes on fd " +
          toString(pfds_[0].fd));
  }
}

void MessageQueueManager::mergePollfds(
    std::vector<struct pollfd> &target) const {
  for (std::vector<struct pollfd>::iterator it = target.begin();
       it != target.end(); ++it) {
    const std::pair<bool, std::size_t> res = findIndexByFd_(it->fd);
    if (res.first) { // fd is tracked, merge events
      it->events = static_cast<short>(it->events | pfds_[res.second].events);
    }
  }
}

void MessageQueueManager::discard(int fd) {
  const std::pair<bool, std::size_t> res = findIndexByFd_(fd);
  if (!res.first) {
    return; // nothing to discard
  }
  const std::size_t i = res.second;
  // Remove the entry entirely since the queue is now empty
  removeAt_(i);
  shrinkVecToFit(pfds_);
  shrinkVecToFit(queues_);
}

bool MessageQueueManager::hasBacklog(int fd) const {
  // Check if the fd is tracked
  // We should never have empty queues
  return findIndexByFd_(fd).first;
}

bool MessageQueueManager::hasBacklog() const { return !queues_.empty(); }

std::vector<int> MessageQueueManager::takeDeadFds() {
  std::vector<int> out;
  out.swap(deadFds_); // efficient move-out
  shrinkVecToFit(deadFds_);
  return out;
}

bool MessageQueueManager::hasDeadFds() const { return !deadFds_.empty(); }
