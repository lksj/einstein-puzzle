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


#include <stdlib.h>
#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_ttf.h>
#include "main.h"
#include "utils.h"
#include "storage.h"
#include "unicode.h"
#include "messages.h"
#include "sound.h"


Screen screen;
Random rndGen;


static void initScreen()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        throw Exception(std::wstring(L"Error initializing SDL: ") + 
                fromMbcs(SDL_GetError()));
    atexit(SDL_Quit);
    if (TTF_Init())
        throw Exception(L"Error initializing font engine");
    screen.setMode(getStorage()->get(L"fullscreen", 1) != 0);
    screen.setSize(getStorage()->get(L"screenSize", 1));
    screen.initCursors();
    
    SDL_Surface *mouse = loadImage(L"cursor.bmp");
    SDL_SetColorKey(mouse, SDL_SRCCOLORKEY, SDL_MapRGB(mouse->format, 0, 0, 0));
    screen.setMouseImage(mouse);
    SDL_FreeSurface(mouse);
    SDL_WM_SetCaption("Einstein", NULL);

#ifdef __APPLE__
    screen.setCursor(false);
#else
    screen.setCursor(getStorage()->get(L"niceCursor", 1));
#endif
}

static void initAudio()
{
    sound = new Sound();
    sound->setVolume((float)getStorage()->get(L"volume", 20) / 100.0f);
}


#ifdef __APPLE__
static std::wstring getResourcesPath(const std::wstring& path)
{
    int idx = path.find_last_of(L'/');
    return path.substr(0, idx) + L"/../../"; 
}
#endif

static void loadResources(const std::wstring &selfPath)
{
    StringList dirs;
#ifdef WIN32
    dirs.push_back(getStorage()->get(L"path", L"") + L"\\res"); 
#else
#ifdef __APPLE__
    dirs.push_back(getResourcesPath(selfPath));
#else
    dirs.push_back("/usr" L"/share/einstein/res");
    dirs.push_back(fromMbcs(getenv("HOME")) + L"/.einstein/res");
#endif
#endif
    dirs.push_back(L"res");
    dirs.push_back(L".");
    resources = new ResourcesCollection(dirs);
    msg.load();
}


/*static void checkBetaExpire()
{
    if (1124832535L + 60L*60L*24L*40L < time(NULL)) {
        Font font(L"laudcn2.ttf", 16);
        Area area;
        showMessageWindow(&area, L"darkpattern.bmp", 
                700, 100, &font, 255,255,255, 
                msg(L"expired"));
    }
}*/



int main(int argc, char *argv[])
{
#ifndef WIN32
    ensureDirExists(fromMbcs(getenv("HOME")) + std::wstring(L"/.einstein"));
#endif
    
    try {
        loadResources(fromUtf8(argv[0]));
        initScreen();
        initAudio();
//        checkBetaExpire();
        menu();
        getStorage()->flush();
    } catch (Exception &e) {
        std::cerr << L"ERROR: " << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << L"ERROR: Unknown exception" << std::endl;
    }
    screen.doneCursors();
    
    return 0;
}

