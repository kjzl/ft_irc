#ifndef CASEMAPPEDSTRING_HPP
#define CASEMAPPEDSTRING_HPP

#include <string>
// https://modern.ircdocs.horse/#casemapping-parameter
//#define CASEMAP_ASCII
#define CASEMAP_RFC1459

class CaseMappedString {
private:
	std::string data_;
	std::string caseMappedData_;

	// Static helper methods for case mapping
	static char toCaseMapped(char c);
	static std::string toCaseMappedString(const std::string& str);

public:
	CaseMappedString();
	CaseMappedString(const std::string& str);
	CaseMappedString(const char* str);
	CaseMappedString(const CaseMappedString& other);
	CaseMappedString& operator=(const CaseMappedString& other);
	CaseMappedString& operator=(const std::string& str);
	CaseMappedString& operator=(const char* str);
	virtual ~CaseMappedString();

    const std::string& str() const;
    operator const std::string&() const;

    // String utility functions
    bool empty() const;
    size_t length() const;
    size_t size() const;
    void clear();
    const char* c_str() const;
    char at(size_t pos) const;
    char operator[](size_t pos) const;

    bool operator<(const CaseMappedString& other) const;
    bool operator==(const CaseMappedString& other) const;
    bool operator!=(const CaseMappedString& other) const;
    bool operator<=(const CaseMappedString& other) const;
    bool operator>(const CaseMappedString& other) const;
    bool operator>=(const CaseMappedString& other) const;
};

#endif // CASEMAPPEDSTRING_HPP
