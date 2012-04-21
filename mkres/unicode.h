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


#ifndef __UNICODE_H__
#define __UNICODE_H__


/// \file unicode.h
/// Definition of UNICODE handling rotinues.


#include <string>
#include <iostream>


/// Convert unicode string to multibyte string in UTF-8 encoding
/// \param str string in unicode
std::string toUtf8(const std::wstring &str);

/// Convert multibyte string in UTF-8 encoding to unicode string
/// \param str string in UTF-8 encoding
std::wstring fromUtf8(const std::string &str);

/// Convert multibyte string in UTF-8 encoding to unicode string
/// \param str string in UTF-8 encoding
/// \param len string length in bytes
std::wstring fromUtf8(const char *str, int len);

/// Convert unicode string to multibyte string in system default encoding
/// \param str string in unicode
std::string toMbcs(const std::wstring &str);

/// Convert unicode string to multibyte string in system default encoding
/// \param str string in unicode
std::wstring fromMbcs(const std::string &str);

/// Convert unicode string to default multibyte encoding
/// and write it to stream.
/// \param stream stream
/// \param str string to output
std::ostream& operator << (std::ostream &stream, const std::wstring &str);

/// Returns length of UTF-8 character in bytes by first UTF-8 character byte.
/// \param c first byte of UTF-8 character.
int getUtf8Length(unsigned char c);

#endif
