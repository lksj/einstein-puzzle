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


#ifndef __ICONSET_H__
#define __ICONSET_H__


#include <SDL/SDL.h>


class IconSet
{
    private:
        SDL_Surface *largeIcons[6][6][2];
        SDL_Surface *emptyFieldIcon, *emptyHintIcon, *nearHintIcon[2];
        SDL_Surface *sideHintIcon[2], *betweenArrow[2];
    
    public:
        IconSet();
        virtual ~IconSet();

    public:
        SDL_Surface* getLargeIcon(int row, int num, bool highlighted);
        SDL_Surface* getEmptyFieldIcon() { return emptyFieldIcon; };
        SDL_Surface* getEmptyHintIcon() { return emptyHintIcon; };
        SDL_Surface* getNearHintIcon(bool h) { return nearHintIcon[h ? 1 : 0]; };
        SDL_Surface* getSideHintIcon(bool h) { return sideHintIcon[h ? 1 : 0]; };
        SDL_Surface* getBetweenArrow(bool h) { return betweenArrow[h ? 1 : 0]; };
};


#endif

