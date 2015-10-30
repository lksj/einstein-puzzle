#include <stdarg.h>

#include "messages.h"
#include "formatter.h"
#include "resources.h"
#include "exceptions.h"
#include "buffer.h"
#include "utils.h"
#include "unicode.h"


Messages msg;


Messages::Messages()
{
}

Messages::~Messages()
{
}

class ResVisitor: public Visitor<Resource*>
{
    private:
        Messages &messages;
        Buffer *buffer;
    
    public:
        ResVisitor(Messages &m, Buffer *b): messages(m) { buffer = b; };
        
        virtual void onVisit(Resource *&r) {
            messages.loadFromResource(r, buffer);
        }
};

void Messages::load()
{
    Buffer buffer;
    ResVisitor loader(*this, &buffer);
    resources->forEachInGroup(L"messages", loader);
}

void Messages::loadFromResource(Resource *res, Buffer *buffer)
{
    if (! res) return;

    int cnt = res->getVariantsCount();
    for (int i = 0; i < cnt; i++) {
        ResVariant *var = res->getVariant(i);
        if (var) {
            try {
                int score = var->getI18nScore();
                var->getData(*buffer);
                loadBundle(score, (unsigned char*)buffer->getData(), 
                        buffer->getSize());
            } catch (Exception &e) {
                std::cerr << std::wstring(L"Error loading text bundle " +
                        res->getName() + L": " + e.getMessage());
            }
        }
    }
}

std::wstring Messages::getMessage(const std::wstring &key) const
{
    StrMap::const_iterator i = messages.find(key);
    if (i != messages.end())
        return (*i).second.message->getMessage();
    else
        return key;
}

std::wstring Messages::format(const wchar_t *key, va_list ap) const
{
    std::wstring s;
    StrMap::const_iterator i = messages.find(key);
    if (i != messages.end())
        s = (*i).second.message->format(ap);
    else
        s = key;
    return s;
}

std::wstring Messages::format(const wchar_t *key, ...) const
{
    va_list ap;
    va_start(ap, key);
    std::wstring s = format(key, ap);
    va_end(ap);
    return s;
}

std::wstring Messages::operator ()(const wchar_t *key, ...) const
{
    va_list ap;
    va_start(ap, key);
    std::wstring s = format(key, ap);
    va_end(ap);
    return s;
}

void Messages::loadBundle(int score, unsigned char *data, size_t size)
{
    if ((data[0] != 'C') || (data[1] != 'M') || (data[2] != 'F'))
        throw Exception(L"Invalid format of message file");
    if (readInt(data + 3) != 1)
        throw Exception(L"Unknown version of message file");

    int offset = readInt(data + size - 4);
    int cnt = readInt(data + offset);
    offset += 4;

    for (int i = 0; i < cnt; i++) {
        int sz = readInt(data + offset);
        offset += 4;
        if (sz > 0) {
            std::wstring name(fromUtf8((char*)data + offset, sz));
            int msgOffset = readInt(data + offset + sz);
            StrMap::iterator i = messages.find(name);
            if (i == messages.end()) {
                ScoredStr ss = { score, new Formatter(data, msgOffset) };
                messages[name] = ss;
            } else {
                ScoredStr &ss = (*i).second;
                if (ss.score <= score) {
                    ss.score = score;
                    ss.message = new Formatter(data, msgOffset);
                }
            }
        }
        offset += sz + 4;
    }
}

