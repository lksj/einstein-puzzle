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


#include "streams.h"

#include "exceptions.h"
#include "unicode.h"


UtfStreamReader::UtfStreamReader(std::ifstream *s)
{
    stream = s;
}

UtfStreamReader::~UtfStreamReader() = default;

// This function is very slow because of poor fromUtf8 function design
wchar_t UtfStreamReader::getNextChar()
{
    unsigned char buf[10];

    if (!backBuf.empty()) {
        wchar_t wc = backBuf.front();
        backBuf.pop_front();
        return wc;
    }
    
    if (! stream->good())
        throw Exception(L"Error reading from stream 1");

    int sz = stream->readsome((char*)buf, 1);
    if (1 != sz)
        throw Exception(L"Error reading from stream 2");
    int size = getUtf8Length(buf[0]);
    if (size > 1) {
        sz = stream->readsome((char*)buf + 1, size - 1);
        if (size - 1 != sz)
            throw Exception(L"Error reading from stream 3");
    }
    buf[size] = 0;
    std::string s((char*)buf);
    std::wstring ws(fromUtf8(s));
    if (1 != ws.length())
        throw Exception(L"Error converting UTF-8 character to wide character");
    return ws[0];
}

void UtfStreamReader::ungetChar(wchar_t ch)
{
    backBuf.push_back(ch);
}

bool UtfStreamReader::isEof()
{
    if (stream->eof())
        return true;     // FIXME: it doesn't work. why?
    return EOF == stream->peek();
}

