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


#ifndef __FORMATTER_H__
#define __FORMATTER_H__


#include <stdarg.h>
#include <string>
#include <vector>


class ArgValue;


/// Localized message formatter
class Formatter
{
    public:
        typedef enum {
            EMPTY_CMD = 0,
            TEXT_COMMAND,
            INT_ARG,
            STRING_ARG,
            DOUBLE_ARG,
            FLOAT_ARG
        } CmdType;
        
        typedef struct
        {
            CmdType type;
            void *data;
        } Command;

    private:
        int commandsCnt;
        int argsCnt;

        Command *commands;
        
        CmdType *args;

    public:
        /// Create localized message from message buffer.
        /// \param data buffer contained message file
        /// \param offset offset to message from buffer start
        Formatter(unsigned char *data, int offset);
        ~Formatter();

    public:
        /// Get message text.
        std::wstring getMessage() const;
        
        /// Fromat message
        /// \param ap list of arguments
        std::wstring format(va_list ap) const;

    private:
        std::wstring format(std::vector<ArgValue*> &argValues) const;
};




#endif

