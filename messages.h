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


#ifndef __MESSAGES_H__
#define __MESSAGES_H__


#include <map>
#include <string>
#include <cstdarg>


class Resource;
class Formatter;
class Buffer;


/// Localized messages formatter
class Messages
{
    private:
        class ScoredStr {
        public:
            int score;
            Formatter *message;
            ~ScoredStr();
            ScoredStr(int score, Formatter* message);
        };
        typedef std::map<std::wstring, ScoredStr*> StrMap;
        StrMap messages;
    
    public:
        /// Create empty messages table.
        Messages();
        ~Messages();

    public:
        /// Load message tables from resources.
        void load();

        /// Get simple text string
        /// \param key message key
        std::wstring getMessage(const std::wstring &key) const;

        /// Shorter alias for getMessage
        /// \param key message key
        std::wstring operator [](const std::wstring &key) const {
            return getMessage(key);
        }
        
        /// Format message
        /// \param key message key
        std::wstring format(const wchar_t *key, ...) const;
        
        /// Shorter alias for format
        /// \param key message key
        std::wstring operator ()(const wchar_t *key, ...) const;
        
        /// Load messages from resource
        /// \param res resource
        void loadFromResource(Resource *res, Buffer *buffer);

    private:
        void loadBundle(int score, unsigned char *data, size_t size);
        std::wstring format(const wchar_t *key, va_list ap) const;
};


extern Messages msg;


#endif

