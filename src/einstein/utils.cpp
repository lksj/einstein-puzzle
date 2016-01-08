#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <wchar.h>

//#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#endif

#include <fstream>

#include "utils.h"
#include "main.h"
#include "unicode.h"
#include "sound.h"



int getCornerPixel(SDL_Surface *surface)
{
    SDL_LockSurface(surface);
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8*)surface->pixels;
    int pixel = 0;
    switch (bpp) {
        case 1: pixel = *p;  break;
        case 2: pixel = *(Uint16 *)p; break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixel = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case 4: pixel = *(Uint32 *)p; break;
        default: pixel = 0;       /* shouldn't happen, but avoids warnings */
    }
    SDL_UnlockSurface(surface);
    return pixel;
}



SDL_Surface* loadImage(const std::wstring &name, bool transparent)
{
    int size;
    void *bmp;

    bmp = resources->getRef(name, size);
    if (! bmp)
        throw Exception(name + L" is not found");
    SDL_RWops *op = SDL_RWFromMem(bmp, size);
    SDL_Surface *s = SDL_LoadBMP_RW(op, 0);
    SDL_FreeRW(op);
    resources->delRef(bmp);
    if (! s)
        throw Exception(L"Error loading " + name);
    SDL_Surface *screenS = SDL_DisplayFormat(s);
    SDL_FreeSurface(s);
    if (! screenS)
        throw Exception(L"Error translating to screen format " + name);
    if (transparent)
        SDL_SetColorKey(screenS, SDL_SRCCOLORKEY, getCornerPixel(screenS));
    return screenS;
}


#ifdef WIN32
#include <sys/timeb.h>
struct timezone { };

int gettimeofday(struct timeval* tp, int* /*tz*/) 
{
    struct timeb tb;
    ftime(&tb);
    tp->tv_sec = tb.time;
    tp->tv_usec = 1000*tb.millitm;
    return 0;
}

int gettimeofday(struct timeval* tp, struct timezone* /*tz*/) 
{
    return gettimeofday(tp, (int*)NULL);
}
#endif



int gettimeofday(struct timeval* tp)
{
#ifdef WIN32
    return gettimeofday(tp, (int*)NULL);
#else
    struct timezone tz;
    return gettimeofday(tp, &tz);
#endif
}

void drawWallpaper(const std::wstring &name)
{
    SDL_Surface *tile = loadImage(name);
    SDL_Rect src = { 0, 0, tile->w, tile->h };
    SDL_Rect dst = { 0, 0, tile->w, tile->h };
    for (int y = 0; y < screen.getHeight(); y += tile->h)
        for (int x = 0; x < screen.getWidth(); x += tile->w) {
            dst.x = x;
            dst.y = y;
            SDL_BlitSurface(tile, &src, screen.getSurface(), &dst);
        }
    SDL_FreeSurface(tile);
}


void setPixel(SDL_Surface *s, int x, int y, int r, int g, int b)
{
    int bpp = s->format->BytesPerPixel;
    Uint32 pixel = SDL_MapRGB(s->format, r, g, b);
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8*)s->pixels + y * s->pitch + x * bpp;

    switch (bpp) {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16 *)p = pixel;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
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
}


void getPixel(SDL_Surface *surface, int x, int y, 
        Uint8 *r, Uint8 *g, Uint8 *b)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    Uint32 pixel;
    switch (bpp) {
        case 1: pixel = *p;  break;
        case 2: pixel = *(Uint16 *)p; break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixel = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case 4: pixel = *(Uint32 *)p; break;
        default: pixel = 0;       /* shouldn't happen, but avoids warnings */
    }
    SDL_GetRGB(pixel, surface->format, r, g, b);
}


static int gammaTable[256];
static double lastGamma = -1.0;


void adjustBrightness(SDL_Surface *image, int x, int y, double k)
{
    if (lastGamma != k) {
        for (int i = 0; i <= 255; i++) {
            gammaTable[i] = (int)(255.0 * pow((double)i / 255.0, 1.0 / k) + 0.5);
            if (gammaTable[i] > 255)
                gammaTable[i] = 255;
        }
        lastGamma = k;
    }
    
    Uint8 r, g, b;
    getPixel(image, x, y, &r, &g, &b);
    setPixel(image, x, y, gammaTable[r], gammaTable[g], gammaTable[b]);
}


SDL_Surface* adjustBrightness(SDL_Surface *image, double k, bool transparent)
{
    if (lastGamma != k) {
        for (int i = 0; i <= 255; i++) {
            gammaTable[i] = (int)(255.0 * pow((double)i / 255.0, 1.0 / k) + 0.5);
            if (gammaTable[i] > 255)
                gammaTable[i] = 255;
        }
        lastGamma = k;
    }
    
    SDL_Surface *s = SDL_DisplayFormat(image);
    if (! s)
        throw Exception(L"Error converting image to display format");
    
    SDL_LockSurface(s);
    
    Uint8 r, g, b;
    for (int j = 0; j < s->h; j++)
        for (int i = 0; i < s->w; i++) {
            getPixel(s, i, j, &r, &g, &b);
            setPixel(s, i, j, gammaTable[r], gammaTable[g], gammaTable[b]);
        }
    
    SDL_UnlockSurface(s);

    if (transparent)
        SDL_SetColorKey(s, SDL_SRCCOLORKEY, getCornerPixel(s));

    return s;
}


