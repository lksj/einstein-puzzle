#include <string.h>
#include "iconset.h"
#include "utils.h"


IconSet::IconSet()
{
    std::wstring buf = L"xy.bmp";
    
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 6; j++) {
            buf[1] = L'1' + j;
            buf[0] = L'a' + i;
            smallIcons[i][j][0] = loadImage(buf);
            smallIcons[i][j][1] = adjustBrightness(smallIcons[i][j][0], 1.5, false);
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
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 6; j++)
            for (int k = 0; k < 2; k++) {
                SDL_FreeSurface(smallIcons[i][j][k]);
                SDL_FreeSurface(largeIcons[i][j][k]);
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

SDL_Surface* IconSet::getSmallIcon(int row, int num, bool h)
{
    return smallIcons[row][num-1][h ? 1 : 0];
}

