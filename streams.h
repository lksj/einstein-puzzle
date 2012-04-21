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


#ifndef __STREAMS_H__
#define __STREAMS_H__


#include <fstream>
#include <list>


/// Read utf-8 file and convert it to wide characters
class UtfStreamReader
{
    private:
        /// Pointer to file stream
        std::ifstream *stream;

        /// Push back buffet
        std::list<wchar_t> backBuf;
    
    public:
        /// Create utf-8 stream reader.
        /// \param stream pointer to file stream.
        UtfStreamReader(std::ifstream *stream);
        
        /// Destructor
        ~UtfStreamReader();

    public:
        /// Read next unicode character.
        wchar_t getNextChar();

        /// Push back character.
        /// \param ch character to push back
        void ungetChar(wchar_t ch);

        /// Check if end of file reached.
        bool isEof();
};


#endif

