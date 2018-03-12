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


#ifndef __FORMAT_H__
#define __FORMAT_H__


#include "buffer.h"

#include <map>


class Formatter
{
    public:
        virtual ~Formatter() = default;
        virtual void format(const std::wstring &fileName, Buffer &output) = 0;
};


class FormatRegistry
{
    private:
        typedef std::map<std::wstring, Formatter*> FormattersMap;
        FormattersMap formatters;

    public:
        FormatRegistry();
        ~FormatRegistry();

    public:
        Formatter* get(const std::wstring &name);
};


extern FormatRegistry formatRegistry;


#endif

