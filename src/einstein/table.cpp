#include "table.h"

#include <fstream>
#include "convert.h"
#include "unicode.h"
#include "streams.h"
#include "lexal.h"
#include "exceptions.h"


class IntValue: public Value
{
    private: 
        int value;
    
    public:
        IntValue(int val) { value = val; };
        virtual ~IntValue() { };

    public:
        virtual Type getType() const { return Value::Integer; };
        virtual int asInt() const { return value; };
        virtual double asDouble() const { return value; };
        virtual std::wstring asString() const { return toString(value); };
        virtual ::Table* asTable() const { 
            throw Exception(L"Can't convert integer to table"); 
        };
        virtual Value* clone() const { return new IntValue(value); };
};


class DoubleValue: public Value
{
    private: 
        double value;
    
    public:
        DoubleValue(double val) { value = val; };
        virtual ~DoubleValue() { };

    public:
        virtual Type getType() const { return Value::Double; };
        virtual int asInt() const { return (int)value; };
        virtual double asDouble() const { return value; };
        virtual std::wstring asString() const { return toString(value); };
        virtual ::Table* asTable() const { 
            throw Exception(L"Can't convert double to table"); 
        };
        virtual Value* clone() const { return new DoubleValue(value); };
};


class StringValue: public Value
{
    private: 
        std::wstring value;
    
    public:
        StringValue(const std::wstring& val): value(val) { };
        virtual ~StringValue() { };

    public:
        virtual Type getType() const { return Value::String; };
        virtual int asInt() const { return strToInt(value); };
        virtual double asDouble() const { return strToDouble(value); };
        virtual std::wstring asString() const { return value; };
        virtual ::Table* asTable() const { 
            throw Exception(L"Can't convert string to table"); 
        };
        virtual Value* clone() const { return new StringValue(value); };
};


class TableValue: public Value
{
    private: 
        ::Table *value;
    
    public:
        TableValue(::Table *val) { value = val; };
        virtual ~TableValue() { delete value; };

    public:
        virtual Type getType() const { return Value::Table; };
        virtual int asInt() const { 
            throw Exception(L"Can't convert table to int"); 
        };
        virtual double asDouble() const { 
            throw Exception(L"Can't convert table to double"); 
        };
        virtual std::wstring asString() const { 
            throw Exception(L"Can't convert table to string"); 
        };
        virtual ::Table* asTable() const { return value; };
        virtual Value* clone() const { 
            return new TableValue(new ::Table(*value)); 
        };
};


Table::Table(const Table &table)
{
    *this = table;
}

Table::Table(const std::string &fileName)
{
    lastArrayIndex = 0;
    std::ifstream stream(fileName.c_str(), 
            std::ios::binary | std::ios::in);
    if (! stream.good())
        throw Exception(L"Error opening file '" + fromMbcs(fileName) + L"");
    UtfStreamReader reader(&stream);
    Lexal lexal = Lexal(reader);
    parse(lexal, false, 0, 0);
}

Table::Table(Lexal &lexal, int line, int pos)
{
    lastArrayIndex = 0;
    parse(lexal, true, line, pos);
}

Table::Table()
{
    lastArrayIndex = 0;
}


Table::~Table()
{
    for (ValuesMap::iterator i = fields.begin(); i != fields.end(); i++)
        delete (*i).second;
}


Table& Table::operator = (const Table &table)
{
    if (this == &table) 
        return *this;

    fields.clear();
    lastArrayIndex = table.lastArrayIndex;
    for (ValuesMap::const_iterator i = table.fields.begin(); 
            i != table.fields.end(); i++)
        fields[(*i).first] = (*i).second->clone();
    
    return *this;
}


static Value* lexToValue(Lexal &lexal, const Lexeme &lexeme)
{
    switch (lexeme.getType())
    {
        case Lexeme::Ident:
        case Lexeme::String: 
            return new StringValue(lexeme.getContent());
        case Lexeme::Integer: 
            return new IntValue(strToInt(lexeme.getContent()));
        case Lexeme::Float: 
            return new DoubleValue(strToDouble(lexeme.getContent()));
        case Lexeme::Symbol: 
            if (L"{" == lexeme.getContent())
                return new TableValue(new Table(lexal, lexeme.getLine(),
                            lexeme.getPos()));
        default:
            throw Exception(L"Invalid lexeme type at " + lexeme.getPosStr());
    }
}


void Table::addValuePair(Lexal &lexal, const std::wstring &name)
{
    Lexeme lex = lexal.getNext();
    if (Lexeme::Eof == lex.getType())
        throw Exception(L"Unexpected end of file");
    fields[name] = lexToValue(lexal, lex);
}

void Table::addArrayElement(Lexal &lexal, const Lexeme &lexeme)
{
    fields[numToStr(lastArrayIndex)] = lexToValue(lexal, lexeme);
    lastArrayIndex++;
}

void Table::parse(Lexal &lexal, bool needBracket, int startLine, int startPos)
{
    Lexeme lex;
    bool read = true;
    
    while (true) {
        if (read) {
            lex = lexal.getNext();
            read = true;
        }
        Lexeme::Type type = lex.getType();
        if (Lexeme::Eof == type) {
            if (! needBracket)
                return;
            else
                throw Exception(L"Table started at " + Lexal::posToStr(
                            startLine, startPos) + L" is never finished");
        } else if ((Lexeme::Symbol == type) && (L"}" == lex.getContent())
                && needBracket)
            return;
        else if ((Lexeme::Symbol == type) && ((L"," == lex.getContent()) ||
                    (L";" == lex.getContent()))) {
            // ignore separator
        } else if (Lexeme::Ident == type) {
            Lexeme nextLex = lexal.getNext();
            if (Lexeme::Symbol == nextLex.getType()) {
                if (L"=" == nextLex.getContent())
                    addValuePair(lexal, lex.getContent());
                else {
                    addArrayElement(lexal, lex);
                    lex = nextLex;
                    read = false;
                }
            } else
                throw Exception(L"Unexpected token at " + Lexal::posToStr(
                                startLine, startPos));
        } else
            addArrayElement(lexal, lex);
    }
}


