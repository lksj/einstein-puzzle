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


#ifndef __TABLESTORAGE_H__
#define __TABLESTORAGE_H__


#include "storage.h"
#include "table.h"


class TableStorage: public Storage
{
    private:
        Table table;
    
    public:
        TableStorage();
        virtual ~TableStorage();

    public:
        int get(const std::wstring &name, int dflt) override;
        std::wstring get(const std::wstring &name, const std::wstring &dflt) override;
        void set(const std::wstring &name, int value) override;
        void set(const std::wstring &name, const std::wstring &value) override;
        void flush() override;

    private:
        std::wstring getFileName();
};


#endif

