#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <stdlib.h>
#include <string>


/// Dynamic growing buffer
class Buffer
{
    private:
        size_t size;
        size_t allocated;
        void *data;
        int currentPos;
    
    public:
        /// Create buffer
        /// \param size initial size of buffer
        /// \param allocated bytes to allocate
        Buffer(int size=0, int allocated=1024);
        
        ~Buffer();

    public:
        /// Set buffer size, expands memory if needed
        /// \param size new size
        void setSize(size_t size);

        /// Get current size of buffer
        size_t getSize();

        /// Get actual bytes used by buffer
        size_t getAllocated();

        /// Get pointer to data
        void* getData();

    public:
        /// Move pointer to specified position.
        /// \param offset offset from buffer start
        void gotoPos(int offset);

        /// Add data to buffer and advance current position by data length.
        /// Grow buffer if needed.
        /// \param data pointer to data
        /// \param length data size.
        size_t putData(const unsigned char *data, size_t length);
        
        /// Add data to buffer and advance current position by data length.
        /// Grow buffer if needed.
        /// \param data pointer to data
        /// \param length data size.
        size_t putData(const char *data, size_t length) {
            return putData((const unsigned char*)data, length);
        };

        /// Add integer to buffer and advance current position by 4.
        /// Grow buffer if needed.
        /// \param value value to add.
        size_t putInteger(int value);

        /// Add string to buffer encoded in UTF-8 and advance current 
        /// position by string length.  String stored prefixed by
        /// string length.
        /// Grow buffer if needed.
        /// \param string value to add.
        size_t putUtf8(const std::wstring &string);
        
        /// Add byte to buffer and advance current position by 1.
        /// Grow buffer if needed.
        /// \param value value to add.
        size_t putByte(unsigned char value);
};


#endif

