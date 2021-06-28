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


#include "msgwriter.h"

#include "convert.h"
#include "unicode.h"

#include <iostream>
#include <sstream>


class TextCommand: public MsgCommand
{
    private:
        std::wstring text;

    public:
        explicit TextCommand(const std::wstring &s): text(s) { }
        
        std::wstring toString()  override {
            return L"text: '" + text + L"'";
        }
        
        int write(Buffer &buffer) override {
            int sz = buffer.putByte(1);
            //sz += writeInt(stream, text.length());
            sz += buffer.putUtf8(text);
            return sz;
        }
};

class ArgCommand: public MsgCommand
{
    protected:
        int argNo;

    public:
        explicit ArgCommand(int no) { argNo = no; }
};


class IntArgCommand: public ArgCommand
{
    public:
        explicit IntArgCommand(int no): ArgCommand(no) { }
       
        std::wstring toString() override {
            return L"int_arg " + ::toString(argNo);
        }
        
        int write(Buffer &buffer) override {
            int sz = buffer.putByte(2);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


class StrArgCommand: public ArgCommand
{
    public:
        explicit StrArgCommand(int no): ArgCommand(no) { }
        std::wstring toString() override {
            return L"str_arg " + ::toString(argNo);
        }
        
        int write(Buffer &buffer) override {
            int sz = buffer.putByte(3);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


class FloatArgCommand: public ArgCommand
{
    public:
        explicit FloatArgCommand(int no): ArgCommand(no) { }
       
        std::wstring toString() override {
            return L"float_arg " + ::toString(argNo);
        }
        
        int write(Buffer &buffer) override {
            int sz = buffer.putByte(4);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


class DoubleArgCommand: public ArgCommand
{
    public:
        explicit DoubleArgCommand(int no): ArgCommand(no) { }
       
        std::wstring toString() override {
            return L"float_arg " + ::toString(argNo);
        }
        
        int write(Buffer &buffer) override {
            int sz = buffer.putByte(5);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


//////////////////////////////////////////////////////////////////////////
//
// Message
// 
//////////////////////////////////////////////////////////////////////////


Message::Message(const std::wstring &msg)
{
    const int len = msg.length();
    std::wstring str;
    int i = 0;
    bool numbersUsed = false;
    int currentArg = 1;
    
    while (i < len) {
        wchar_t ch = msg[i];
        if ((L'%' == ch) && (i != len - 1)) {
            ch = msg[++i];
            if (L'%' != ch) {
                if (str.length()) {
                    commands.push_back(new TextCommand(str));
                    str.clear();
                }
                while ((ch >= L'0') && (ch <= L'9') && (i < len)) {  // read arg no
                    str += ch;
                    ch = msg[++i];
                }
                if (i == len)
                    throw Exception(L"Escape sequence is not finished");
                if (L'$' == ch) { // skeep $ sign
                    ch = msg[++i];
                    if (i == len)
                        throw Exception(L"Escape sequence is not finished");
                }
                int argNo;
                if (str.length()) {
                    argNo = strToInt(str);
                    str.clear();
                    numbersUsed = true;
                } else {
                    if (numbersUsed)
                        throw Exception(L"Can't use unnumbered arguments"
                                L" if numbered was used before");
                    argNo = currentArg++;
                }
                if (argNo <= 0)
                    throw Exception(L"Invalid argument number");
                MsgCommand *cmd;
                switch (ch) {
                    case L'd':
                    case L'i': cmd = new IntArgCommand(argNo); break;
                    case L's': cmd = new StrArgCommand(argNo); break;
                    case L'f': cmd = new FloatArgCommand(argNo); break;
                    case L'e': cmd = new DoubleArgCommand(argNo); break;
                    default: 
                          throw Exception(std::wstring(L"Format string '%")
                                  + ch + std::wstring(L"' is not supported"));
                }
                commands.push_back(cmd);
            } else
                str += ch;
        } else
            str += ch;
        i++;
    }
    
    if (str.length())
        commands.push_back(new TextCommand(str));
}


Message::~Message()
{
    for (auto& command : commands)
        delete command;
}

int Message::save(Buffer &buffer)
{
    int sz = buffer.putInteger(commands.size());
    for (auto cmd : commands)
    {
        sz += cmd->write(buffer);
    }
    return sz;
}

//////////////////////////////////////////////////////////////////////////
//
// MsgWriter
// 
//////////////////////////////////////////////////////////////////////////

MsgWriter::MsgWriter() = default;

MsgWriter::~MsgWriter()
{
    for (auto& message : messages)
        delete message.second.message;
}

void MsgWriter::add(const std::wstring &key, const std::wstring &msg)
{
    Message *message = new Message(msg);
    
    const MsgMap::iterator iter = messages.find(key);
    if (iter != messages.end()) {
        std::cerr << L"Warning: message '" << key << L"' already exists"
            << std::endl;
        delete (*iter).second.message;
    }
    
    const MsgEntry entry = { 0, message };
    messages[key] = entry;
}

int MsgWriter::writeHeader(Buffer &buffer)
{
    buffer.putData("CMF", 3);
    int offset = 3;
    offset += buffer.putInteger(1);
    offset += buffer.putInteger(0);
    return offset;
}

int MsgWriter::writeMessages(Buffer &buffer, int offset)
{
    for (auto& message : messages)
    {
        MsgEntry &e = message.second;
        e.offset = offset;
        offset += e.message->save(buffer);
    }
    return offset;
}

int MsgWriter::writeDirectory(Buffer &buffer)
{
    int offset = buffer.putInteger(messages.size());
    for (auto& message : messages)
    {
        const std::wstring &name = message.first;
        offset += buffer.putUtf8(name);
        offset += buffer.putInteger(message.second.offset);
    }
    return offset;
}

void MsgWriter::save(Buffer &buffer)
{
    int offset = writeHeader(buffer);
    offset = writeMessages(buffer, offset);
    const int headerStart = offset;
    writeDirectory(buffer);
    buffer.putInteger(headerStart);
}

