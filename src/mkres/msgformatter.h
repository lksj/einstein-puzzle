#ifndef __MSG_FORMATTER_H__
#define __MSG_FORMATTER_H__


#include "format.h"


class MsgFormatter: public Formatter
{
    public:
        MsgFormatter();
        virtual void format(const std::wstring &fileName, Buffer &output);
};


#endif

