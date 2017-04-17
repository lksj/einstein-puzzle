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

