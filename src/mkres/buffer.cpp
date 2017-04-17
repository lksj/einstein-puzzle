#include "buffer.h"
#include <string.h>
#include "exceptions.h"
#include "unicode.h"


Buffer::Buffer(int sz, int alloc)
{
    allocated = alloc;
    size = sz;
    if (size > allocated)
        allocated = size;
    if (allocated < 1024)
        allocated = 1024;
    data = malloc(allocated);
    if (! data)
        throw Exception(L"Error allocating memory for Buffer");
}

Buffer::~Buffer()
{
    free(data);
}

void Buffer::setSize(size_t sz)
{
    if (sz > allocated) {
        int newAl = allocated + sz + 1024;
        void *d = realloc(data, newAl);
        if (! d)
            throw Exception(L"Error expanding buffer memory");
        data = d;
        allocated = newAl;
    }
    
    size = sz;
}

size_t Buffer::getSize()
{
    return size;
}

size_t Buffer::getAllocated()
{
    return allocated;
}

void* Buffer::getData()
{
    return data;
}


void Buffer::gotoPos(int offset)
{
    currentPos = offset;
}


size_t Buffer::putData(const unsigned char *d, size_t length)
{
    if (size < currentPos + length)
        setSize(currentPos + length);
    memcpy((unsigned char*)data + currentPos, d, length);
    currentPos += length;
    return length;
}


size_t Buffer::putInteger(int v)
{
    unsigned char b[4];
    int i, ib;

    for (i = 0; i < 4; i++) {
        ib = v & 0xFF;
        v = v >> 8;
        b[i] = ib;
    }

    return putData(b, 4);
}


size_t Buffer::putUtf8(const std::wstring &string)
{
    std::string s(toUtf8(string));
    putInteger(s.length());
    putData((const unsigned char*)s.c_str(), s.length());
    return 4 + s.length();
}


size_t Buffer::putByte(unsigned char value)
{
    if (size < (size_t)currentPos + 1)
        setSize(currentPos + 1);
    ((unsigned char*)data)[currentPos] = value;
    currentPos++;
    return 1;
}


