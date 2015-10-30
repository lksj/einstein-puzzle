#ifndef __UTILS_H__
#define __UTILS_H__

#include <SDL.h>
#include <string>
#ifdef WIN32
#include <sys/time.h>
#endif
#include <iostream>
#include "resources.h"
#include "widgets.h"



SDL_Surface* loadImage(const std::wstring &name, bool transparent=false);
SDL_Surface* adjustBrightness(SDL_Surface *image, double k, bool transparent=false);
int gettimeofday(struct timeval* tp);
void drawWallpaper(const std::wstring &name);
void showWindow(Area *area, const std::wstring &fileName);
bool isInRect(int evX, int evY, int x, int y, int w, int h);
std::wstring numToStr(int no);
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
void ensureDirExists(const std::wstring &fileName);
int readInt(std::istream &stream);
std::wstring readString(std::istream &stream);
void writeInt(std::ostream &stream, int value);
void writeString(std::ostream &stream, const std::wstring &value);

/// Read 4-bytes integer from memory.
int readInt(unsigned char *buffer);


#endif

