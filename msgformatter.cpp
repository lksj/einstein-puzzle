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


#include "msgformatter.h"

#include "msgwriter.h"
#include "table.h"
#include "unicode.h"


MsgFormatter::MsgFormatter() = default;

void MsgFormatter::format(const std::wstring &fileName, Buffer &output)
{
    Table table(toMbcs(fileName));
    MsgWriter msg;
    for (const auto& i : table)
    {
        msg.add(i.first, i.second->asString());
    }
    msg.save(output);
}

