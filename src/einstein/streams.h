#ifndef __STREAMS_H__
#define __STREAMS_H__


#include <fstream>
#include <list>


/// Read utf-8 file and convert it to wide characters
class UtfStreamReader
{
    private:
        /// Pointer to file stream
        std::ifstream *stream;

        /// Push back buffet
        std::list<wchar_t> backBuf;
    
    public:
        /// Create utf-8 stream reader.
        /// \param stream pointer to file stream.
        UtfStreamReader(std::ifstream *stream);
        
        /// Destructor
        ~UtfStreamReader();

    public:
        /// Read next unicode character.
        wchar_t getNextChar();

        /// Push back character.
        /// \param ch character to push back
        void ungetChar(wchar_t ch);

        /// Check if end of file reached.
        bool isEof();
};


#endif

