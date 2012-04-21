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
#include <exception>


/// Basic exception class
class Exception: public std::exception
{
    private:
        std::wstring message;
    
    public:
        /// Create exception
        /// \param msg message text
        Exception(const std::wstring& msg) throw() { message = msg; };
        virtual ~Exception() throw() { };

    public:
        /// Get error message.
        virtual const std::wstring& getMessage() const throw() { return message; };

        /// Inherited way of receiving error messags
        virtual const char* what() const throw() { return "Exception"; };
};


#endif

