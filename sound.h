#ifndef __SOUND_H__
#define __SOUND_H__


#include <string>
#include <map>
#include <SDL_mixer.h>


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

