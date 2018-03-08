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


#include <string.h>
#include "iconset.h"
#include "utils.h"


IconSet::IconSet()
{
    std::wstring buf = L"xy.bmp";
    
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 6; j++) {
            buf[1] = L'1' + j;
            buf[0] = L'A' + i;
            largeIcons[i][j][0] = loadImage(buf);
            largeIcons[i][j][1] = adjustBrightness(largeIcons[i][j][0], 1.5, false);
        }
    emptyFieldIcon = loadImage(L"tile.bmp");
    emptyHintIcon = loadImage(L"hint-tile.bmp");
    nearHintIcon[0] = loadImage(L"hint-near.bmp");
    nearHintIcon[1] = adjustBrightness(nearHintIcon[0], 1.5, false);
    sideHintIcon[0] = loadImage(L"hint-side.bmp");
    sideHintIcon[1] = adjustBrightness(sideHintIcon[0], 1.5, false);
    betweenArrow[0] = loadImage(L"betwarr.bmp", true);
    betweenArrow[1] = adjustBrightness(betweenArrow[0], 1.5, false);
}

IconSet::~IconSet()
{
    for (auto& largeIcon : largeIcons)
        for (auto& j : largeIcon)
            for (auto& k : j)
            {
                SDL_FreeSurface(k);
            }
    SDL_FreeSurface(emptyFieldIcon);
    SDL_FreeSurface(emptyHintIcon);
    SDL_FreeSurface(nearHintIcon[0]);
    SDL_FreeSurface(nearHintIcon[1]);
    SDL_FreeSurface(sideHintIcon[0]);
    SDL_FreeSurface(sideHintIcon[1]);
    SDL_FreeSurface(betweenArrow[0]);
    SDL_FreeSurface(betweenArrow[1]);
}

SDL_Surface* IconSet::getLargeIcon(int row, int num, bool h)
{
    return largeIcons[row][num-1][h ? 1 : 0];
}
