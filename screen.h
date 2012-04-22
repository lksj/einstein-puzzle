// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-04-22 by Jordan Evens <jordan.evens@gmail.com>

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


#ifndef __SCREEN_H__
#define __SCREEN_H__


#include "SDL/SDL.h"
#include <vector>
#include <list>
#include <string>


class VideoMode
{
    private:
        int width;
        int height;
        int bpp;
        bool fullScreen;

    public:
        VideoMode(int w, int h, int bpp, bool fullscreen) 
        { 
            width = w; 
            height = h; 
            this->bpp = bpp; 
            this->fullScreen = fullscreen;
        }

    public:
        int getWidth() const { return width; };
        int getHeight() const { return height; };
        int getBpp() const { return bpp; };
        bool isFullScreen() const { return fullScreen; };
};


class Screen
{
    private:
        SDL_Surface *screen;
        bool fullScreen;
        SDL_Surface *mouseImage;
        SDL_Surface *mouseSave;
        std::list<SDL_Rect> regions;
        bool mouseVisible;
        SDL_Rect *regionsList;
        int maxRegionsList;
        int saveX, saveY;
        bool niceCursor;
        SDL_Cursor *cursor, *emptyCursor;
    
    public:
        Screen();
        ~Screen();

    public:
        const VideoMode getVideoMode() const;
        int getWidth() const;
        int getHeight() const;
        void setMode(const VideoMode& mode);
        std::vector<VideoMode> getFullScreenModes() const;
        void centerMouse();
        void setMouseImage(SDL_Surface *image);
        void hideMouse();
        void showMouse();
        void updateMouse();
        void flush();
        void addRegionToUpdate(int x, int y, int w, int h);
        void setPixel(int x, int y, int r, int g, int b);
        SDL_Surface* getSurface() { return screen; };
        void draw(int x, int y, SDL_Surface *surface);
        void setCursor(bool nice);
        void initCursors();
        void doneCursors();
        SDL_Surface* createSubimage(int x, int y, int width, int height);
        void drawWallpaper(const std::wstring &name);
        SDL_PixelFormat* getFormat();
};


#endif

