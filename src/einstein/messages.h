#ifndef __MESSAGES_H__
#define __MESSAGES_H__


#include <map>
#include <string>
#include <stdarg.h>


class Resource;
class Formatter;
class Buffer;


/// Localized messages formatter
class Messages
{
    private:
        typedef struct {
            int score;
            Formatter *message;
        } ScoredStr;
        typedef std::map<std::wstring, ScoredStr> StrMap;
        StrMap messages;
    
    public:
        /// Create empty messages table.
        Messages();
        ~Messages();

    public:
        /// Load message tables from resources.
        void load();

        /// Get simple text string
        /// \param key message key
        std::wstring getMessage(const std::wstring &key) const;

        /// Shorter alias for getMessage
        /// \param key message key
        std::wstring operator [](const std::wstring &key) const {
            return getMessage(key);
        };
        
        /// Format message
        /// \param key message key
        std::wstring format(const wchar_t *key, ...) const;
        
        /// Shorter alias for format
        /// \param key message key
        std::wstring operator ()(const wchar_t *key, ...) const;
        
        /// Load messages from resource
        /// \param res resource
        void loadFromResource(Resource *res, Buffer *buffer);

    private:
        void loadBundle(int score, unsigned char *data, size_t size);
        std::wstring format(const wchar_t *key, va_list ap) const;
};


extern Messages msg;


#endif

