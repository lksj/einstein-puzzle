#ifndef __FONT_H__
#define __FONT_H__


#include <string>
#include <SDL_ttf.h>


class Font
{
    private:
        TTF_Font *font;
        void *data;
    
    public:
        Font(const std::wstring &name, int ptsize);
        ~Font();

    public:
        void draw(SDL_Surface *s, int x, int y, int r, int g, int b, 
                bool shadow, const std::wstring &text);
        void draw(int x, int y, int r, int g, int b, bool shadow, 
                const std::wstring &text);
        int getWidth(const std::wstring &text);
        int getWidth(wchar_t ch);
        int getHeight(const std::wstring &text);
        void getSize(const std::wstring &text, int &width, int &height);
};


#endif

