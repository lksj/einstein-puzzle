#include "messages.h"

#include <iostream>
#include <sstream>
#include "convert.h"
#include "unicode.h"


class TextCommand: public MsgCommand
{
    private:
        std::wstring text;

    public:
        TextCommand(const std::wstring &s): text(s) { };
        
        virtual std::wstring toString() {
            return L"text: '" + text + L"'";
        }
        
        virtual int write(Buffer &buffer) {
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
        ArgCommand(int no) { argNo = no; };
};


class IntArgCommand: public ArgCommand
{
    public:
        IntArgCommand(int no): ArgCommand(no) { };
       
        virtual std::wstring toString() {
            return L"int_arg " + ::toString(argNo);
        }
        
        virtual int write(Buffer &buffer) {
            int sz = buffer.putByte(2);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


class StrArgCommand: public ArgCommand
{
    public:
        StrArgCommand(int no): ArgCommand(no) { };
        virtual std::wstring toString() {
            return L"str_arg " + ::toString(argNo);
        }
        
        virtual int write(Buffer &buffer) {
            int sz = buffer.putByte(3);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


class FloatArgCommand: public ArgCommand
{
    public:
        FloatArgCommand(int no): ArgCommand(no) { };
       
        virtual std::wstring toString() {
            return L"float_arg " + ::toString(argNo);
        }
        
        virtual int write(Buffer &buffer) {
            int sz = buffer.putByte(4);
            sz += buffer.putInteger(4);
            sz += buffer.putInteger(argNo);
            return sz;
        }
};


class DoubleArgCommand: public ArgCommand
{
    public:
        DoubleArgCommand(int no): ArgCommand(no) { };
       
        virtual std::wstring toString() {
            return L"float_arg " + ::toString(argNo);
        }
        
        virtual int write(Buffer &buffer) {
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
    int len = msg.length();
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
    for (Commands::iterator i = commands.begin(); i != commands.end(); i++)
        delete *i;
}

int Message::save(Buffer &buffer)
{
    int sz = buffer.putInteger(commands.size());
    for (Commands::iterator i = commands.begin(); i != commands.end(); i++) {
        MsgCommand *cmd = *i;
        sz += cmd->write(buffer);
    }
    return sz;
}

//////////////////////////////////////////////////////////////////////////
//
// Messages
// 
//////////////////////////////////////////////////////////////////////////

Messages::Messages()
{
}

Messages::~Messages()
{
    for (MsgMap::iterator i = messages.begin(); i != messages.end(); i++)
        delete (*i).second.message;
}

void Messages::add(const std::wstring &key, const std::wstring &msg)
{
    Message *message = new Message(msg);
    
    MsgMap::iterator iter = messages.find(key);
    if (iter != messages.end()) {
        std::cerr << L"Warning: message '" << key << L"' already exists"
            << std::endl;
        delete (*iter).second.message;
    }
    
    MsgEntry entry = { 0, message };
    messages[key] = entry;
}

int Messages::writeHeader(Buffer &buffer)
{
    buffer.putData("CMF", 3);
    int offset = 3;
    offset += buffer.putInteger(1);
    offset += buffer.putInteger(0);
    return offset;
}

int Messages::writeMessages(Buffer &buffer, int offset)
{
    for (MsgMap::iterator i = messages.begin(); i != messages.end(); i++) {
        MsgEntry &e = (*i).second;
        e.offset = offset;
        offset += e.message->save(buffer);
    }
    return offset;
}

int Messages::writeDirectory(Buffer &buffer)
{
    int offset = buffer.putInteger(messages.size());
    for (MsgMap::iterator i = messages.begin(); i != messages.end(); i++) {
        const std::wstring &name = (*i).first;
        offset += buffer.putUtf8(name);
        offset += buffer.putInteger((*i).second.offset);
    }
    return offset;
}

void Messages::save(Buffer &buffer)
{
    int offset = writeHeader(buffer);
    offset = writeMessages(buffer, offset);
    int headerStart = offset;
    writeDirectory(buffer);
    buffer.putInteger(headerStart);
}

