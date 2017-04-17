#include "msgformatter.h"
#include "table.h"
#include "unicode.h"
#include "messages.h"


MsgFormatter::MsgFormatter()
{
}

void MsgFormatter::format(const std::wstring &fileName, Buffer &output)
{
    Table table(toMbcs(fileName));
    Messages msg;
    for (Table::Iterator i = table.begin(); i != table.end(); i++) {
        msg.add((*i).first, (*i).second->asString());
    }
    msg.save(output);
}

