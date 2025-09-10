#ifndef MESSAGEQUEUEMANAGER_HPP
#define MESSAGEQUEUEMANAGER_HPP

#include "MessageQueue.hpp"
#include <string>
#include <utility>
#include <vector>

// Maximum amount of queued outbound data per tracked socket (in bytes).
// This is a soft guard used at insertion time.
#define MAX_BACKLOG_SIZE 32768

/**
 * @brief Per-socket outbound queue manager for non-blocking writes.
 *
 * Tracks sockets that have pending outbound data and drains them when the
 * application reports POLLOUT readiness. Internally maintains two parallel
 * vectors, kept sorted by file descriptor value:
 * - pfds_: struct pollfd entries (events always include POLLOUT)
 * - queues_: MessageQueue instances containing the pending bytes per fd
 *
 * Lookups are O(log N) using binary search on the sorted pfds_. Indices in
 * pfds_ and queues_ are aligned; insertion/removal updates both containers.
 *
 * Error handling and closures:
 * - If poll reports POLLERR/POLLHUP/POLLNVAL for a tracked fd, or send(2)
 *   returns a fatal error (e.g., EPIPE/ECONNRESET), the fd is recorded in
 *   deadFds_ and its entry is removed. Callers should periodically retrieve
 *   and clear these via takeDeadFds() and perform cleanup (close, remove from
 *   higher-level state).
 *
 * Sending path overview:
 * - send(fd, msg) first tries to drain any existing backlog for fd without
 *   blocking. If the backlog is gone, it performs a single optimistic
 *   non-blocking send of msg (retrying once on EINTR).
 *   - If fully written, nothing is queued.
 *   - If partially written, only the remainder is queued.
 *   - If EAGAIN/EWOULDBLOCK, the full message is queued.
 *   - On fatal errors, fd is recorded dead and the message is dropped.
 *
 * Poll integration:
 * - This class does not call poll(2). Call mergePollfds() before polling to
 *   OR in POLLOUT for fds that have backlog. After poll returns, call
 *   drainQueuesForPolled() with the poll result set to drain ready sockets and
 *   handle errors/closures. All draining is non-blocking.
 *
 * Invariants:
 * - An fd is tracked only while it has pending bytes. Entries with empty
 *   queues are removed immediately after draining. insertAt_() always
 *   initializes the queue with a non-empty message, so empty queues are not
 *   persisted.
 */
class MessageQueueManager {
  public:
	/** @brief Construct an empty manager. Sockets are not owned. */
	MessageQueueManager();
	/** @brief Destructor. Does not close sockets. */
	virtual ~MessageQueueManager();
	/** @brief Copy constructor. Copies tracked queues, pfds, and dead fd list.
	 */
	MessageQueueManager(const MessageQueueManager &other);
	/** @brief Assignment operator. Copies tracked queues, pfds, and dead fd
	 * list. */
	MessageQueueManager &operator=(const MessageQueueManager &other);

	/**
	 * @brief Queue data for non-blocking delivery to fd, attempting an
	 *        immediate one-shot write first.
	 *
	 * Behavior:
	 * - If fd was previously marked dead, the call is a no-op.
	 * - Any existing backlog for fd is first drained (non-blocking).
	 * - If backlog is fully drained, a single non-blocking send(2) of msg is
	 *   attempted (retrying once on EINTR). Unsent bytes, if any, are queued.
	 * - If the socket would block, the full message is queued.
	 * - On fatal errors, fd is recorded in deadFds_ and no data is queued.
	 *
	 * This function does not call poll(2). Use mergePollfds() before your poll
	 * call and drainQueuesForPolled() afterwards to progress queued writes.
	 *
	 * @param fd  Connected socket file descriptor (preferably O_NONBLOCK).
	 * @param msg Bytes to send (appends CRLF etc. should already be present).
	 */
	void send(int fd, const std::string &msg);
	/**
	 * @brief Drain queued data for sockets that were reported by poll(2).
	 *
	 * For each entry in polled:
	 * - If the fd is tracked and revents contains POLLERR/POLLHUP/POLLNVAL,
	 *   the fd is recorded dead and its entry is removed.
	 * - If the fd is tracked and revents contains POLLOUT, queued bytes are
	 *   written with non-blocking send(2) until the queue empties or would
	 *   block again. Empty queues are removed.
	 *
	 * @param polled The vector returned from poll(2) (or your working copy).
	 */
	void drainQueuesForPolled(const std::vector<struct pollfd> &polled);
	/**
	 * @brief Ensure POLLOUT is monitored for sockets with backlog.
	 *
	 * For each pollfd in target that is already present in this manager, ORs
	 * its events with the manager's desired events (currently POLLOUT). This
	 * does not add new pollfd entries; it only modifies existing ones.
	 *
	 * @param target In/out pollfd list used by the caller for poll(2).
	 */
	void mergePollfds(std::vector<struct pollfd> &target) const;
	/**
	 * @brief Drop any queued data and stop tracking fd.
	 *
	 * Does not mark the fd as dead and does not close it.
	 *
	 * @param fd Socket file descriptor to discard.
	 */
	void discard(int fd);
	/**
	 * @brief Report whether fd currently has queued data pending.
	 *
	 * @param fd Socket file descriptor.
	 * @return true if fd is tracked and its queue is non-empty; false if fd is
	 *         not tracked or its queue is empty.
	 */
	bool hasBacklog(int fd) const;
	/**
	 * @brief Report whether any socket has queued data.
	 * @return true if at least one fd is tracked with pending bytes; false if
	 *         no backlogs exist.
	 */
	bool hasBacklog() const;
	/**
	 * @brief Take and clear the list of fds that hit a fatal write error or
	 * were reported closed by poll.
	 *
	 * The returned fds should be closed and removed from higher-level data
	 * structures by the caller.
	 *
	 * @return Vector of dead fds (the internal list is cleared).
	 */
	std::vector<int> takeDeadFds();
	/**
	 * @brief Check if any dead fds are pending retrieval via takeDeadFds().
	 */
	bool			 hasDeadFds() const;

