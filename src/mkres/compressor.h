#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__


#include <string>
#include <iostream>
#include <list>
#include <fstream>
#include "format.h"



class Entry
{
    public:
        std::wstring name;
        int realSize;
        int offset;
        int packedSize;
        int comprLevel;
        std::wstring group;
        std::wstring fileName;
        Formatter *formatter;

    public:
        Entry(const std::wstring &n, int level, const std::wstring &grp,
                const std::wstring &fn, Formatter *frmt): 
                        name(n), group(grp), fileName(fn)
                
        {
            comprLevel = level;
            offset = packedSize = realSize = 0;
            formatter = frmt;
        };
};


class ResourceCompressor
{
    private:
        typedef std::list<Entry> Entries;
        Entries entries;
        int priority;
        Buffer unpackedBuffer, packedBuffer;
        std::ostream *stream;
        bool dontDeleteStream;
    
    public:
        ResourceCompressor();
        ~ResourceCompressor();

    public:
        void add(const Entry &entry) { entries.push_back(entry); }
        void setPriority(int p) { priority = p; };
        void compress(const std::string &outputFile, bool verbose);
        void printDeps(const std::string &outputFile, 
                const std::string &sourceFile);

    private:
        void adjustBuffers(int fileSize);
        int writeHeader();
        void compressEntry(Entry &entry, int &offset);
        void showEntryStat(Entry &entry);
        void writeFooter(int &offset);
        void openStream(const std::string &outputFile);
        void closeStream();
        void readData(const std::wstring &fileName);
        void runFormatter(Formatter *formatter, const std::wstring &fileName);
};


#endif

