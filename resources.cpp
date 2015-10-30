#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <zlib.h>

#include "resources.h"
#include "exceptions.h"
#include "unicode.h"
#include "convert.h"
#include "i18n.h"
#include "utils.h"


ResourcesCollection *resources = NULL;


///////////////////////////////////////////////////////////////////
//
// UnpackedResourceStream
//
///////////////////////////////////////////////////////////////////


class UnpackedResourceStream: public ResourceStream
{
    private:
        std::ifstream &stream;
        size_t size;
        long offset;
        long pos;
    
    public:
        UnpackedResourceStream(std::ifstream &stream, long offset, 
                size_t size);

    public:
        virtual size_t getSize() { return size; };
        virtual void seek(long offset);
        virtual void read(char *buffer, size_t size);
        virtual long getPos() { return pos; };
};

UnpackedResourceStream::UnpackedResourceStream(std::ifstream &s, 
        long off, size_t sz): stream(s)
{
    offset = off;
    size = sz;
    pos = 0;
}

void UnpackedResourceStream::seek(long off)
{
    if ((off < 0) || ((size_t)off > size))
        throw Exception(L"Invalid seek in ResourceStream");
    pos = off;
}

void UnpackedResourceStream::read(char *buffer, size_t sz)
{
    if (! buffer)
        throw Exception(L"Invalid buffer in ResourceStream");
    if (sz + pos > size)
        throw Exception(L"Attempt of reading after resource end");
    stream.seekg(offset + pos, std::ios::beg);
    stream.read(buffer, sz);
    pos += sz;
}


///////////////////////////////////////////////////////////////////
//
// MemoryResourceStream
//
///////////////////////////////////////////////////////////////////


class MemoryResourceStream: public ResourceStream
{
    private:
        char *data;
        size_t size;
        ResVariant *resource;
        long pos;
    
    public:
        MemoryResourceStream(ResVariant *resource);
        virtual ~MemoryResourceStream();

    public:
        virtual size_t getSize() { return size; };
        virtual void seek(long offset);
        virtual void read(char *buffer, size_t size);
        virtual long getPos() { return pos; };
};

MemoryResourceStream::MemoryResourceStream(ResVariant *res)
{
    resource = res;
    data = (char*)res->getRef(size);
    pos = 0;
}

MemoryResourceStream::~MemoryResourceStream()
{
    resource->delRef(data);
}

void MemoryResourceStream::seek(long off)
{
    if ((off < 0) || ((size_t)off > size))
        throw Exception(L"Invalid seek in ResourceStream");
    pos = off;
}

void MemoryResourceStream::read(char *buffer, size_t sz)
{
    if (! buffer)
        throw Exception(L"Invalid buffer in ResourceStream");
    if (sz + pos > size)
        throw Exception(L"Attempt of reading after resource end");
    memcpy(buffer, data, sz);
    pos += sz;
}


///////////////////////////////////////////////////////////////////
//
// ResourceFile
//
///////////////////////////////////////////////////////////////////


ResourceFile::ResourceFile(const std::wstring &fileName, Buffer *buf):
        name(fileName)
{
    if (buf) {
        buffer = buf;
        ownBuffer = false;
    } else {
        buffer = new Buffer();
        ownBuffer = true;
    }
    
    stream.open(toMbcs(fileName).c_str(), std::ios::in | std::ios::binary);
    if (stream.fail())
        throw Exception(L"Error loading resource file '" + name + L"'");
    
    char sign[4];
    stream.read(sign, 4);
    int readed = stream.gcount();
    if (stream.fail() || (readed != 4) || (sign[0] != 'C') || 
            (sign[1] != 'R') || (sign[2] != 'F') || sign[3])
        throw Exception(L"Invalid resource file '" + name + L"'");

    int major = readInt(stream);
    readed = stream.gcount();
    int minor = readInt(stream);
    readed += stream.gcount();
    priority = readInt(stream);
    readed += stream.gcount();
    if (stream.fail() || (readed != 12) || (major != 2) || (minor < 0))
        throw Exception(L"Incompatible version of resource file '" + 
                name + L"'");
}


ResourceFile::~ResourceFile()
{
    stream.close();
    if (ownBuffer)
        delete buffer;
}


