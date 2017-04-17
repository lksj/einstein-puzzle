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
        virtual ~Value() { };

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
        Table(const std::string &fileName);
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
        double getDouble(const std::wstring &key, double dflt=0) const;
        Table* getTable(const std::wstring &key, Table *dflt=NULL) const;
        void setString(const std::wstring &key, const std::wstring &value);
        void setInt(const std::wstring &key, int value);
        void setDouble(const std::wstring &key, double value);
        void setTable(const std::wstring &key, Table *value);
        
    private:
        void parse(Lexal &lexal, bool needBracket, int startLine, int startPos);
        void addArrayElement(Lexal &lexal, const Lexeme &lexeme);
        void addValuePair(Lexal &lexal, const std::wstring &name);
        void setValue(const std::wstring &key, Value *value);
};



#endif

