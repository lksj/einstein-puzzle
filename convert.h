#ifndef __CONVERT_H__
#define __CONVERT_H__


#include <iostream>
#include <sstream>
#include <string>

#include "exceptions.h"
#include "unicode.h"


/// Convert value to string
/// \param x value
template <typename T>
inline std::wstring toString(const T &x)
{
#ifndef WIN32
    std::wostringstream o;
    if (! (o << x))
        throw Exception(L"Can't convert " + fromMbcs(typeid(x).name())
                + L" to string");
    return o.str();
#else   // Mingw doesn't support std::wostringstream yet :-(
    std::ostringstream o;
    if (! (o << x))
        throw Exception(L"Can't convert " + fromMbcs(typeid(x).name()) 
                + L" to string");
    return fromMbcs(o.str());
#endif
}


/// Convert string to lower case.
std::wstring toLowerCase(const std::wstring &s);

/// Convert string to upper case
std::wstring toUpperCase(const std::wstring &s);

/// Convert integer to string.
std::wstring numToStr(int num);

/// Convert unsigned integer to string.
std::wstring numToStr(unsigned int num);

/// Convert string to integer
int strToInt(const std::wstring &str);

/// Conver string to double
double strToDouble(const std::wstring &str);


#endif
