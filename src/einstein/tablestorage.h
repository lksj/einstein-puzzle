#ifndef __TABLESTORAGE_H__
#define __TABLESTORAGE_H__


#include "storage.h"
#include "table.h"


class TableStorage: public Storage
{
    private:
        Table table;
    
    public:
        TableStorage();
        virtual ~TableStorage();

    public:
        virtual int get(const std::wstring &name, int dflt);
        virtual std::wstring get(const std::wstring &name, 
                const std::wstring &dflt);
        virtual void set(const std::wstring &name, int value);
        virtual void set(const std::wstring &name, const std::wstring &value);
        virtual void flush();

    private:
        std::wstring getFileName();
};


#endif

