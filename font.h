// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-04-29 by Jordan Evens <jordan.evens@gmail.com>

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


#ifndef __FONT_H__
#define __FONT_H__


#include <string>
#include <SDL/SDL_ttf.h>


class Font
{
    private:
        TTF_Font *unscaled;
        TTF_Font *scaled;
        TTF_Font *font;
        float scale;
        void *data;
        std::wstring name;
        int uSize;
        int resSize;
    
    protected:
        void rescale();
        TTF_Font* loadFont(int ptsize);
    
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
        void setScaled(bool isScaled);
};


#endif

