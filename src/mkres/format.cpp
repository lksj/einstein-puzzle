#include "format.h"
#include "msgformatter.h"


FormatRegistry formatRegistry;


FormatRegistry::FormatRegistry()
{
    formatters[L"messages"] = new MsgFormatter();
}


FormatRegistry::~FormatRegistry()
{
    for (FormattersMap::iterator i = formatters.begin(); 
            i != formatters.end(); i++)
        delete (*i).second;
}


Formatter* FormatRegistry::get(const std::wstring &name)
{
    return formatters[name];
}