void ResourceFile::getDirectory(Directory &directory)
{
    stream.seekg(-8, std::ios::end);
    if (stream.fail())
        throw Exception(L"Error reading " + name + L" directory");
    int start = readInt(stream);
    int count = readInt(stream);
    stream.seekg(start, std::ios::beg);
    if (stream.fail())
        throw Exception(L"Error reading " + name + L" directory");

    for (int i = 0; i < count; i++) {
        DirectoryEntry entry;
        entry.name = readString(stream);
        entry.unpackedSize = readInt(stream);
        entry.offset = readInt(stream);
        entry.packedSize = readInt(stream);
        entry.level = readInt(stream);
        entry.group = readString(stream);
        directory.push_back(entry);
    }
    
    if (stream.fail())
        throw Exception(L"Error reading " + name + L" directory");
}


void ResourceFile::unpack(char *in, int inSize, char *out, int outSize)
{
    z_stream zs;
    
    memset(&zs, 0, sizeof(z_stream));
    zs.next_in = (Bytef*)in;
    zs.avail_in = inSize;
    zs.next_out = (Bytef*)out;
    zs.avail_out = outSize;
    
    if (inflateInit(&zs) != Z_OK) 
        throw Exception(name + L": Error initializing inflate stream.");
    
    if (inflate(&zs, Z_FINISH) != Z_STREAM_END)
        throw Exception(name + L": Error decompresing element.");
    
    if (inflateEnd(&zs) != Z_OK)
        throw Exception(name + L": Error finishing decompresing.");
}


void ResourceFile::load(char *buf, long offset, long packedSize, 
        long unpackedSize, int level)
{
    char *inBuf=NULL;
    
    try {
        if (! level) {
            stream.seekg(offset, std::ios::beg);
            stream.read(buf, unpackedSize);
            return;
        }

        buffer->setSize(packedSize);
        stream.seekg(offset, std::ios::beg);
        stream.read((char*)buffer->getData(), packedSize);
        unpack((char*)buffer->getData(), packedSize, buf, unpackedSize);
    } catch (Exception &e) {
        if (inBuf) free(inBuf);
        throw e;
    } catch (...) {
        if (inBuf) free(inBuf);
        throw Exception(name + L": Error loading resource");
    }
}


void* ResourceFile::load(long offset, long packedSize, long unpackedSize,
        int level)
{
    char *outBuf=NULL;
    
    try {
        outBuf = (char*)malloc(unpackedSize);
        if (! outBuf)
            throw Exception(name + L": Error allocating memory");
        load(outBuf, offset, packedSize, unpackedSize, level);
    } catch (Exception &e) {
        if (outBuf) free(outBuf);
        throw e;
    } catch (...) {
        if (outBuf) free(outBuf);
        throw Exception(name + L": Error loading resource");
    }

    return outBuf;
}


///////////////////////////////////////////////////////////////////
//
// SimpleResourceFile
//
///////////////////////////////////////////////////////////////////


SimpleResourceFile::SimpleResourceFile(const std::wstring &fileName,
        Buffer *buf): ResourceFile(fileName, buf)
{
    Directory entries;    
    getDirectory(entries);
    for (Directory::iterator i = entries.begin(); i != entries.end(); i++) {
        DirectoryEntry &e = *i;
        directory[e.name] = e;
    }
}

void* SimpleResourceFile::load(const std::wstring &name, int &size)
{
    DirectoryMap::iterator i = directory.find(name);
    if (i != directory.end()) {
        DirectoryEntry &e = (*i).second;
        size = e.unpackedSize;
        return ResourceFile::load(e.offset, e.packedSize, e.unpackedSize,
                e.level);
    } else
        throw Exception(L"Resource '" + name + L"' not found");
}

void SimpleResourceFile::load(const std::wstring &name, Buffer &outBuf)
{
    DirectoryMap::iterator i = directory.find(name);
    if (i != directory.end()) {
        DirectoryEntry &e = (*i).second;
        outBuf.setSize(e.unpackedSize);
        ResourceFile::load((char*)outBuf.getData(), e.offset, 
                e.packedSize, e.unpackedSize, e.level);
    } else
        throw Exception(L"Resource '" + name + L"' not found");
}

///////////////////////////////////////////////////////////////////
//
// ResVariant
//
///////////////////////////////////////////////////////////////////


