// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

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


#ifndef __SOUND_H__
#define __SOUND_H__


#include <map>
#include <string>
#include <SDL/SDL_mixer.h>


class Sound
{
    private:
        bool disabled;
        
        typedef std::map<std::wstring, Mix_Chunk*> ChunkMap;
        ChunkMap chunkCache;

        bool enableFx;
        float volume;
    
    public:
        Sound();
        ~Sound();

    public:
        void play(const std::wstring &name);
        void setVolume(float volume);
};


extern Sound *sound;


#endif

