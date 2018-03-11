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


#include "formatter.h"
#include "utils.h"
#include "convert.h"


#define ADD_ARG(t) \
                commands[commandsCnt].type = t; \
                argNo = readInt(data + offset); \
                if (argNo > maxArg) \
                    maxArg = argNo; \
                commands[commandsCnt].data = (void*)argNo; \
                commandsCnt++;

Formatter::Formatter(unsigned char *data, int offset)
{
    int cnt = readInt(data + offset);
    if (! cnt) {
        commandsCnt = argsCnt = 0;
        commands = NULL;
        args = NULL;
    }
    
    offset += 4;
    commands = new Command[cnt];
    commandsCnt = 0;
    
    int maxArg = 0, argNo;
    
    for (int i = 0; i < cnt; i++) {
        int type = data[offset];
        offset++;
        int size = readInt(data + offset);
        offset += 4;
        switch (type) {
            case 1:
                commands[commandsCnt].type = TEXT_COMMAND;
                commands[commandsCnt].data = new std::wstring(
                        fromUtf8((char*)data + offset, size));
                commandsCnt++;
                break;
            
            case 2: ADD_ARG(INT_ARG); break;
            case 3: ADD_ARG(STRING_ARG); break;
            case 4: ADD_ARG(FLOAT_ARG); break;
            case 5: ADD_ARG(DOUBLE_ARG); break;
        }
        offset += size;
    }

    argsCnt = maxArg;
    if (! argsCnt)
        args = NULL;
    else {
        args = new CmdType[argsCnt];
        memset(args, 0, sizeof(CmdType) * argsCnt);
        for (int i = 0; i < commandsCnt; i++) {
            Command &c = commands[i];
            if ((c.type == INT_ARG) || (c.type == STRING_ARG) ||
                    (c.type == FLOAT_ARG) || (c.type == DOUBLE_ARG))
            {
                long no = (long)c.data;
                args[no - 1] = c.type;
            }
        }
    }
}

Formatter::~Formatter()
{
    for (int i = 0; i < commandsCnt; i++)
        if (TEXT_COMMAND == commands[i].type)
            delete (std::wstring*)(commands[i].data);
    delete[] commands;
    delete[] args;
}

std::wstring Formatter::getMessage() const
{
    std::wstring s;

    for (int i = 0; i < commandsCnt; i++)
        if (TEXT_COMMAND == commands[i].type)
            s += *(std::wstring*)(commands[i].data);
    return s;
}


class ArgValue
{
    public:
        virtual ~ArgValue() = default;
        virtual std::wstring format(Formatter::Command *command) = 0;
};

template <typename T>
class TemplatedArgValue: public ArgValue
{
    private:
        T value;
    
    public:
        TemplatedArgValue(const T &v) { value = v; };
        std::wstring format(Formatter::Command *command) override { 
            return toString(value);
        };
};

class StrArgValue: public ArgValue
{
    private:
        std::wstring value;

    public:
        StrArgValue(const std::wstring &v): value(v) { };
        std::wstring format(Formatter::Command *command) override {
            return value;
        };
};


std::wstring Formatter::format(std::vector<ArgValue*> &argValues) const
{
    std::wstring s;
    long no;

    for (int i = 0; i < commandsCnt; i++) {
        Command *cmd = &commands[i];

        switch (cmd->type) {
            case TEXT_COMMAND:
                s += *(std::wstring*)(cmd->data);
                break;
                
            case STRING_ARG:
            case INT_ARG:
                no = (long)cmd->data - 1;
                if (no < (long)argValues.size())
                    s += argValues[no]->format(cmd);
                break;

            default: ;
        }
    }
    
    return s;
}

std::wstring Formatter::format(va_list ap) const
{
    if (! argsCnt)
        return getMessage();
    
    std::vector<ArgValue*> argValues;
    
    for (int i = 0; i < argsCnt; i++) {
        switch (args[i]) {
            case INT_ARG:
                argValues.push_back(new TemplatedArgValue<int>
                        (va_arg(ap, int))); 
                break;
            case STRING_ARG:
                argValues.push_back(new StrArgValue(va_arg(ap, wchar_t*)));
                break;
            case DOUBLE_ARG:
                argValues.push_back(new TemplatedArgValue<double>
                        (va_arg(ap, double)));
                break;
            case FLOAT_ARG:
                argValues.push_back(new TemplatedArgValue<float>
                        ((float)va_arg(ap, double)));
                break;
            default:
                i = argsCnt;
        }
    }
 
    std::wstring s = format(argValues);

    for (auto& argValue : argValues)
        delete argValue;
    
    return s;
}

