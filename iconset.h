#ifndef __ICONSET_H__
#define __ICONSET_H__


#include <SDL.h>


class IconSet
{
    private:
        SDL_Surface *smallIcons[6][6][2];
        SDL_Surface *largeIcons[6][6][2];
        SDL_Surface *emptyFieldIcon, *emptyHintIcon, *nearHintIcon[2];
        SDL_Surface *sideHintIcon[2], *betweenArrow[2];
    
    public:
        IconSet();
        virtual ~IconSet();

    public:
        SDL_Surface* getLargeIcon(int row, int num, bool highlighted);
        SDL_Surface* getSmallIcon(int row, int num, bool highlighted);
        SDL_Surface* getEmptyFieldIcon() { return emptyFieldIcon; };
        SDL_Surface* getEmptyHintIcon() { return emptyHintIcon; };
        SDL_Surface* getNearHintIcon(bool h) { return nearHintIcon[h ? 1 : 0]; };
        SDL_Surface* getSideHintIcon(bool h) { return sideHintIcon[h ? 1 : 0]; };
        SDL_Surface* getBetweenArrow(bool h) { return betweenArrow[h ? 1 : 0]; };
};


#endif

