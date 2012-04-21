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


#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__


#include <string>
#include <list>


class Tokenizer;


class Token
{
    friend class Tokenizer;

    public:
        enum Type {
            Word,
            Para,
            Eof
        };

    private:
        Type type;
        std::wstring content;

    private:
        Token(Type type) { this->type = type; };
        Token(Type type, const std::wstring &content): content(content) {
            this->type = type;
        }

    public:
        Type getType() const { return type; };
        const std::wstring& getContent() const { return content; };
        std::wstring toString() const;
};


class Tokenizer
{
    private:
        std::wstring text;
        int currentPos;
        std::list<Token> stack;

    public:
        Tokenizer(const std::wstring &s): text(s) { currentPos = 0; };

    public:
        Token getNextToken();
        void unget(const Token &token);
        bool isFinished();

    private:
        bool skipSpaces(bool notSearch);
};


#endif

