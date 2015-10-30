#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__


#include <string>
#include <iostream>
#include <exception>


/// Basic exception class
class Exception: public std::exception
{
    private:
        std::wstring message;
    
    public:
        /// Create exception
        /// \param msg message text
        Exception(const std::wstring& msg) throw() { message = msg; };
        virtual ~Exception() throw() { };

    public:
        /// Get error message.
        virtual const std::wstring& getMessage() const throw() { return message; };

        /// Inherited way of receiving error messags
        virtual const char* what() const throw() { return "Exception"; };
};


#endif

