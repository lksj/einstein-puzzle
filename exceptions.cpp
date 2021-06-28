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


#include "exceptions.h"

#include <string>
#include <locale>
#include <codecvt>
#include <stdexcept>
#include <type_traits>

using convert_type = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_type, wchar_t> converter;

Exception::Exception(const char* msg) noexcept:
    std::runtime_error(msg)
{
}

Exception::Exception(const std::wstring& msg) noexcept:
    std::runtime_error(converter.to_bytes(msg).c_str())
{
}

const std::wstring Exception::getMessage() const noexcept
{
    return std::wstring(converter.from_bytes(what()));
}

static_assert(std::is_nothrow_copy_constructible<Exception>::value, "Exception must be nothrow copy constructible");
