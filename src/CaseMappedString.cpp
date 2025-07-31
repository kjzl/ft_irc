#include "CaseMappedString.hpp"
#include <cctype>

// Static helper method to convert a character according to active case mapping
char CaseMappedString::toCaseMapped(char c)
{
#ifdef CASEMAP_ASCII
	// ASCII case mapping: A-Z -> a-z
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
#elif defined(CASEMAP_RFC1459)
	// RFC1459 case mapping: A-Z -> a-z and [\]^ -> {|}~
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	else if (c == '[')
		return '{';
	else if (c == ']')
		return '}';
	else if (c == '\\')
		return '|';
	else if (c == '^')
		return '~';
	return c;
#else
	#error "No case mapping defined. Please define either CASEMAP_ASCII or CASEMAP_RFC1459"
#endif
}

// Static helper method to convert a string according to active case mapping
std::string CaseMappedString::toCaseMappedString(const std::string& str)
{
	std::string	result;

	result.reserve(str.length());
	std::string::const_iterator end = str.end();
	for (std::string::const_iterator it = str.begin(); it != end; ++it)
		result += toCaseMapped(*it);
	return result;
}

CaseMappedString::CaseMappedString() : data_(""), caseMappedData_("")
{

}

CaseMappedString::CaseMappedString(const std::string& str)
	: data_(str), caseMappedData_(toCaseMappedString(str))
{

}

CaseMappedString::CaseMappedString(const char* str)
	: data_(str), caseMappedData_(toCaseMappedString(std::string(str)))
{

}

CaseMappedString::CaseMappedString(const CaseMappedString& other)
	: data_(other.data_), caseMappedData_(other.caseMappedData_)
{

}

CaseMappedString& CaseMappedString::operator=(const CaseMappedString& other)
{

	if (this->data_ != other.data_)
	{
		data_ = other.data_;
		caseMappedData_ = other.caseMappedData_;
	}
	return *this;
}

CaseMappedString& CaseMappedString::operator=(const std::string& str)
{
	if (this->data_ != str)
	{
		data_ = str;
		caseMappedData_ = toCaseMappedString(str);
	}
	return *this;
}

CaseMappedString& CaseMappedString::operator=(const char* str)
{
	if (this->data_ != str)
	{
		data_ = str;
		caseMappedData_ = toCaseMappedString(std::string(str));
	}
	return *this;
}

CaseMappedString::~CaseMappedString()
{

}

bool CaseMappedString::operator<(const CaseMappedString& other) const
{
	return caseMappedData_ < other.caseMappedData_;
}

bool CaseMappedString::operator==(const CaseMappedString& other) const
{
	return caseMappedData_ == other.caseMappedData_;
}

bool CaseMappedString::operator!=(const CaseMappedString& other) const
{
	return !(*this == other);
}

bool CaseMappedString::operator<=(const CaseMappedString& other) const
{
	return *this < other || *this == other;
}

bool CaseMappedString::operator>(const CaseMappedString& other) const
{
	return !(*this <= other);
}

bool CaseMappedString::operator>=(const CaseMappedString& other) const
{
	return !(*this < other);
}

