#include "../include/MessageQueue.hpp"
#include "../include/Debug.hpp"

MessageQueue::MessageQueue() : totalBytes_(0) {
	debug("MessageQueue default constructor called");
}

MessageQueue::MessageQueue(const MessageQueue &other) {
	parts_		= other.parts_;
	totalBytes_ = other.totalBytes_;
}

MessageQueue &MessageQueue::operator=(const MessageQueue &other) {
	if (this != &other) {
		parts_		= other.parts_;
		totalBytes_ = other.totalBytes_;
	}
	return *this;
}

MessageQueue::~MessageQueue() { debug("MessageQueue destructor called"); }

size_t MessageQueue::totalBytes() const { return totalBytes_; }

bool MessageQueue::empty() const { return parts_.empty(); }

size_t MessageQueue::size() const { return parts_.size(); }

std::string &MessageQueue::front() { return parts_.front(); }

const std::string &MessageQueue::front() const { return parts_.front(); }

void MessageQueue::pushBack(const std::string &part) {
	parts_.push_back(part);
	totalBytes_ += part.size();
}

void MessageQueue::popFront() {
	if (parts_.empty())
		return;
	totalBytes_ -= parts_.front().size();
	parts_.pop_front();
}

/**
 * @brief Removes bytes from the very first part in the queue.
 *
 * Removes exactly n bytes from the front-most part. If n equals the size
 * of the front part, that part is removed from the queue.
 *
 * @param n Number of bytes to remove from the front part.
 * @throws std::logic_error If n is greater than the size of the front part.
 */
void MessageQueue::removeBytesFromFront(size_t n) {
	if (n == 0)
		return;

	std::string &s = parts_.front();
	if (n >= s.size()) {
		totalBytes_ -= s.size();
		parts_.pop_front();
	} else {
		s.erase(0, n);
		totalBytes_ -= n;
	}
}

void MessageQueue::clear() {
	parts_.clear();
	totalBytes_ = 0;
}
