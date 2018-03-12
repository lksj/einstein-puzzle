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


#include <SDL/SDL.h>
#include "screen.h"

#include "exceptions.h"
#include "unicode.h"
#include "utils.h"

#include <sstream>

#define UNSCALED_WIDTH      800
#define UNSCALED_HEIGHT     600
#define NUM_MODES           4

int modes[NUM_MODES][2]={{800,600},{1024,768},{1152,864},{1400,1050}};

int DESKTOP_WIDTH = 0;
int DESKTOP_HEIGHT = 0;

Screen::Screen()
    : screen(nullptr), scale(1.0), fullScreen(false), screenSize(0),
        mouseImage(nullptr), mouseSave(nullptr), mouseVisible(false),
        regionsList(nullptr), maxRegionsList(0), saveX(0), saveY(0),
        niceCursor(false), cursor(nullptr), emptyCursor(nullptr)
{
}

Screen::~Screen()
{
    SDL_SetCursor(cursor);
    SDL_FreeSurface(mouseImage);
    SDL_FreeSurface(mouseSave);
    if (regionsList) free(regionsList);
}

std::vector<std::wstring> Screen::getModeList()
{
    std::vector<std::wstring> v;
    int n = NUM_MODES;
    v.reserve(n);
    
    for (int i = 0; i < n; i++)
    {
        std::wstringstream streamVal;
        streamVal << modes[i][0] << "x" << modes[i][1];
        v.push_back(streamVal.str());
    }
    return v;
}


void Screen::setMode(bool isFullScreen)
{
    if (!screen || fullScreen != isFullScreen)
    {
        fullScreen = isFullScreen;
        applyMode();
    }
}

void Screen::applyMode()
{
    int flags = SDL_SWSURFACE /*| SDL_OPENGL*/;
    if (!screen)
    {
        screen = SDL_SetVideoMode(0, 0, 24, flags);
        DESKTOP_WIDTH = screen->w;
        DESKTOP_HEIGHT = screen->h;
    }
    
    int i = 0;
    if (fullScreen)
    {
        flags = flags | SDL_FULLSCREEN;
    }
    else
    {
      i = screenSize;
    }
    
    VideoMode mode = VideoMode(modes[i][0], modes[i][1], 24, fullScreen);
    scale = mode.getWidth()/(float)UNSCALED_WIDTH;
    
    SDL_FreeSurface(screen);
    screen = SDL_SetVideoMode(mode.getWidth(), mode.getHeight(), mode.getBpp(), flags);
    regions.clear();
    if (! screen)
        throw Exception(L"Couldn't set video mode: " + 
                fromMbcs((SDL_GetError())));
}


int Screen::getWidth() const
{
    if (screen) 
        return UNSCALED_WIDTH;
    else 
        throw Exception(L"No video mode selected"); 
}


int Screen::getHeight() const
{
    if (screen) 
        return UNSCALED_HEIGHT;
    else 
        throw Exception(L"No video mode selected");
}

void Screen::setMouseImage(SDL_Surface *image)
{
    if (mouseImage) {
        SDL_FreeSurface(mouseImage);
        mouseImage = nullptr;
    }
    if (mouseSave) {
        SDL_FreeSurface(mouseSave);
        mouseSave = nullptr;
    }

    if (! image) return;
    
    mouseImage = SDL_DisplayFormat(image);
    if (! mouseImage) 
        throw Exception(L"Error creating surface");
    //mouseSave = SDL_DisplayFormat(image);
    mouseSave = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
            image->w, image->h, screen->format->BitsPerPixel,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
    if (! mouseSave) {
        SDL_FreeSurface(mouseImage);
        throw Exception(L"Error creating buffer surface");
    }
    SDL_SetColorKey(mouseImage, SDL_SRCCOLORKEY, 
            SDL_MapRGB(mouseImage->format, 0, 0, 0));
}


void Screen::hideMouse()
{
    if (! mouseVisible)
        return;
   
    if (! niceCursor) {
        mouseVisible = false;
        return;
    }

    if (mouseSave) {
        SDL_Rect src = { 0, 0, mouseSave->w, mouseSave->h };
        SDL_Rect dst = { saveX, saveY, mouseSave->w, mouseSave->h };
        if (src.w > 0) {
            SDL_BlitSurface(mouseSave, &src, screen, &dst);
            addRegionToUpdate(scaleDown(dst.x), scaleDown(dst.y), dst.w, dst.h);
        }
    }
    mouseVisible = false;
}

