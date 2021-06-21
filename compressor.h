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


#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__


#include "format.h"

#include <fstream>
#include <list>
#include <cstring>

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
        }
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
        void setPriority(int p) { priority = p; }
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