ResVariant::ResVariant(ResourceFile *f, int score,
        const ResourceFile::DirectoryEntry &e)
{
    file = f;
    i18nScore = score;
    offset = e.offset;
    unpackedSize = e.unpackedSize;
    packedSize = e.packedSize;
    level = e.level;
    refCnt = 0;
    data = NULL;
}


ResVariant::~ResVariant()
{
    if (data)
        free((char*)data - sizeof(ResVariant*));
}

void* ResVariant::getRef()
{
    if (! refCnt) {
        char* d = (char*)malloc(unpackedSize + sizeof(void*));
        if (! d)
            throw Exception(L"ResVariant::getRef memory allocation error");
        ResVariant *self = this;
        file->load(d + sizeof(self), offset, packedSize, unpackedSize,
                level);
        memcpy(d, &self, sizeof(self));
        data = d + sizeof(self);
    }
        
    refCnt++;
    return data;
}

void* ResVariant::getRef(size_t &sz)
{
    sz = unpackedSize;
    return getRef();
}

void ResVariant::delRef(void *dta)
{
    if ((! refCnt) || (dta != data))
        throw Exception(L"Invalid ResVariant::delRef call");

    refCnt--;
    if (! refCnt) {
        free((char*)data - sizeof(ResVariant*));
        data = NULL;
    }
}

void* ResVariant::getDynData()
{
    if (! refCnt)
        return file->load(offset, packedSize, unpackedSize, level);
    else {
        char* d = (char*)malloc(unpackedSize);
        if (! d)
            throw Exception(L"ResVariant::getDynData memory allocation error");
        memcpy(d, data, unpackedSize);
        return data;
    }
}

void ResVariant::getData(Buffer &buffer)
{
    buffer.setSize(unpackedSize);
    if (! refCnt)
        file->load((char*)buffer.getData(), offset, packedSize, 
                unpackedSize, level);
    else
        memcpy((char*)buffer.getData(), data, unpackedSize);
}

ResourceStream* ResVariant::createStream()
{
    if (refCnt || level)
        return new MemoryResourceStream(this);
    else
        return new UnpackedResourceStream(file->getStream(), offset, 
                packedSize);
}


///////////////////////////////////////////////////////////////////
//
// Resource
//
///////////////////////////////////////////////////////////////////


Resource::Resource(ResourceFile *file, int i18nScore,
        const ResourceFile::DirectoryEntry &entry, const std::wstring &n):
                name(n)
{
    addVariant(file, i18nScore, entry);
}

Resource::~Resource()
{
    for (Variants::iterator i = variants.begin(); i != variants.end(); i++)
        delete *i;
}

class ScorePredicate
{
    public:
        int score;
        
        ScorePredicate(int sc) { score = sc; }

        bool operator() (const ResVariant *r) const {
            return r->getI18nScore() == score;
        };
};

class ResVariantMoreThen
{
    public:
        bool operator() (const ResVariant *v1, const ResVariant *v2) const {
            return v1->getI18nScore() > v2->getI18nScore();
        };
};

void Resource::addVariant(ResourceFile *file, int i18nScore,
        const ResourceFile::DirectoryEntry &entry)
{
    if (! variants.size()) {
        variants.push_back(new ResVariant(file, i18nScore, entry));
        return;
    }
    
    ScorePredicate p(i18nScore);
    Variants::iterator i = std::find_if(variants.begin(), variants.end(), p);
    if (i != variants.end()) {
        delete *i;
        *i = new ResVariant(file, i18nScore, entry);
    } else {
        variants.push_back(new ResVariant(file, i18nScore, entry));
        ResVariantMoreThen comparator;
        std::sort(variants.begin(), variants.end(), comparator);
    }
}


void* Resource::getRef(int variant)
{
    return variants[variant]->getRef();
}


void* Resource::getRef(int *size, int variant)
{
    if (size)
        *size = variants[variant]->getSize();
    return variants[variant]->getRef();
}


void Resource::delRef(void *data)
{
    ResVariant *v = *(ResVariant**)((char*)data - sizeof(ResVariant*));
    v->delRef(data);
}


void Resource::getData(Buffer &buffer, int variant)
{
    variants[variant]->getData(buffer);
}

ResourceStream* Resource::createStream(int variant)
{
    return variants[variant]->createStream();
}


///////////////////////////////////////////////////////////////////
//
// ResourcesCollection
//
///////////////////////////////////////////////////////////////////


