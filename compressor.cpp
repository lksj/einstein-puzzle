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


#include "compressor.h"

#include "convert.h"
#include "exceptions.h"
#include <cstring>
#include <zlib.h>

ResourceCompressor::ResourceCompressor() 
    : priority(1000), stream(nullptr), dontDeleteStream(true)
{
}

ResourceCompressor::~ResourceCompressor() = default;


void ResourceCompressor::adjustBuffers(int fileSize)
{
    int maxComprSize = (int)((fileSize + 100.0) * 2.1);
    if (maxComprSize < 1024)
        maxComprSize = 1024;
    unpackedBuffer.setSize(fileSize);
    packedBuffer.setSize(maxComprSize);
}


static int writeInt(std::ostream *stream, int v)
{
    unsigned char b[4];

    for (unsigned char& i : b)
    {
        const int ib = v & 0xFF;
        v = v >> 8;
        i = ib;
    }
    
    stream->write((char*)&b, 4);

    return 4;
}


static int writeString(std::ostream *stream, const std::wstring &value)
{
    std::string v(toUtf8(value));
    const int len = v.length() + 1;
    stream->write(v.c_str(), len);
    return len;
}


int ResourceCompressor::writeHeader()
{
    int offset = writeString(stream, L"CRF");
    offset += writeInt(stream, 2);
    offset += writeInt(stream, 1);
    offset += writeInt(stream, priority);
    return offset;
}


void ResourceCompressor::showEntryStat(Entry &entry)
{
    std::cerr << entry.name << "  before: " << entry.realSize <<
        ", after: " << entry.packedSize << ", ratio: " <<
        (100.0 / entry.realSize) * entry.packedSize << "%" << std::endl;
}

void ResourceCompressor::openStream(const std::string &outputFile)
{
    if ((outputFile.length()) && (outputFile != "-")) {
        dontDeleteStream = false;
        std::ofstream *s = new std::ofstream();
        stream = s;
        s->open(outputFile.c_str(), std::ios::out | std::ios::binary);
        if (stream->fail()) {
            delete stream;
            throw std::runtime_error("Can't open output file");
        }
    } else {
        dontDeleteStream = true;
        stream = &std::cout;
    }
}

void ResourceCompressor::closeStream()
{
    if (! dontDeleteStream)
        delete stream;
}

void ResourceCompressor::compress(const std::string &outputFile, bool verbose)
{
    openStream(outputFile);
    
    int offset = writeHeader();
    for (auto& e : entries)
    {
        compressEntry(e, offset);
        if (verbose)
            showEntryStat(e);
    }
    writeFooter(offset);

    closeStream();
}

void ResourceCompressor::printDeps(const std::string &outputFile, 
        const std::string &sourceFile)
{
    std::cout << std::endl << outputFile << ": " << sourceFile << " ";
    
    int width = outputFile.length() + sourceFile.length() + 3;
    
    for (auto& e : entries)
    {
        std::cout << " ";
        const int len = e.fileName.length() + 1;
        if (len + width > 77) {
            std::cout << "\\" << std::endl << "\t";
            width = 7;
        }
        std::cout << e.fileName;
        width += len;
    }
    
    std::cout << std::endl;
}


void ResourceCompressor::writeFooter(int &offset)
{
    const int start = offset;
    
    for (auto& e : entries)
    {
        offset += writeString(stream, e.name);
        offset += writeInt(stream, e.realSize);
        offset += writeInt(stream, e.offset);
        offset += writeInt(stream, e.packedSize);
        offset += writeInt(stream, e.comprLevel);
        offset += writeString(stream, e.group);
    }
        
    offset += writeInt(stream, start);
    offset += writeInt(stream, entries.size());
}


static int pack(char *in, int inSize, char *out, int maxOutSize, int level)
{
    if (! level) {
        if (inSize)
            memcpy(out, in, inSize);
        return inSize;
    } else {
        z_stream zs;
        memset(&zs, 0, sizeof(z_stream));
        zs.next_in = (Bytef*)in;
        zs.avail_in = inSize;
        zs.next_out = (Bytef*)out;
        zs.avail_out = maxOutSize;

        if (deflateInit(&zs, level) != Z_OK)
            throw std::runtime_error("Can't init compressor");
        if (deflate(&zs, Z_FINISH) != Z_STREAM_END)
            throw std::runtime_error("Error compressing data");
        if (deflateEnd(&zs) != Z_OK)
            throw std::runtime_error("Error finishing compressor");

        return zs.total_out;
    }
}


void ResourceCompressor::readData(const std::wstring &fileName)
{
    std::ifstream ifs(toMbcs(fileName).c_str(), 
            std::ios::in | std::ios::binary);
    if (ifs.fail())
        throw Exception(L"Error opening file '" + fileName + L"'");

    ifs.seekg(0, std::ios::end);
    const int realSize = ifs.tellg();
    unpackedBuffer.setSize(realSize);
    ifs.seekg(0, std::ios::beg);
    if (realSize <= 0)
        throw Exception(L"File '" + fileName + L"' has invalid size");
    
    ifs.read((char*)unpackedBuffer.getData(), realSize);
    if (ifs.fail() || (ifs.gcount() != realSize))
        throw Exception(L"Error reading from file '" + fileName + L"'");
    ifs.close();

}

void ResourceCompressor::runFormatter(Formatter *formatter, 
        const std::wstring &fileName)
{
    unpackedBuffer.setSize(0);
    unpackedBuffer.gotoPos(0);
    formatter->format(fileName, unpackedBuffer);
}

void ResourceCompressor::compressEntry(Entry &entry, int &offset)
{
    entry.offset = offset;
    
    if (! entry.formatter)
        readData(entry.fileName);
    else
        runFormatter(entry.formatter, entry.fileName);
    entry.realSize = unpackedBuffer.getSize();
    adjustBuffers(entry.realSize);
    
    entry.packedSize = pack((char*)unpackedBuffer.getData(), entry.realSize,
            (char*)packedBuffer.getData(), packedBuffer.getSize(), 
            entry.comprLevel);
    stream->write((char*)packedBuffer.getData(), entry.packedSize);
    
    offset += entry.packedSize;
}



