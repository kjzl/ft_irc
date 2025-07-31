#include <sstream>
#include <string>

template <typename T>
std::string	toString(T in)
{
	std::ostringstream oss;
	oss << in;
	std::string inString = oss.str();
	return (inString);
}
