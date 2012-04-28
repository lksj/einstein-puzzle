// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-04-27 by Jordan Evens <jordan.evens@gmail.com>

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

int modes[][2]={{800,600},{1024,768},{1152,864},{1400,1050}};

int DESKTOP_WIDTH = 0;
int DESKTOP_HEIGHT = 0;

Screen::Screen()
{
    screen = NULL;
    unscaled = NULL;
    mouseImage = NULL;
    mouseSave = NULL;
    mouseVisible = false;
    regionsList = NULL;
    maxRegionsList = 0;
    scale = 1;
}

Screen::~Screen()
{
    SDL_SetCursor(cursor);
    if (mouseImage) SDL_FreeSurface(mouseImage);
    if (mouseSave) SDL_FreeSurface(mouseSave);
    if (regionsList) free(regionsList);
}

std::vector<std::wstring> Screen::getModeList()
{
    std::vector<std::wstring> v;
    int n = sizeof(modes)/sizeof(int*);
    v.reserve(n);
    
    for (int i = 0; i < n; i++)
    {
        std::wstringstream streamVal;
        streamVal << modes[i][0] << "x" << modes[i][1];
        v.push_back(streamVal.str());
    }
    return v;
}


const VideoMode Screen::getVideoMode() const
{
    return VideoMode(screen->w, screen->h, screen->format->BitsPerPixel, fullScreen);
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
    if (!unscaled)
    {
      unscaled = SDL_CreateRGBSurface(SDL_SWSURFACE, UNSCALED_WIDTH, UNSCALED_HEIGHT, 
                                                            screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask,
                                                            screen->format->Bmask, screen->format->Amask);
    }
}


std::vector<VideoMode> Screen::getFullScreenModes() const
{
    std::vector<VideoMode> modes;
    return modes;
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

void Screen::centerMouse()
{
    if (screen) 
        SDL_WarpMouse(screen->w / 2, screen->h / 2);
    else 
        throw Exception(L"No video mode selected");
}

void Screen::setMouseImage(SDL_Surface *image)
{
    if (mouseImage) {
        SDL_FreeSurface(mouseImage);
        mouseImage = NULL;
    }
    if (mouseSave) {
        SDL_FreeSurface(mouseSave);
        mouseSave = NULL;
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
            addRegionToUpdate(reverseScale(dst.x), reverseScale(dst.y), dst.w, dst.h);
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
            addRegionToUpdate(reverseScale(dst.x), reverseScale(dst.y), dst.w, dst.h);
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
    if (! regions.size()) return;
    
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
            i != regions.end(); i++, j++)
        regionsList[j] = *i;

    SDL_UpdateRects(screen, regions.size(), regionsList);
    regions.clear();
}


void Screen::addRegionToUpdate(int chkX, int chkY, int chkW, int chkH)
{
    int x = this->doScale(chkX);
    int y = this->doScale(chkY);
    int w = this->doScale(chkW);
    int h = this->doScale(chkH);
    
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
    SDL_Rect src = { 0, 0, tile->w, tile->h };
    SDL_Rect u_dst = { x, y, tile->w, tile->h };
    SDL_BlitSurface(tile, &src, unscaled, &u_dst);
    
    SDL_PixelFormat* fmt = screen->format;
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
                                                                    doScale(tile->w), doScale(tile->h), fmt->BitsPerPixel,
                                                                    fmt->Rmask, fmt->Gmask,
                                                                    fmt->Bmask, fmt->Amask);
    SDL_Rect s_dst = { 0, 0, s->w, s->h };
    SDL_SoftStretch(unscaled, &u_dst, s, &s_dst);
    
    SDL_Rect dst = { this->doScale(x), this->doScale(y), s->w, s->h };
    SDL_BlitSurface(s, &s_dst, screen, &dst);
    SDL_FreeSurface(s);
    
    //~ //this fixes the twitching when you click on menus but it causes new game to be really slow
    //~ SDL_Rect src_full = { 0, 0, unscaled->w, unscaled->h };
    //~ SDL_Rect dst_full = { 0, 0, screen->w, screen->h };
    //~ SDL_SoftStretch(unscaled, &src_full, screen, &dst_full);
}

void Screen::drawDirect(int x, int y, SDL_Surface *tile)
{
    SDL_Rect src = { 0, 0, tile->w, tile->h };
    SDL_Rect s_dst = { doScale(x), doScale(y), tile->w, tile->h };
    SDL_BlitSurface(tile, &src, screen, &s_dst);
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
            width, height, screen->format->BitsPerPixel,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
    if (! s)
        throw Exception(L"Error creating buffer surface");
    SDL_Rect src = { x, y, width, height };
    SDL_Rect dst = { 0, 0, width, height };
    SDL_BlitSurface(unscaled, &src, s, &dst);
    return s;
}

void Screen::drawWallpaper(const std::wstring &name)
{
    drawTiled(name, unscaled);
    
    //HACK: using draw causes seams so do the scale here
    SDL_Rect src_full = { 0, 0, unscaled->w, unscaled->h };
    SDL_Rect dst_full = { 0, 0, screen->w, screen->h };
    SDL_SoftStretch(unscaled, &src_full, screen, &dst_full);
}

SDL_PixelFormat* Screen::getFormat()
{
    return screen->format;
}

void Screen::setClipRect(SDL_Rect* rect)
{
    if (rect)
    {
      SDL_Rect sRect = { doScale(rect->x), doScale(rect->y),
                                      doScale(rect->w), doScale(rect->h) };
      SDL_SetClipRect(screen, &sRect);
    }
    else
    {
        SDL_SetClipRect(screen, rect);
    }
}

int Screen::doScale(int i)
{
    return (int)(i*scale);
}

int Screen::reverseScale(int i)
{
    return (int)(i/scale);
}

SDL_Surface* Screen::getRegion(int x, int y, int w, int h)
{
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
            24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0/*0xFF000000*/);
    SDL_Rect src = { x, y, w, h };
    SDL_Rect dst = { 0, 0, w, h };
    SDL_BlitSurface(unscaled, &src, s, &dst);
    
    return s;
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

SDL_Surface* Screen::getScaled()
{
    return screen;
}
