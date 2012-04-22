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


#include <SDL/SDL.h>
#include "screen.h"
#include "exceptions.h"
#include "unicode.h"
#include "utils.h"

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
      if (scaleUp)
      {
          while (i < ((int)(sizeof(modes)/sizeof(modes[0]))-1) && 
                      modes[i][0] <= DESKTOP_WIDTH && modes[i][1] <= DESKTOP_HEIGHT)
          {
              i++;
          }
      }
    }
    
    VideoMode mode = VideoMode(modes[i][0], modes[i][1], 24, fullScreen);
    scale = mode.getWidth()/(float)UNSCALED_WIDTH;
    
    SDL_FreeSurface(screen);
    screen = SDL_SetVideoMode(mode.getWidth(), mode.getHeight(), mode.getBpp(), flags);
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
            addRegionToUpdate(dst.x, dst.y, dst.w, dst.h);
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
            addRegionToUpdate(dst.x, dst.y, dst.w, dst.h);
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


void Screen::setPixel(int x, int y, int r, int g, int b)
{
    SDL_LockSurface(unscaled);
    int bpp = unscaled->format->BytesPerPixel;
    Uint32 pixel = SDL_MapRGB(unscaled->format, r, g, b);
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8*)unscaled->pixels + y * unscaled->pitch + x * bpp;

    switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
    SDL_UnlockSurface(unscaled);
}


void Screen::draw(int x, int y, SDL_Surface *tile)
{
    SDL_Rect src = { 0, 0, tile->w, tile->h };
    SDL_Rect u_dst = { x, y, tile->w, tile->h };
    SDL_Rect dst = { this->doScale(x), this->doScale(y), this->doScale(tile->w), this->doScale(tile->h) };
    
    SDL_BlitSurface(tile, &src, unscaled, &u_dst);
    SDL_SoftStretch(unscaled, &u_dst, screen, &dst);
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
    SDL_Surface *tile = loadImage(name);
    SDL_Rect src = { 0, 0, tile->w, tile->h };
    SDL_Rect dst = { 0, 0, tile->w, tile->h };
    for (int y = 0; y < unscaled->h; y += tile->h)
        for (int x = 0; x < unscaled->w; x += tile->w) {
            dst.x = x;
            dst.y = y;
            SDL_BlitSurface(tile, &src, unscaled, &dst);
        }
    SDL_FreeSurface(tile);
}

SDL_PixelFormat* Screen::getFormat()
{
    return screen->format;
}

void Screen::setClipRect(SDL_Rect* rect)
{
  SDL_SetClipRect(screen, rect);
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

void Screen::setScale(bool isScaleUp)
{
    if (scaleUp != isScaleUp)
    {
      scaleUp = isScaleUp;
      if (!fullScreen)
      {
          applyMode();
      }
    }
}