class ResFileMoreThen
{
    public:
        bool operator() (const ResourceFile *f1, const ResourceFile *f2) const {
            return f1->getPriority() > f2->getPriority();
        };
};

ResourcesCollection::ResourcesCollection(StringList &directories)
{
    loadResourceFiles(directories);
    ResFileMoreThen comparator;
    std::sort(files.begin(), files.end(), comparator);
    processFiles();
}


ResourcesCollection::~ResourcesCollection()
{
    for (ResourcesMap::iterator i = resources.begin(); i != resources.end(); i++)
        delete (*i).second;
    for (ResourceFiles::iterator i = files.begin(); i != files.end(); i++)
        delete *i;
}


void ResourcesCollection::loadResourceFiles(StringList &directories)
{
    for (StringList::iterator i = directories.begin();
            i != directories.end(); i++)
    {
        const std::wstring &d = *i;
        DIR *dir = opendir(toMbcs(d).c_str());
        if (dir) {
            struct dirent *de;
            while ((de = readdir(dir)))
                if (de->d_name[0] != '.') {
                    std::wstring s(fromMbcs(de->d_name));
                    if ((s.length() > 4) && 
                            (toLowerCase(s.substr(s.length() - 4)) == L".res"))
                        files.push_back(new ResourceFile(d + L"/" + s, &buffer));
                }
            closedir(dir);
        }
    }
}


void ResourcesCollection::processFiles()
{
    ResourceFile::Directory dir;
    for (std::vector<ResourceFile*>::iterator i = files.begin(); 
            i != files.end(); i++) 
    {
        ResourceFile *file = *i;
        file->getDirectory(dir);
        for (ResourceFile::Directory::iterator j = dir.begin(); 
                j != dir.end(); j++) 
        {
            ResourceFile::DirectoryEntry &de = *j;
            std::wstring name, ext, language, country;
            splitFileName(de.name, name, ext, language, country);
            int score = getScore(language, country, locale);
            if (score > 0) {
                std::wstring resName = name + L"." + ext;
                Resource *res = resources[resName];
                if (! res) {
                    res = new Resource(file, score, de, resName);
                    resources[resName] = res;
                    if (de.group.length())
                        groups[de.group].push_back(res);
                } else
                    res->addVariant(file, score, de);
            }
        }
        dir.clear();
    }
}


Resource* ResourcesCollection::getResource(const std::wstring &name)
{
    Resource *r = resources[name];
    if (! r)
        throw Exception(L"Resource '" + name + L"' not found");
    return r;
}


void* ResourcesCollection::getRef(const std::wstring &name, int &size)
{
    Resource *r = getResource(name);
    ResVariant *v = r->getVariant(0);
    size = v->getSize();
    return v->getRef();
}


void* ResourcesCollection::getRef(const std::wstring &name)
{
    Resource *r = getResource(name);
    ResVariant *v = r->getVariant(0);
    return v->getRef();
}

ResourceStream* ResourcesCollection::createStream(const std::wstring &name)
{
    Resource *r = getResource(name);
    return r->createStream();
}

void ResourcesCollection::delRef(void *data)
{
    ResVariant *v = *(ResVariant**)((char*)data - sizeof(ResVariant*));
    v->delRef(data);
}

void ResourcesCollection::forEachInGroup(const std::wstring &name, 
        Visitor<Resource*> &visitor)
{
    if (groups.count(name) > 0) {
        ResourcesList &l = groups[name];
        for (ResourcesList::iterator i = l.begin(); i != l.end(); i++) {
            Resource *r = *i;
            visitor.onVisit(r);
        }
    }
}

void ResourcesCollection::loadData(const std::wstring &name, Buffer &buffer)
{
    Resource *r = getResource(name);
    r->getData(buffer);
}


///////////////////////////////////////////////////////////////////
//
// ResDataHolder
//
///////////////////////////////////////////////////////////////////


ResDataHolder::ResDataHolder()
{
    data = NULL;
    size = 0;
}

ResDataHolder::ResDataHolder(const std::wstring &name)
{
    load(name);
}

ResDataHolder::~ResDataHolder()
{
    if (data)
        resources->delRef(data);
}

void ResDataHolder::load(const std::wstring &name)
{
    int s;
    data = resources->getRef(name, s);
    size = (size_t)s;
}