bool Table::hasKey(const std::wstring &key) const
{
    return (fields.end() != fields.find(key));
}


static std::wstring encodeString(const std::wstring &str)
{
    std::wstring res;
    int len = str.length();

    res += L"\"";
    for (int i = 0; i < len; i++) {
        wchar_t ch = str[i];
        switch (ch) {
            case '\n':  res += L"\\n";  break;
            case '\r':  res += L"\\r";  break;
            case '\'':  res += L"\\'";  break;
            case '\"':  res += L"\\\"";  break;
            case '\\':  res += L"\\\\";  break;
            default:
                res += ch;
        }
    }
    res += L"\"";

    return res;
}

static std::wstring printValue(Value *value, bool butify, int spaces)
{
    if (! value)
        return L"";

    switch (value->getType()) {
        case Value::Integer: 
            return toString(value->asInt());
            
        case Value::Double: 
            {
                std::wstring s = toString(value->asDouble());
                if (s.find(L'.') >= s.length())
                    s.append(L".0");
                return s;
            }
            
        case Value::String: 
            return encodeString(value->asString());

        case Value::Table: 
            return value->asTable()->toString(true, butify, spaces);
    }
        
    return L"";
}


bool Table::isArray() const
{
    int size = fields.size();

    for (int i = 0; i < size; i++) {
        if (! hasKey(::toString(i)))
            return false;
    }

    return true;
}


std::wstring Table::toString() const
{
    return toString(false, true, 0);
}


static bool isInteger(const std::wstring &str)
{
    int sz = str.length();
    for (int i = 0; i < sz; i++) {
        wchar_t ch = str[i];
        if ((ch < L'0') || (ch > L'9'))
            return false;
    }
    return true;
}


std::wstring Table::toString(bool printBraces, bool butify, int spaces) const
{
    std::wstring res;

    if (printBraces)
        res += butify ? L"{\n" : L"{";
    bool printNames = ! isArray();

    for (ValuesMap::const_iterator i = fields.begin(); i != fields.end(); 
            i++) 
    {
        const std::wstring &name = (*i).first;
        Value *value = (*i).second;
        if (butify)
            for (int j = 0; j < spaces; j++) 
                res += L" ";
        else
            res += L" ";
        if (printNames && (! isInteger(name)))
            res += name + L" = ";
        res += printValue(value, butify, spaces + 4);
        if (printNames)
            res += butify ? L";\n" : L";";
        else
            res += butify ? L",\n" : L",";
    }

    if (printBraces) {
        if (! butify)
            res += L" }";
        else {
            int ident = (spaces >= 4) ? spaces - 4 : 0;
            for (int j = 0; j < ident; j++) 
                res += L" ";
            res += L"}";
        }
    }

    return res;
}


Value::Type Table::getType(const std::wstring &key) const
{
    ValuesMap::const_iterator i = fields.find(key);
    if (i == fields.end())
        throw Exception(L"Field '" + key + L"' doesn't exists in the table");
    return (*i).second->getType();
}


std::wstring Table::getString(const std::wstring &key, 
        const std::wstring &dflt) const
{
    ValuesMap::const_iterator i = fields.find(key);
    return (i != fields.end()) ? (*i).second->asString() : dflt;
}

int Table::getInt(const std::wstring &key, int dflt) const
{
    ValuesMap::const_iterator i = fields.find(key);
    return (i != fields.end()) ? ((*i).second ? (*i).second->asInt() : dflt) : dflt;
}

double Table::getDouble(const std::wstring &key, double dflt) const
{
    ValuesMap::const_iterator i = fields.find(key);
    return (i != fields.end()) ? (*i).second->asDouble() : dflt;
}

Table* Table::getTable(const std::wstring &key, Table *dflt) const
{
    ValuesMap::const_iterator i = fields.find(key);
    return (i != fields.end()) ? (*i).second->asTable() : dflt;
}

void Table::setValue(const std::wstring &key, Value *value)
{
    ValuesMap::const_iterator i = fields.find(key);
    if (i != fields.end())
        delete (*i).second;
    fields[key] = value;
}

void Table::setString(const std::wstring &key, const std::wstring &value)
{
    setValue(key, new StringValue(value));
}

void Table::setInt(const std::wstring &key, int value)
{
    setValue(key, new IntValue(value));
}

void Table::setDouble(const std::wstring &key, double value)
{
    setValue(key, new DoubleValue(value));
}

void Table::setTable(const std::wstring &key, Table *value)
{
    setValue(key, new TableValue(value));
}


void Table::save(const std::wstring &fileName) const
{
    std::ofstream stream(toMbcs(fileName).c_str(), std::ios::out 
            | std::ios::binary);
    if (! stream.good())
        throw Exception(L"Can't open '" + fileName + L"' for writing");
    std::string utfStr(toUtf8(toString()));
    stream.write(utfStr.c_str(), utfStr.length());
    if (! stream.good())
        throw Exception(L"Can't write table to file '" + fileName + L"'");
    stream.close();
}

