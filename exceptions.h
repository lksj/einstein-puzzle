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


#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__


#include <string>
#include <iostream>


class Exception
{
    private:
        std::wstring message;
    
    public:
        explicit Exception(const std::wstring& msg) : message(msg) {};
        virtual ~Exception() { };

    public:
        const std::wstring& getMessage() const { return message; };
};


#endif

