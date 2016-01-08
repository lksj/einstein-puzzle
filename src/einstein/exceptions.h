#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__


#include <string>
#include <iostream>


class Exception
{
    private:
        std::wstring message;
    
    public:
        Exception(const std::wstring& msg) { message = msg;  /*std::cout << msg << std::endl;*/ };
        virtual ~Exception() { };

    public:
        const std::wstring& getMessage() const { return message; };
};


#endif

