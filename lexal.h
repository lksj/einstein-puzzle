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


#ifndef __LEXAL_H__
#define __LEXAL_H__


#include "streams.h"


class Lexeme
{
    public:
        typedef enum Type {
            String,
            Integer,
            Float,
            Ident,
            Symbol,
            Eof
        };

    private:
        int line;
        int pos;
        Type type;
        std::wstring content;
    
    public:
        Lexeme() { }
        Lexeme(Type type, const std::wstring &content, int line, int pos);
        ~Lexeme() = default;

    public:
        Type getType() const { return type; }
        const std::wstring getContent() const { return content; }
        std::wstring getPosStr() const;
        int getLine() const { return line; }
        int getPos() const { return pos; }
};


class Lexal
{
    private:
        UtfStreamReader &reader;
        int line;
        int pos;
        
    public:
        explicit Lexal(UtfStreamReader &reader);
        ~Lexal() = default;

    public:
        Lexeme getNext();
        static std::wstring posToStr(int line, int pos);

    private:
        void skipSpaces();
        void skipToLineEnd();
        void skipMultilineComment(int startLine, int startPos);
        Lexeme readIdent(int startLine, int startPos, wchar_t first);
        Lexeme readNumber(int startLine, int startPos, wchar_t first);
        Lexeme readString(int startLine, int startPos, wchar_t quote);
};


#endif

