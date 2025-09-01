#ifndef IRCUTILS_HPP
#define IRCUTILS_HPP

#include <string>
#include <sstream>

void safeSend(int fd, const std::string &msg);
template <typename T>
std::string toString(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

#endif
