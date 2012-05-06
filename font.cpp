// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-05-06 by Jordan Evens <jordan.evens@gmail.com>

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


#include "font.h"
#include "main.h"
#include "utils.h"
#include "unicode.h"


static Uint16 *convBuf = NULL;
static size_t bufSize = 0;


static Uint16 *strToUint16(const std::wstring &text)
{
    const wchar_t *str = text.c_str();
    if ((! str) || (sizeof(wchar_t) == sizeof(Uint16)))
        return (Uint16*)str;
    else {
        size_t len = wcslen(str);
        if (! convBuf) {
            size_t sz = len * 2 + 1;
            convBuf = (Uint16*)malloc(sizeof(Uint16) * sz);
            bufSize = sz;
        } else
            if (bufSize < len + 1) {
                size_t sz = len * 2 + 1;
                convBuf = (Uint16*)realloc(convBuf, sizeof(Uint16) * sz);
                // I should check if it is NULL, but I'm too lazy today
                bufSize = sz;
            }
        for (unsigned int i = 0; i <= len; i++)
            convBuf[i] = (Uint16)str[i];
        return convBuf;
    }
}



Font::Font(const std::wstring &name, int ptsize):
    name(name), uSize(ptsize)
{
    data = resources->getRef(name, resSize);
    if (! data)
        throw Exception(name + L" not found");
    
    scale = -1.0;
    font = NULL;
    rescale();
}


Font::~Font()
{
    TTF_CloseFont(font);
    resources->delRef(data);
}


void Font::draw(SDL_Surface *s, int x, int y, int r, int g, int b, 
        bool shadow, const std::wstring &text)
{
    if (text.length() < 1)
        return;
    
    Uint16 *str = strToUint16(text);
    
    if (shadow) {
        SDL_Color color = { 1, 1, 1, 1 };
        SDL_Surface *surface = TTF_RenderUNICODE_Blended(font, str, color);
        SDL_Rect src = { 0, 0, surface->w, surface->h };
        SDL_Rect dst = { x+1, y+1, surface->w, surface->h };
        SDL_BlitSurface(surface, &src, s, &dst);
        SDL_FreeSurface(surface);
    }
    SDL_Color color = { r, g, b, 0 };
    SDL_Surface *surface = TTF_RenderUNICODE_Blended(font, str, color);
    SDL_Rect src = { 0, 0, surface->w, surface->h };
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_BlitSurface(surface, &src, s, &dst);
    SDL_FreeSurface(surface);
}

void Font::draw(int x, int y, int r, int g, int b, bool shadow, 
        const std::wstring &text)
{
    rescale();
    draw(screen.getSurface(), screen.doScale(x), screen.doScale(y), r,g,b, shadow, text);
}

void Font::rescale()
{
    if (screen.getScale() != scale)
    {
        TTF_CloseFont(font);
        scale = screen.getScale();
        font = loadFont(screen.doScale(uSize));
    }
}

TTF_Font* Font::loadFont(int ptsize)
{
    SDL_RWops *op = SDL_RWFromMem(data, resSize);
    TTF_Font* f = TTF_OpenFontRW(op, 1, ptsize);

    if (! f)
        throw Exception(L"Error loading font " + name);
    
    return f;
}

int Font::getWidth(const std::wstring &text)
{
    rescale();
    int w, h;
    Uint16 *str = strToUint16(text);
    TTF_SizeUNICODE(font, str, &w, &h);
    return w;
}

int Font::getWidth(wchar_t ch)
{
    rescale();
    int minx, maxx, miny, maxy, advance;
    TTF_GlyphMetrics(font, (Uint16)ch, &minx, &maxx, &miny, &maxy, &advance);
    return advance;
}

int Font::getHeight(const std::wstring &text)
{
    rescale();
    int w, h;
    Uint16 *str = strToUint16(text);
    TTF_SizeUNICODE(font, str, &w, &h);
    return h;
}

void Font::getSize(const std::wstring &text, int &width, int &height)
{
    rescale();
    Uint16 *str = strToUint16(text);
    TTF_SizeUNICODE(font, str, &width, &height);
}
