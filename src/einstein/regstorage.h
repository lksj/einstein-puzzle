#ifndef __REGSTORAGE_H__
#define __REGSTORAGE_H__
#ifdef WIN32                    // Win32 only


#include <windows.h>
#include "storage.h"


class RegistryStorage: public Storage
{
    private:
        HKEY globalKey;
        HKEY userKey;

    public:
        RegistryStorage();
        virtual ~RegistryStorage();

    public:
        virtual int get(const std::wstring &name, int dflt) ;
        virtual std::wstring get(const std::wstring &name, 
                const std::wstring &dflt);
        virtual void set(const std::wstring &name, int value);
        virtual void set(const std::wstring &name, const std::wstring &value);
        virtual void flush() { };
};


#endif
#endif