void Screen::showMouse()
{
    if (mouseVisible)
        return;
    
    if (! niceCursor) {
        mouseVisible = true;
        return;
    }
    
    if (mouseImage && mouseSave) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        saveX = x;
        saveY = y;
        SDL_Rect src = { 0, 0, mouseSave->w, mouseSave->h };
        SDL_Rect dst = { x, y, mouseImage->w, mouseImage->h };
        if (src.w > 0) {
            SDL_BlitSurface(screen, &dst, mouseSave, &src);
            SDL_BlitSurface(mouseImage, &src, screen, &dst);
            addRegionToUpdate(scaleDown(dst.x), scaleDown(dst.y), dst.w, dst.h);
        }
    }
    mouseVisible = true;
}

void Screen::updateMouse()
{
    hideMouse();
    showMouse();
}

void Screen::flush()
{
    if (regions.empty()) return;
    
    if (! regionsList) {
        regionsList = (SDL_Rect*)malloc(sizeof(SDL_Rect) * regions.size());
        if (! regionsList) {
            regions.clear();
            throw Exception(L"Error allocating regions buffer");
        }
        maxRegionsList = regions.size();
    } else {
        if (maxRegionsList < (int)regions.size()) {
            SDL_Rect *r = (SDL_Rect*)realloc(regionsList, 
                    sizeof(SDL_Rect) * regions.size());
            if (! r) {
                regions.clear();
                free(regionsList);
                throw Exception(L"Error incrementing regions buffer");
            }
            regionsList = r;
            maxRegionsList = regions.size();
        }
    }

    int j = 0;
    for (std::list<SDL_Rect>::iterator i = regions.begin();
            i != regions.end(); ++i, ++j)
        regionsList[j] = *i;

    SDL_UpdateRects(screen, regions.size(), regionsList);
    regions.clear();
}


void Screen::addRegionToUpdate(int chkX, int chkY, int chkW, int chkH)
{
    int x = scaleUp(chkX);
    int y = scaleUp(chkY);
    int w = scaleUp(chkW);
    int h = scaleUp(chkH);
    
    if (((x >= screen->w) || (y >= screen->h)) || (0 >= w) || (0 >= h))
        return;
    if ((x + w < 0) || (y + h < 0))
        return;
    if (x + w > screen->w)
        w = screen->w - x;
    if (y + h > screen->h)
        h = screen->h - y;
    if (0 > x) {
        w = w + x;
        x = 0;
    }
    if (0 > y) {
        h = h + y;
        y = 0;
    }
    SDL_Rect r = { x, y, w, h };
    regions.push_back(r);
}


void Screen::draw(int x, int y, SDL_Surface *tile)
{
    blitDraw(x, y, tile, screen);
}

void Screen::drawScaled(int x, int y, SDL_Surface *tile)
{
    SDL_Surface *s = scaleUp(tile);
    blitDraw(scaleUp(x), scaleUp(y), s, screen);
    SDL_FreeSurface(s);
}

void Screen::setCursor(bool nice)
{
    if (nice == niceCursor)
        return;
    
    bool oldVisible = mouseVisible;
    if (mouseVisible)
        hideMouse();
    niceCursor = nice;

    if (niceCursor)
        SDL_SetCursor(emptyCursor);
    else
        SDL_SetCursor(cursor);
    
    if (oldVisible)
        showMouse();
}

void Screen::initCursors()
{
    cursor = SDL_GetCursor();
    Uint8 t = 0;
    emptyCursor = SDL_CreateCursor(&t, &t, 8, 1, 0, 0);
}

void Screen::doneCursors()
{
    if (niceCursor)
        SDL_SetCursor(cursor);
    SDL_FreeCursor(emptyCursor);
}

SDL_Surface* Screen::createSubimage(int x, int y, int width, int height)
{
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
            scaleUp(width), scaleUp(height), screen->format->BitsPerPixel,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
    if (! s)
        throw Exception(L"Error creating buffer surface");
    SDL_Rect src = { scaleUp(x), scaleUp(y), scaleUp(width), scaleUp(height) };
    SDL_Rect dst = { 0, 0, src.w, src.h };
    SDL_BlitSurface(screen, &src, s, &dst);
    return s;
}

void Screen::drawWallpaper(const std::wstring &name)
{
    drawTiled(name, screen);
    addRegionToUpdate(0, 0, getWidth(), getHeight());
}

SDL_PixelFormat* Screen::getFormat()
{
    return screen->format;
}

void Screen::setClipRect(SDL_Rect* rect)
{
    if (rect)
    {
      SDL_Rect sRect = { scaleUp(rect->x), scaleUp(rect->y),
                                      scaleUp(rect->w), scaleUp(rect->h) };
      SDL_SetClipRect(screen, &sRect);
    }
    else
    {
        SDL_SetClipRect(screen, rect);
    }
}

void Screen::setSize(int size)
{
    if (screenSize != size)
    {
      screenSize = size;
      if (!fullScreen)
      {
          applyMode();
      }
    }
}


float Screen::getScale()
{
    return scale;
}
