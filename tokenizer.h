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