class CenteredBitmap: public Widget
{
    private:
        SDL_Surface *tile;
        int x, y;
        
    public:
        CenteredBitmap(const std::wstring &fileName) {
            tile = loadImage(fileName);
            x = (screen.getWidth() - tile->w) / 2;
            y = (screen.getHeight() - tile->h) / 2;
        };

        virtual ~CenteredBitmap() {
            SDL_FreeSurface(tile);
        };

        virtual void draw() {
            screen.draw(x, y, tile);
            screen.addRegionToUpdate(x, y, tile->w, tile->h);
        };
};


void showWindow(Area *parentArea, const std::wstring &fileName)
{
    Area area;

    area.add(parentArea);
    area.add(new CenteredBitmap(fileName));
    area.add(new AnyKeyAccel());
    area.run();
    sound->play(L"click.wav");
}


bool isInRect(int evX, int evY, int x, int y, int w, int h)
{
    return ((evX >= x) && (evX < x + w) && (evY >= y) && (evY < y + h));
}

std::wstring secToStr(int time)
{
    int hours = time / 3600;
    int v = time - hours * 3600;
    int minutes = v / 60;
    int seconds = v - minutes * 60;

    wchar_t buf[50];
#ifdef WIN32
    swprintf(buf, L"%02i:%02i:%02i", hours, minutes, seconds);
#else
    swprintf(buf, 50, L"%02i:%02i:%02i", hours, minutes, seconds);
#endif

    return buf;
}


void showMessageWindow(Area *parentArea, const std::wstring &pattern, 
        int width, int height, Font *font, int r, int g, int b,
        const std::wstring &msg)
{
    Area area;

    int x = (screen.getWidth() - width) / 2;
    int y = (screen.getHeight() - height) / 2;
    
    area.add(parentArea);
    area.add(new Window(x, y, width, height, pattern, 6));
    area.add(new Label(font, x, y, width, height, Label::ALIGN_CENTER,
                Label::ALIGN_MIDDLE, r, g, b, msg));
    area.add(new AnyKeyAccel());
    area.run();
    sound->play(L"click.wav");
}


void drawBevel(SDL_Surface *s, int left, int top, int width, int height,
        bool raised, int size)
{
    double k, f, kAdv, fAdv;
    if (raised) {
        k = 2.6;
        f = 0.1;
        kAdv = -0.2;
        fAdv = 0.1;
    } else {
        f = 2.6;
        k = 0.1;
        fAdv = -0.2;
        kAdv = 0.1;
    }
    for (int i = 0; i < size; i++) {
        for (int j = i; j < height - i - 1; j++)
            adjustBrightness(s, left + i, top + j, k);
        for (int j = i; j < width - i; j++)
            adjustBrightness(s, left + j, top + i, k);
        for (int j = i+1; j < height - i; j++)
            adjustBrightness(s, left + width - i - 1, top + j, f);
        for (int j = i; j < width - i - 1; j++)
            adjustBrightness(s, left + j, top + height - i - 1, f);
        k += kAdv;
        f += fAdv;
    }
}

//#ifndef WIN32

void ensureDirExists(const std::wstring &fileName)
{
    std::string s(toMbcs(fileName));
    struct stat buf;
    if (! stat(s.c_str(), &buf)) {
        if (! S_ISDIR(buf.st_mode))
            unlink(s.c_str());
        else
            return;
    }
#ifndef WIN32
    mkdir(s.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#else
    mkdir(s.c_str());
#endif
}

/*#else

void ensureDirExists(const std::wstring &fileName)
{
    PORT ME!
}

#endif*/

int readInt(std::istream &stream)
{
    if (stream.fail())
        throw Exception(L"Error reading string");
    unsigned char buf[4];
    stream.read((char*)buf, 4);
    if (stream.fail())
        throw Exception(L"Error reading string");
    return buf[0] + buf[1] * 256 + buf[2] * 256 * 256 + 
        buf[3] * 256 * 256 * 256;
}


std::wstring readString(std::istream &stream)
{
    std::string str;
    char c;

    if (stream.fail())
        throw Exception(L"Error reading string");
    
    c = stream.get();
    while (c && (! stream.fail())) {
        str += c;
        c = stream.get();
    }

    if (stream.fail())
        throw Exception(L"Error reading string");

    return fromUtf8(str);
}

void writeInt(std::ostream &stream, int v)
{
    unsigned char b[4];
    int i, ib;

    for (i = 0; i < 4; i++) {
        ib = v & 0xFF;
        v = v >> 8;
        b[i] = ib;
    }
    
    stream.write((char*)&b, 4);
}

void writeString(std::ostream &stream, const std::wstring &value)
{
    std::string s(toUtf8(value));
    stream.write(s.c_str(), s.length() + 1);
}

int readInt(unsigned char *buf)
{
    return buf[0] + buf[1] * 256 + buf[2] * 256 * 256 + 
        buf[3] * 256 * 256 * 256;
}

