#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "tablestorage.h"
#include "unicode.h"
#include "exceptions.h"


TableStorage::TableStorage()
{
    try {
        table = Table(toMbcs(getFileName()));
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

