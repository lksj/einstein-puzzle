#ifndef __FORMATTER_H__
#define __FORMATTER_H__


#include <stdarg.h>
#include <string>
#include <vector>


class ArgValue;


/// Localized message formatter
class Formatter
{
    public:
        typedef enum {
            EMPTY_CMD = 0,
            TEXT_COMMAND,
            INT_ARG,
            STRING_ARG,
            DOUBLE_ARG,
            FLOAT_ARG
        } CmdType;
        
        typedef struct
        {
            CmdType type;
            void *data;
        } Command;

    private:
        int commandsCnt;
        int argsCnt;

        Command *commands;
        
        CmdType *args;

    public:
        /// Create localized message from message buffer.
        /// \param data buffer contained message file
        /// \param offset offset to message from buffer start
        Formatter(unsigned char *data, int offset);
        ~Formatter();

    public:
        /// Get message text.
        std::wstring getMessage() const;
        
        /// Fromat message
        /// \param ap list of arguments
        std::wstring format(va_list ap) const;

    private:
        std::wstring format(std::vector<ArgValue*> &argValues) const;
};




#endif

