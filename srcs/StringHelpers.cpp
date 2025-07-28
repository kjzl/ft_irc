#include "StringHelpers.hpp"

bool str_is_upper(const std::string& s)
{
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
    {
        if (!std::isupper(static_cast<unsigned char>(*it)))
        {
            return false;
        }
    }
    return true;
}
