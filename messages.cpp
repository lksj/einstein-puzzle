// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

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


#include "messages.h"

#include "buffer.h"
#include "exceptions.h"
#include "formatter.h"
#include "resources.h"
#include "unicode.h"
#include "utils.h"

#include <cstdarg>


Messages msg;


Messages::ScoredStr::~ScoredStr()
{
    delete message;
}


Messages::ScoredStr::ScoredStr(int score, Formatter* message)
    : score(score), message(message)
{
}

Messages::Messages() = default;

Messages::~Messages()
{
    for (auto kv : messages)
    {
        delete kv.second;
    }
}

class ResVisitor: public Visitor<Resource*>
{
    private:
        Messages &messages;
        Buffer *buffer;
    
    public:
        ResVisitor(Messages &m, Buffer *b): messages(m) { buffer = b; }
        
        void onVisit(Resource *&r) override {
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

    const int cnt = res->getVariantsCount();
    for (int i = 0; i < cnt; i++) {
        ResVariant *var = res->getVariant(i);
        if (var) {
            try {
                const int score = var->getI18nScore();
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
    const StrMap::const_iterator i = messages.find(key);
    if (i != messages.end())
        return (*i).second->message->getMessage();
    else
        return key;
}

std::wstring Messages::format(const wchar_t *key, va_list ap) const
{
    std::wstring s;
    const StrMap::const_iterator i = messages.find(key);
    if (i != messages.end())
        s = (*i).second->message->format(ap);
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
    const int cnt = readInt(data + offset);
    offset += 4;

    for (int i = 0; i < cnt; i++) {
        const int sz = readInt(data + offset);
        offset += 4;
        if (sz > 0) {
            const std::wstring name(fromUtf8((char*)data + offset, sz));
            const int msgOffset = readInt(data + offset + sz);
            StrMap::iterator i = messages.find(name);
            if (i == messages.end()) {
                ScoredStr* ss = new ScoredStr(score, new Formatter(data, msgOffset));
                messages[name] = ss;
            } else {
                ScoredStr* ss = (*i).second;
                if (ss->score <= score) {
                    ss->score = score;
                    delete ss->message;
                    ss->message = new Formatter(data, msgOffset);
                }
            }
        }
        offset += sz + 4;
    }
}

