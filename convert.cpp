// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

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


#include "convert.h"

std::wstring toLowerCase(const std::wstring &s)
{
    std::wstring res;

    int len = s.length();
    for (int i = 0; i < len; i++)
        res += (wchar_t)towlower(s[i]);

    return res;
}

std::wstring toUpperCase(const std::wstring &s)
{
    std::wstring res;

    int len = s.length();
    for (int i = 0; i < len; i++)
        res += (wchar_t)towupper(s[i]);

    return res;
}

std::wstring numToStr(int num)
{
    wchar_t buf[30];
#ifdef WIN32
    swprintf(buf, L"%i", num);
#else
    swprintf(buf, 29, L"%i", num);
#endif
    buf[29] = 0;
    return std::wstring(buf);
}

std::wstring numToStr(unsigned int num)
{
    wchar_t buf[30];
#ifdef WIN32
    swprintf(buf, L"%u", num);
#else
    swprintf(buf, 29, L"%u", num);
#endif
    buf[29] = 0;
    return std::wstring(buf);
}

int strToInt(const std::wstring &str)
{
    int n;
    wchar_t *endptr;

    n = wcstol(str.c_str(), &endptr, 10);
    if ((! str.c_str()[0]) || (endptr[0])) 
        throw Exception(L"Invalid integer '" + str + L"'");
    return n;
}

double strToDouble(const std::wstring &str)
{
    double n;
    wchar_t *endptr;

    n = wcstod(str.c_str(), &endptr);
    if ((! str.c_str()[0]) || (endptr[0])) 
        throw Exception(L"Invalid double '" + str + L"'");
    return n;
}
