#ifndef __FORMAT_H__
#define __FORMAT_H__


#include <map>
#include "buffer.h"


class Formatter
{
    public:
        virtual ~Formatter() { };
        virtual void format(const std::wstring &fileName, Buffer &output) = 0;
};


class FormatRegistry
{
    private:
        typedef std::map<std::wstring, Formatter*> FormattersMap;
        FormattersMap formatters;

    public:
        FormatRegistry();
        ~FormatRegistry();

    public:
        Formatter* get(const std::wstring &name);
};


extern FormatRegistry formatRegistry;


#endif

