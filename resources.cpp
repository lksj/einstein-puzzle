// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-08-04 by Jordan Evens <jordan.evens@gmail.com>

// Einstein Puzzle is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// Einstein Puzzle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


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
        explicit MemoryResourceStream(ResVariant *resource);
        virtual ~MemoryResourceStream();

    public:
        virtual size_t getSize() { return size; };
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
        throw;
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
        throw;
    } catch (...) {
        if (outBuf) free(outBuf);
        throw Exception(name + L": Error loading resource");
    }

    return outBuf;
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
    for (auto& variant : variants)
        delete variant;
}

class ScorePredicate
{
    public:
        int score;
        
        explicit ScorePredicate(int sc) { score = sc; }

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
    if (variants.empty()) {
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
    for (auto& resource : resources)
        delete resource.second;
    for (auto& file : files)
        delete file;
}


void ResourcesCollection::loadResourceFiles(StringList &directories)
{
    for (auto& d : directories)
    {
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
    for (auto file : files)
    {
        file->getDirectory(dir);
        for (auto& de : dir)
        {
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
        for (auto r : l)
        {
            visitor.onVisit(r);
        }
    }
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

