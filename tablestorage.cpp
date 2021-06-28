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


#include "tablestorage.h"

#include "exceptions.h"
#include "unicode.h"

#include <iostream>

#ifndef WIN32
#include <sys/stat.h>
#endif

TableStorage::TableStorage()
{
    std::wstring name = getFileName();
#ifndef WIN32
    std::string sname(name.begin(), name.end());
    // HACK: make the config file and directory if they don't exist
    struct stat buffer;
    if (stat (sname.c_str(), &buffer) != 0)
    {
        system("mkdir ~/.einstein");
        system("touch ~/.einstein/einsteinrc");
    }
#endif
    try {
        table = Table(toMbcs(name));
    } catch (Exception &e) {
        std::cerr << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << "Unknown config file error" << std::endl;
    }
}

TableStorage::~TableStorage()
{
    flush();
}

std::wstring TableStorage::getFileName()
{
#ifndef WIN32
    return std::wstring(fromMbcs(getenv("HOME"))) + L"/.einstein/einsteinrc";
#else
    return L"einstein.cfg";
#endif
}

int TableStorage::get(const std::wstring &name, int dflt)
{
    return table.getInt(name, dflt);
}

std::wstring TableStorage::get(const std::wstring &name, 
            const std::wstring &dflt)
{
    return table.getString(name, dflt);
}

void TableStorage::set(const std::wstring &name, int value)
{
    table.setInt(name, value);
}

void TableStorage::set(const std::wstring &name, const std::wstring &value)
{
    table.setString(name, value);
}

void TableStorage::flush()
{
    table.save(getFileName());
}

