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


#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <stdlib.h>
#include <string>


/// Dynamic growing buffer
class Buffer
{
    private:
        size_t size;
        size_t allocated;
        void *data;
        int currentPos;
    
    public:
        /// Create buffer
        /// \param size initial size of buffer
        /// \param allocated bytes to allocate
        Buffer(int size=0, int allocated=1024);
        
        ~Buffer();

    public:
        /// Set buffer size, expands memory if needed
        /// \param size new size
        void setSize(size_t size);

        /// Get current size of buffer
        size_t getSize();

        /// Get actual bytes used by buffer
        size_t getAllocated();

        /// Get pointer to data
        void* getData();

    public:
        /// Move pointer to specified position.
        /// \param offset offset from buffer start
        void gotoPos(int offset);

        /// Add data to buffer and advance current position by data length.
        /// Grow buffer if needed.
        /// \param data pointer to data
        /// \param length data size.
        size_t putData(const unsigned char *data, size_t length);
        
        /// Add data to buffer and advance current position by data length.
        /// Grow buffer if needed.
        /// \param data pointer to data
        /// \param length data size.
        size_t putData(const char *data, size_t length) {
            return putData((const unsigned char*)data, length);
        };

        /// Add integer to buffer and advance current position by 4.
        /// Grow buffer if needed.
        /// \param value value to add.
        size_t putInteger(int value);

        /// Add string to buffer encoded in UTF-8 and advance current 
        /// position by string length.  String stored prefixed by
        /// string length.
        /// Grow buffer if needed.
        /// \param string value to add.
        size_t putUtf8(const std::wstring &string);
        
        /// Add byte to buffer and advance current position by 1.
        /// Grow buffer if needed.
        /// \param value value to add.
        size_t putByte(unsigned char value);
};


#endif

