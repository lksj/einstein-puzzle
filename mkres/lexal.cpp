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


#include "lexal.h"
#include "convert.h"

        
Lexeme::Lexeme(Type t, const std::wstring &cont, int line, int pos)
{
    type = t;
    content = cont;
    this->line = line;
    this->pos = pos;
}

std::wstring Lexeme::getPosStr() const
{
    return Lexal::posToStr(line, pos);
}



Lexal::Lexal(UtfStreamReader &rdr): reader(rdr)
{
    line = 1;
    pos = 0;
}


static bool isIdentStart(wchar_t ch) {
    return ((L'a' <= ch) && (L'z' >= ch)) || ((L'A' <= ch) && (L'Z' >= ch))
        || (L'_' == ch);
}

static bool isIdentCont(wchar_t ch) {
    return ((L'a' <= ch) && (L'z' >= ch)) || ((L'A' <= ch) && (L'Z' >= ch))
        || (L'_' == ch) || (L'.' == ch) || ((L'0' <= ch) && (L'9' >= ch));
}

static bool isWhiteSpace(wchar_t ch) {
    return (L' ' == ch) || (L'\t' == ch) || (L'\n' == ch) || (L'\r' == ch);
}

static bool isDigit(wchar_t ch) {
    return (L'0' <= ch) && (L'9' >= ch);
}

static bool isSymbol(wchar_t ch) {
    return (L'{' == ch) || (L'}' == ch) || (L',' == ch) || (L'=' == ch)
         || (L';' == ch);
}

static bool isQuote(wchar_t ch) {
    return (L'\'' == ch) || (L'"' == ch);
}


std::wstring Lexal::posToStr(int line, int pos)
{
    return L"(" + toString(line) + L":" + toString(pos) + L")";
}


Lexeme Lexal::getNext()
{
    skipSpaces();
    if (reader.isEof())
        return Lexeme(Lexeme::Eof, L"", line, pos);

    int startLine = line;
    int startPos = pos;

    wchar_t ch = reader.getNextChar();
    pos++;

    if (isIdentStart(ch))
        return readIdent(startLine, startPos, ch);
    else if (isDigit(ch))
        return readNumber(startLine, startPos, ch);
    else if (isQuote(ch))
        return readString(startLine, startPos, ch);
    else if (isSymbol(ch))
        return Lexeme(Lexeme::Symbol, toString(ch), startLine, startPos);
    
    throw Exception(L"Invalid character at "+ posToStr(startLine, startPos));
}


Lexeme Lexal::readString(int startLine, int startPos, wchar_t quote)
{
    std::wstring str;
    bool closed = false;
    
    while (! reader.isEof()) {
        wchar_t ch = reader.getNextChar();
        pos++;
        if ('\n' == ch) {
            line++;
            pos = 0;
            str += ch;
        } else if ('\\' == ch) {
            if (! reader.isEof()) {
                wchar_t nextCh = reader.getNextChar();
                if (isWhiteSpace(nextCh))
                    throw Exception(L"Invalid escape sequence at " +
                            posToStr(line, pos));
                pos++;
                switch (nextCh) {
                    case L'\t': str += L'\t'; break;
                    case L'\n': str += L'\n'; break;
                    case L'\r': str += L'\r'; break;
                    default:
                        str += nextCh;
                }
            }
        } else if (quote == ch) {
            closed = true;
            break;
        } else
            str += ch;
    }

    if (! closed)
        throw Exception(L"String at " + posToStr(startLine, startPos)
                + L" doesn't closed");

    return Lexeme(Lexeme::String, str, startLine, startPos);
}

Lexeme Lexal::readNumber(int startLine, int startPos, wchar_t first)
{
    std::wstring number;
    number += first;
    Lexeme::Type type = Lexeme::Integer;
    
    while (! reader.isEof()) {
        wchar_t ch = reader.getNextChar();
        pos++;
        if (isDigit(ch))
            number += ch;
        else if (L'.' == ch) {
            if (Lexeme::Integer == type) {
                type = Lexeme::Float;
                number += ch;
            } else
                throw Exception(L"To many dots in number at " + 
                        posToStr(line, pos));
        } else if ((! isSymbol(ch)) && (! isWhiteSpace(ch)))
            throw Exception(L"invalid number at " + posToStr(line, pos));
        else {
            pos--;
            reader.ungetChar(ch);
            break;
        }
    }

    if (L'.' == number[number.length() - 1])
        throw Exception(L"Missing digit after dot at " + posToStr(line, pos));

    return Lexeme(type, number, startLine, startPos);
}

Lexeme Lexal::readIdent(int startLine, int startPos, wchar_t first)
{
    std::wstring ident;
    ident += first;
    
    while (! reader.isEof()) {
        wchar_t ch = reader.getNextChar();
        if (! isIdentCont(ch)) {
            reader.ungetChar(ch);
            break;
        }
        ident += ch;
        pos++;
    }

    return Lexeme(Lexeme::Ident, ident, startLine, startPos);
}


void Lexal::skipToLineEnd()
{
    while (! reader.isEof()) {
        wchar_t ch = reader.getNextChar();
        pos++;
        if ('\n' == ch) {
            pos = 0;
            line++;
            return;
        }
    }
}


void Lexal::skipMultilineComment(int startLine, int startPos)
{
    while (! reader.isEof()) {
        wchar_t ch = reader.getNextChar();
        pos++;
        if ('\n' == ch) {
            pos = 0;
            line++;
        } else if (('*' == ch) && (! reader.isEof())) {
            wchar_t nextCh = reader.getNextChar();
            if ('/' != nextCh)
                reader.ungetChar(nextCh);
        }
    }
    throw Exception(L"Remark started at " + posToStr(startLine, startPos)
            + L" is not closed");
}


void Lexal::skipSpaces()
{
    while (! reader.isEof()) {
        wchar_t ch = reader.getNextChar();
        pos++;
        if (! isWhiteSpace(ch)) {
            if ('#' == ch)
                skipToLineEnd();
            else {
                bool finish = false;
                if (('/' == ch) && (! reader.isEof())) {
                    wchar_t nextCh = reader.getNextChar();
                    pos++;
                    if ('/' == nextCh)
                        skipToLineEnd();
                    else if ('*' == nextCh)
                        skipMultilineComment(line, pos);
                    else {
                        pos--;
                        reader.ungetChar(nextCh);
                        finish = true;
                    }
                } else
                    finish = true;
                if (finish) {
                    pos--;
                    reader.ungetChar(ch);
                    return;
                }
            }
        } else
            if ('\n' == ch) {
                pos = 0;
                line++;
            }
    }
}

