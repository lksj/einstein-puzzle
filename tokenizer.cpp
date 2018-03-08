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


#include "tokenizer.h"

#include <iostream>


std::wstring Token::toString() const
{
    if (Word == type)
        return L"Word: '" + content + L"'";
    else if (Para == type)
        return L"Para";
    else if (Eof == type)
        return L"Eof";
    else
        return L"Unknown";
}


static bool wisspace(wchar_t ch)
{
//std::cout << "'" << ch << "' " << ((int)ch) << " " << L'\n' << std::endl;
    return (L' ' == ch) || (L'\n' == ch) || (L'\r' == ch)
        || (L'\t' == ch) || (L'\f' == ch) || (L'\v' == ch);
}


bool Tokenizer::skipSpaces(bool notSearch)
{
    int len = text.length();
    bool foundDoubleReturn = false;
    while ((len > currentPos) && wisspace(text[currentPos])) {
        currentPos++;
        if ((! notSearch) && (L'\n' == text[currentPos - 1]) 
                && (currentPos < len) && (L'\n' == text[currentPos]))
            notSearch = foundDoubleReturn = true;
    }
    return foundDoubleReturn;
}


Token Tokenizer::getNextToken()
{
    if (!stack.empty()) {
        Token t(*stack.begin());
        stack.pop_front();
        return t;
    }
    int len = text.length();
    if (skipSpaces(! currentPos) && (currentPos < len))
        return Token(Token::Para);
    if (currentPos >= len)
        return Token(Token::Eof);
    int wordStart = currentPos;
    while ((len > currentPos) && (! wisspace(text[currentPos])))
        currentPos++;
    return Token(Token::Word, text.substr(wordStart, currentPos - wordStart));
}


void Tokenizer::unget(const Token &token)
{
    stack.push_back(token);
}


bool Tokenizer::isFinished()
{
    if (!stack.empty())
        return false;
    return currentPos >= (int)text.length();
}

