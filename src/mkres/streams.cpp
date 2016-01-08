#include "streams.h"
#include "exceptions.h"
#include "unicode.h"


UtfStreamReader::UtfStreamReader(std::ifstream *s)
{
    stream = s;
}

UtfStreamReader::~UtfStreamReader()
{
}

// This function is very slow because of poor fromUtf8 function design
wchar_t UtfStreamReader::getNextChar()
{
    unsigned char buf[10];

    if (0 < backBuf.size()) {
        wchar_t wc = backBuf.front();
        backBuf.pop_front();
        return wc;
    }
    
    if (! stream->good())
        throw Exception(L"Error reading from stream 1");

    int sz = stream->readsome((char*)buf, 1);
    if (1 != sz)
        throw Exception(L"Error reading from stream 2");
    int size = getUtf8Length(buf[0]);
    if (size > 1) {
        sz = stream->readsome((char*)buf + 1, size - 1);
        if (size - 1 != sz)
            throw Exception(L"Error reading from stream 3");
    }
    buf[size] = 0;
    std::string s((char*)buf);
    std::wstring ws(fromUtf8(s));
    if (1 != ws.length())
        throw Exception(L"Error converting UTF-8 character to wide character");
    return ws[0];
}

void UtfStreamReader::ungetChar(wchar_t ch)
{
    backBuf.push_back(ch);
}

bool UtfStreamReader::isEof()
{
    if (stream->eof())
        return true;     // FIXME: it doesn't work. why?
    return EOF == stream->peek();
}

