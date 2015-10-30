#ifndef __RESOURCES_H__
#define __RESOURCES_H__

/** \file resources.h
 * Classes for resources loading and unloading.
 */

#include <string>
#include <fstream>
#include <list>
#include <map>
#include <vector>

#include "visitor.h"
#include "buffer.h"

typedef std::list<std::wstring> StringList;


class ResourceFile;


/// Abstract interface for streamed resource access
class ResourceStream
{
    public:
        virtual ~ResourceStream() { };
        
        /// Get size of resource
        virtual size_t getSize() = 0;

        /// Seek into resource.
        /// \param offset offset from resource start
        virtual void seek(long offset) = 0;

        /// Read data from resource into buffer.
        /// \param buffer buffer of size bytes.
        /// \param size number of bytes to read.
        virtual void read(char *buffer, size_t size) = 0;

        /// Get current position
        virtual long getPos() = 0;

        /// Return true if end of resource reached
        virtual bool isEof() { return (size_t)getPos() >= getSize(); };

        /// Get count of bytes left
        virtual long getAvailable() { return (long)getSize() - getPos(); };
};


/*** 
   * Low level resource file interface.
   * Never use it, use ResourcesCollection instead.
   ***/
class ResourceFile
{
    private:
        std::ifstream stream;           /// resource file input stream
        int priority;                   /// priority of resource file
        std::wstring name;              /// resource file name
        Buffer *buffer;
        bool ownBuffer;
 
    public:
        /// Resource file directory entry
        typedef struct {
            std::wstring name;           /// resource name
            long offset;                /// offset from start of resource file
            long packedSize;            /// compressed size
            long unpackedSize;          /// uncompressed size
            std::wstring group;          /// group name
            int level;                  /// pack level
        } DirectoryEntry;
        /// List of directory entries.
        typedef std::list<DirectoryEntry> Directory;
        
    public:
        /// Create resource file.  Throws exception if file can't be opened.
        /// \param fileName the name of resource file.
        /// \param buffer buffer for temporary data.
        /// Can be shared with other resource files.
        ResourceFile(const std::wstring &fileName, Buffer *buffer=NULL);
        virtual ~ResourceFile();

    public:
        /// Load directory listing.
        /// \param directory list where resource entries will be placed.
        void getDirectory(Directory &directory);

        /// Load data in preallocated buffer buf of size unpackedSize.
        /// \param buf buffer of unpackedSize bytes where unpacked data will
        /// be placed
        /// \param offset offset from start of resource file to packed data
        /// \param packedSize size of packed resource
        /// \param unpackedSize size of unpacked resource
        void load(char *buf, long offset, long packedSize, 
                long unpackedSize, int level);
        
        /// Allocate buffer and load data.  Memory returned by this 
        ///  method must be freed by free() function call.
        /// \param offset offset from start of resource file to packed data
        /// \param packedSize size of packed resource
        /// \param unpackedSize size of unpacked resource
        void* load(long offset, long packedSize, long unpackedSize, 
                int level);

        /// Get priority of this resource file.
        int getPriority() const { return priority; };
        
        /// Get the name of resource file.
        const std::wstring& getFileName() const { return name; };

        /// Get file stream.
        std::ifstream& getStream() { return stream; };

    private:
        /// Unpack memory buffer
        /// \param in buffer of inSize bytes filled with packed data
        /// \param inSize size of packed data
        /// \param out buffer of outSize bytes where unpacked data will
        /// be placed
        /// \param outSize size of unpacked data
        void unpack(char *in, int inSize, char *out, int outSize);
};


/// Simple resource file wrapper.
/// Used at boot time when ResourcesCollection
/// is not available yet.
class SimpleResourceFile: public ResourceFile
{
    private:
        typedef std::map<std::wstring, DirectoryEntry> DirectoryMap;
        DirectoryMap directory;  /// Directory map.
        
    public:
        /// Open resource file.  Throws exception if file can't be opened.
        /// \param fileName the name of resource file.
        /// \param buffer buffer for temporary data.
        /// Can be shared with other resource files.
        SimpleResourceFile(const std::wstring &fileName, Buffer *buffer=NULL);

    public:
        /// Load data.  Memory returned by this method should be freed
        /// by free() function call.
        /// \param name name of resource
        /// \param size returns size of resource
        virtual void* load(const std::wstring &name, int &size);

        /// Load data into the buffer.
        /// \param name name of resource
        /// \param buffer buffer for resource data
        virtual void load(const std::wstring &name, Buffer &buffer);
};


/// Internationalized resource entity.
class ResVariant 
{
    private:
        int i18nScore;
        ResourceFile *file;
        long offset;
        long unpackedSize;
        long packedSize;
        int refCnt;
        void *data;
        int level;
        
    public:
        /// Create resource variation.
        /// \param file reesource file
        /// \param score locale compability score
        /// \param entry entry in global resources directory
        ResVariant(ResourceFile *file, int score,
                const ResourceFile::DirectoryEntry &entry);
        
        ~ResVariant();

    public:
        /// Return locale compability score.
        int getI18nScore() const { return i18nScore; };

        /// Get pointer to unpacked resource data.  
        /// Must be freed after use this delRef()
        void* getRef();

        /// Get pointer to unpacked resource data and return resource size.  
        /// Must be freed after use this delRef().
        /// \param size returned size of resource data.
        void* getRef(size_t &size);

        /// Delete referene to data.
        /// \param data pointer to unpacked data.
        void delRef(void *data);

        /// Return reference counter.
        int getRefCount() const { return refCnt; };

        /// Is data managed by this object
        /// \param data pointer to dataa
        bool isDataOwned(void *data) const { return refCnt && data == this->data; };

