#include <sstream>
#include <string>

std::string	intToStr(int in)
{
	std::ostringstream oss;
	oss << in;
	std::string inString = oss.str();
	return (inString);
}
