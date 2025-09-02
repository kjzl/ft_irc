#ifndef MESSAGEQUEUE_HPP
#define MESSAGEQUEUE_HPP

#include <deque>
#include <string>

/**
 * @brief A queue of messages which tracks the total byte size.
 */
class MessageQueue {
  public:
	MessageQueue();
	MessageQueue(const MessageQueue &other);
	MessageQueue &operator=(const MessageQueue &other);
	virtual ~MessageQueue();
	size_t			   totalBytes() const;
	bool			   empty() const;
	size_t			   size() const;
	std::string		  &front();
	const std::string &front() const;
	void			   pushBack(const std::string &part);
	void			   popFront();
	void			   removeBytesFromFront(size_t n);
	void			   clear();

  private:
	std::deque<std::string> parts_;
	size_t					totalBytes_;
};

#endif // MESSAGEQUEUE_HPP
