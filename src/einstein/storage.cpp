#include "storage.h"

#ifndef WIN32
#include "tablestorage.h"
#else
#include "regstorage.h"
#endif


class StorageHolder
{
    private:
        Storage *storage;

    public:
        StorageHolder();
        ~StorageHolder();

    public:
        Storage* getStorage() { return storage; };
};


StorageHolder::StorageHolder()
{
#ifndef WIN32
    storage = new TableStorage();
#else
    storage = new RegistryStorage();
#endif
}


StorageHolder::~StorageHolder() 
{ 
    if (storage) 
        delete storage; 
}


static StorageHolder storageHolder;


Storage* getStorage()
{
    return storageHolder.getStorage();
}

