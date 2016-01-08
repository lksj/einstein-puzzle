#ifndef __STORAGE_H__
#define __STORAGE_H__


#include <string>


class Storage
{
    public:
        virtual ~Storage() { };

    public:
        virtual int get(const std::wstring &name, int dflt) = 0;
        virtual std::wstring get(const std::wstring &name, 
                const std::wstring &dflt) = 0;
        virtual void set(const std::wstring &name, int value) = 0;
        virtual void set(const std::wstring &name, const std::wstring &value) = 0;
        virtual void flush() = 0;
};


Storage* getStorage();


#endif

