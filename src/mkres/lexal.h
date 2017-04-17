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
        Lexeme() { };
        Lexeme(Type type, const std::wstring &content, int line, int pos);
        ~Lexeme() { };

    public:
        const Type getType() const { return type; };
        const std::wstring getContent() const { return content; };
        std::wstring getPosStr() const;
        int getLine() const { return line; };
        int getPos() const { return pos; };
};


class Lexal
{
    private:
        UtfStreamReader &reader;
        int line;
        int pos;
        
    public:
        Lexal(UtfStreamReader &reader);
        ~Lexal() { };

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

