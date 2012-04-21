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


#include <list>
#include <map>
#include <string>
#include "buffer.h"


class MsgCommand 
{
    public:
        virtual ~MsgCommand() { };

    public:
        virtual std::wstring toString() = 0;
        virtual int write(Buffer &buffer) = 0;
};


class Message
{
    private:
        typedef std::list<MsgCommand*> Commands;
        Commands commands;

    public:
        Message(const std::wstring &msg);
        ~Message();
        int save(Buffer &buffer);
};


class Messages
{
    private:
        typedef struct {
            int offset;
            Message *message;
        } MsgEntry;
        typedef std::map<std::wstring, MsgEntry> MsgMap;
        MsgMap messages;
    
    public:
        Messages();
        ~Messages();

    public:
        void add(const std::wstring &key, const std::wstring &msg);
        void save(Buffer &buffer);

    private:
        int writeHeader(Buffer &buffer);
        int writeMessages(Buffer &buffer, int offset);
        int writeDirectory(Buffer &buffer);
};


#endif