        /// return data.
        /// destroy it after use with free()
        void* getDynData();

        /// returns size of data
        long getSize() const { return unpackedSize; };

        /// Return data in buffer
        /// \param buffer buffer to store data.
        void getData(Buffer &buffer);
        
        /// Create ResourceStream for resource.
        /// This may be usefull for large streams unpacked data,
        /// for example video and sound.
        /// Delete stream after use.
        ResourceStream* createStream();
};


/// Internationalized resources.
/// Collection of localized data (ResVariant) with single name.
class Resource 
{
    private:
        typedef std::vector<ResVariant*> Variants;
        Variants variants;
        std::wstring name;

    public:
        /// Create resource and add first entry
        /// \param file resource file
        /// \param i18nScore locale compability score
        /// \param entry resource entry in global directory
        /// \param name name of resource
        Resource(ResourceFile *file, int i18nScore,
                const ResourceFile::DirectoryEntry &entry,
                const std::wstring &name);

        ~Resource();

    public:
        /// Add resource variant.
        /// \param file resource file
        /// \param i18nScore locale compability score
        /// \param entry resource entry in global directory
        void addVariant(ResourceFile *file, int i18nScore,
                const ResourceFile::DirectoryEntry &entry);

        /// Get number of variants.
        int getVariantsCount() const { return variants.size(); };

        /// Geturns variant object.
        /// \param variant variant number.
        ResVariant* getVariant(int variant=0) { return variants[variant]; };

        /// Load data from variant.
        /// Data must be freed with delRef().
        /// \param variant variant number.
        void* getRef(int variant=0);

        /// Load data from variant.  
        /// data must be freed with delRef()
        /// \param size size of data.
        /// \param variant variant number.
        void* getRef(int *size, int variant=0);

        /// Unload data
        /// param data pointer to data.
        void delRef(void *data);

        /// Get size of data.
        /// \param variant variant number.
        long getSize(int variant=0) { return variants[variant]->getSize(); };

        /// Get name of this resource.
        const std::wstring& getName() const { return name; };
        
        /// Load data into buffer.
        /// \param buffer buffer for data.
        /// \param variant variant number.
        void getData(Buffer &buffer, int variant=0);
        
        /// Create ResourceStream for resource.
        /// This may be usefull for large streams unpacked data,
        /// for example video and sound.
        /// Delete stream after use.
        /// \param variant variant number.
        ResourceStream* createStream(int variant=0);
};


/// Internationalized resource files collection.
/// When ResourceCollection created all resources checked against
/// current locale.  Resources not belonged to current locale are 
/// ignored, resources that can be used in current locale sorted
/// by locale relevance score.
/// All resources names interpeted as name[_language][_COUNTRY].extension
/// where name is name of resource, language is optional two letters
/// ISO language code, country is two letters ISO country code
/// and extension is file extension.
/// After processing resource name will be truncated to name.extension.
/// By default resource with highest locale relevance score will
/// be loaded. Howether, it is possible to enumerate all variants
/// by using Resource class interface.
/// Resource files loaded in order specified by their priority.
/// Resources from file loaded later can replace resources
/// loaded before it.
class ResourcesCollection
{
    private:
        /// Map resource names to resources.
        typedef std::map<std::wstring, Resource*> ResourcesMap;
        
        /// List of resources.
        typedef std::list<Resource*> ResourcesList;
        
        /// Map group names to resources list.
        typedef std::map<std::wstring, ResourcesList> ResourcesListMap;
        
        /// List of resource files.
        typedef std::vector<ResourceFile*> ResourceFiles;
        
        ResourcesMap resources;    /// Map of all available resources.
        ResourcesListMap groups;   /// Map of all available groups.
        ResourceFiles files;       /// List of resource files.
        Buffer buffer;             /// Temporary buffer for resource files.
        
    public:
        /// Load resource files, make grouping and i18n optimizations.
        ResourcesCollection(StringList &directories);
        ~ResourcesCollection();

    public:
        /// Returns resource entry.  
        /// If resource not found Exception will be thrown.
        Resource* getResource(const std::wstring &name);
 
        /// Load resource. 
        /// Loaded data must be freed with delRef method.
        void* getRef(const std::wstring &name, int &size);
        
        /// Load resource. 
        /// Loaded data must be freed with delRef method.
        void* getRef(const std::wstring &name);

        /// Delete reference to resource.
        void delRef(void *data);
  
        /// Visit all group members.
        void forEachInGroup(const std::wstring &groupName, 
                Visitor<Resource*> &visitor);
        
        /// Create ResourceStream for resource.
        /// This may be usefull for large streams unpacked data,
        /// for example video and sound.
        /// Delete stream after use.
        /// \param name name of resource.
        ResourceStream* createStream(const std::wstring &name);

        /// Load data into buffer.
        /// Usually you don't need this, use getRef instead.
        /// \param name name of resource.
        /// \param buffer buffer for data.
        void loadData(const std::wstring &name, Buffer &buffer);

    private:
        /// Open resource files.
        void loadResourceFiles(StringList &directories);

        /// Make grouping and locale processing.
        void processFiles();
};


/// Helper class for simple resource loading and unloading
class ResDataHolder
{
    private:
        void *data;
        size_t size;
        
    public:
        /// Create holder without data
        ResDataHolder();

        /// Create holder and load data
        ResDataHolder(const std::wstring &name);

        ~ResDataHolder();

    public:
        /// Load resource data
        void load(const std::wstring &name);

        /// Returns pointer to resource data
        void* getData() const { return data; };

        /// Returns size of data
        size_t getSize() const { return size; };
};


/// Global collection of resources. 
/// Careated at first step at boot time stage2
extern ResourcesCollection *resources;


#endif

