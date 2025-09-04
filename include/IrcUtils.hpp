#ifndef IRCUTILS_HPP
#define IRCUTILS_HPP

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <sstream>
#include <string>
#include <vector>

template <typename T> std::string toString(const T &value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

// shrink vectors to avoid excessive capacity usage.
template <typename T> static void shrinkVecToFit(std::vector<T> &v) {
	if (v.capacity() > v.size() * 2 && v.capacity() > 64) {
		std::vector<T>(v).swap(v);
	}
}

// Remove an element by swapping it with the back and popping the back.
// Safe no-op if index is out of range.
template <typename T>
inline void removeAndSwapBack(std::vector<T> &v, size_t index) {
	if (index >= v.size())
		return;
	if (index + 1u < v.size())
		std::swap(v[index], v.back());
	v.pop_back();
}

#endif