  private:
	std::vector<struct pollfd> pfds_;
	std::vector<MessageQueue>  queues_;
	std::vector<int>		   deadFds_;

	/**
	 * @brief Find fd in the sorted pfds_.
	 * @param fd Socket file descriptor.
	 * @return {found, index}; index is the element index when found, otherwise
	 *         the insertion point to keep sort order.
	 */
	std::pair<bool, std::size_t> findIndexByFd_(int fd) const;
	/**
	 * @brief Insert tracking for fd at the given index and enqueue the initial
	 *        message, enabling POLLOUT.
	 *
	 * pfds_ and queues_ are updated in lockstep. The queue is initialized by
	 * appending 'msg' immediately; empty queues are never left tracked. If the
	 * resulting queue size exceeds MAX_BACKLOG_SIZE, the fd is recorded dead
	 * and the entry is removed.
	 *
	 * @param index Insertion position obtained from findIndexByFd_().
	 * @param fd    Socket file descriptor.
	 * @param msg   Initial bytes to queue for this fd (must be non-empty).
	 * @return true if the entry was inserted and the message queued; false if
	 *         the entry was rejected (overflow) and the fd was recorded dead.
	 */
	bool insertAt_(std::size_t index, int fd, const std::string &msg);
	/**
	 * @brief Append a message to an existing queue and enforce backlog limit.
	 *
	 * If the new total exceeds MAX_BACKLOG_SIZE, the fd at 'index' is
	 * recorded dead and its entry is removed.
	 *
	 * @param index Index of the existing tracked fd (from findIndexByFd_()).
	 * @param msg   Bytes to append (should be non-empty).
	 * @return true if the message was queued; false if the fd was recorded
	 *         dead due to backlog overflow and removed.
	 */
	bool insertMsgAtQueue_(std::size_t index, const std::string &msg);
	/** @brief Remove tracking (pfds_ and queues_) at index. */
	void removeAt_(std::size_t index);
	/** @brief Record the fd at index as dead and remove its entry. */
	void markDeadAndRemove_(std::size_t index);
	/** @brief Check if fd is already present in deadFds_. */
	bool isDead_(int fd) const;

	/**
	 * @brief Drain queued bytes for a tracked fd using non-blocking send(2).
	 *
	 * If the fd is not found, does nothing and returns 0. On fatal errors, the
	 * fd is recorded dead and its entry is removed. When the queue empties, the
	 * entry is removed to keep structures compact.
	 *
	 * @param fd_lookup Result of findIndexByFd_(fd).
	 * @return 0 when no backlog remains and fd is alive;
	 *         1 when some data remains (would block);
	 *        -1 when the fd became dead during draining.
	 */
	int	 drainQueueForFd_(const std::pair<bool, std::size_t> fd_lookup);
	/**
	 * @brief Attempt a single non-blocking send of msg on fd.
	 *
	 * Retries once on EINTR. On partial write, fills remainder with the
	 * unsent tail. On EAGAIN/EWOULDBLOCK, remainder is set to msg. On fatal
	 * errors, fd is recorded dead and false is returned.
	 *
	 * @param fd        Socket file descriptor.
	 * @param msg       Bytes to send.
	 * @param remainder Output: unsent portion (cleared when fully sent).
	 * @return true if fd remains usable (even if remainder is non-empty), false
	 *         if a fatal error occurred and the fd was marked dead.
	 */
	bool sendOnFdWithoutBacklog_(int fd, const std::string &msg,
								 std::string &remainder);
};

#endif // MESSAGEQUEUEMANAGER_HPP
