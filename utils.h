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


#ifndef __UTILS_H__
#define __UTILS_H__

#include <SDL/SDL.h>
#include <string>
#ifdef WIN32
#include <sys/time.h>
#endif
#include <iostream>
#include "resources.h"
#include "widgets.h"



SDL_Surface* scaleUp(SDL_Surface* tile);
SDL_Surface* scaleDown(SDL_Surface* tile);
void blitDraw(int x, int y, SDL_Surface *src, SDL_Surface *dst);
void drawTiled(const std::wstring &name, SDL_Surface *s);
SDL_Surface* makeSWSurface(int width, int height);
SDL_Surface* loadImage(const std::wstring &name, bool transparent=false, bool scaled = false);
SDL_Surface* adjustBrightness(SDL_Surface *image, double k, bool transparent=false);
int gettimeofday(struct timeval* tp);
bool isInRect(int evX, int evY, int x, int y, int w, int h);
std::wstring numToStr(int no);
int adjustBrightness(int i, double k);
void adjustBrightness(int *r, int *g, int *b, double k);
void adjustBrightness(SDL_Surface *image, int x, int y, double k);
std::wstring secToStr(int time);
void showMessageWindow(Area *area, const std::wstring &pattern, 
        int width, int height, Font *font, int r, int g, int b,
        const std::wstring &msg);
int getCornerPixel(SDL_Surface *surface);
void getPixel(SDL_Surface *surface, int x, int y, 
        Uint8 *r, Uint8 *g, Uint8 *b);
void setPixel(SDL_Surface *s, int x, int y, int r, int g, int b);
void drawBevel(SDL_Surface *s, int left, int top, int width, int height,
        bool raised, int size);
SDL_Surface* makeBox(int width, int height, const std::wstring &bg);
void ensureDirExists(const std::wstring &fileName);
int readInt(std::istream &stream);
std::wstring readString(std::istream &stream);
void writeInt(std::ostream &stream, int value);
void writeString(std::ostream &stream, const std::wstring &value);

/// Read 4-bytes integer from memory.
int readInt(unsigned char *buffer);


#endif

