#include "sound.h"

#include <iostream>
#include <SDL_events.h>
#include "resources.h"


Sound *sound;


Sound::Sound()
{
    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
    int audio_channels = 2;
    int audio_buffers = 1024;
    disabled = Mix_OpenAudio(audio_rate, audio_format, audio_channels, 
                audio_buffers);
    if (disabled)
        std::cout << "Audio is disabled" << std::endl;
}

Sound::~Sound()
{
    if (! disabled)
        Mix_CloseAudio();
    for (ChunkMap::iterator i = chunkCache.begin(); i != chunkCache.end(); i++)
        Mix_FreeChunk((*i).second);
    Mix_CloseAudio();
}


void Sound::play(const std::wstring &name)
{
    if (disabled || (! enableFx))
        return;
    
    Mix_Chunk *chunk = NULL;
    
    ChunkMap::iterator i = chunkCache.find(name);
    if (i != chunkCache.end())
        chunk = (*i).second;
    else {
        ResDataHolder data(name);
        chunk = Mix_LoadWAV_RW(SDL_RWFromMem(data.getData(), data.getSize()), 
                0);
        chunkCache[name] = chunk;
    }

    if (chunk) {
        Mix_VolumeChunk(chunk, (int)(volume * 128.0f));
        Mix_PlayChannel(-1, chunk, 0);
    }
    SDL_PumpEvents();
}

void Sound::setVolume(float v)
{
    volume = v;
    enableFx = 0.01 < volume;
}

