// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2018-02-11 by Jordan Evens <jordan.evens@gmail.com>

// Einstein Puzzle is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// Einstein Puzzle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef __CONVERT_H__
#define __CONVERT_H__


#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

#include "exceptions.h"
#include "unicode.h"


/// Convert value to string
/// \param x value
template <typename T>
inline std::wstring toString(const T &x)
{
    std::wostringstream o;
    if (! (o << x))
        throw Exception(L"Can't convert " + fromMbcs(typeid(x).name())
                + L" to string");
    return o.str();
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
