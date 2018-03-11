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


#ifndef __TABLE_H__
#define __TABLE_H__


#include <string>
#include <map>
#include "lexal.h"


class Table;


class Value
{
    public:
        typedef enum Type {
            Integer,
            Double,
            String,
            Table
        };
    
    public:
        virtual ~Value() = default;

    public:
        virtual ::Table* asTable() const = 0;
        virtual Type getType() const = 0;
        virtual int asInt() const = 0;
        virtual double asDouble() const = 0;
        virtual std::wstring asString() const = 0;
        virtual Value* clone() const = 0;
};


class Table
{
    public:
        typedef std::map<std::wstring, Value*> ValuesMap;
        typedef ValuesMap::const_iterator Iterator;
        
    private:
        ValuesMap fields;
        int lastArrayIndex;
    
    public:
        Table();
        Table(const Table &table);
        explicit Table(const std::string &fileName);
        Table(Lexal &lexal, int startLine, int startPos);
        ~Table();

    public:
        Table& operator = (const Table &table);
        std::wstring toString() const;
        std::wstring toString(bool printBraces, bool butify, int ident) const;
        bool isArray() const;
        void save(const std::wstring &fileName) const;

    public:
        Iterator begin() const { return fields.begin(); };
        Iterator end() const { return fields.end(); };
        bool hasKey(const std::wstring &key) const;
        Value::Type getType(const std::wstring &key) const;
        std::wstring getString(const std::wstring &key, const std::wstring &dflt = L"") const;
        int getInt(const std::wstring &key, int dflt=0) const;
        Table* getTable(const std::wstring &key, Table *dflt=NULL) const;
        void setString(const std::wstring &key, const std::wstring &value);
        void setInt(const std::wstring &key, int value);
        
    private:
        void parse(Lexal &lexal, bool needBracket, int startLine, int startPos);
        void addArrayElement(Lexal &lexal, const Lexeme &lexeme);
        void addValuePair(Lexal &lexal, const std::wstring &name);
        void setValue(const std::wstring &key, Value *value);
};



#endif

