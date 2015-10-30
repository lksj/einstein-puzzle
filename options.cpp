#include "options.h"
#include "storage.h"
#include "main.h"
#include "messages.h"
#include "sound.h"


class OptionsChangedCommand: public Command
{
    private:
        bool &fullscreen;
        bool &niceCursor;
        float &volume;
        Area *area;
    
    public:
        OptionsChangedCommand(Area *a, bool &fs, bool &ns, float &v): 
            fullscreen(fs), niceCursor(ns), volume(v) {
            area = a;
        };

        virtual void doAction() {
            bool oldFullscreen = (getStorage()->get(L"fullscreen", 1) != 0);
            bool oldCursor = (getStorage()->get(L"niceCursor", 1) != 0);
            float oldVolume = (float)getStorage()->get(L"volume", 20) / 100.0f;
            if (fullscreen != oldFullscreen) {
                getStorage()->set(L"fullscreen", fullscreen);
                screen.setMode(VideoMode(800, 600, 24, fullscreen));
            }
#ifndef __APPLE__
            if (niceCursor != oldCursor) {
                getStorage()->set(L"niceCursor", niceCursor);
                screen.setCursor(niceCursor);
            }
#endif
            if (volume != oldVolume) {
                getStorage()->set(L"volume", (int)(volume * 100.0f));
                sound->setVolume(volume);
            }
            getStorage()->flush();
            area->finishEventLoop();
        };
};


#define LABEL(y, s) \
    area.add(new Label(&font, 300, y, 300, 20, Label::ALIGN_LEFT, \
                Label::ALIGN_MIDDLE, 255,255,255, msg(s)));
#define CHECKBOX(y, var) \
    area.add(new Checkbox(265, y, 20, 20, &font, 255,255,255, L"blue.bmp", \
                var));
#define OPTION(y, s, var) LABEL(y, s) CHECKBOX(y, var)

void showOptionsWindow(Area *parentArea)
{
    Font titleFont(L"nova.ttf", 26);
    Font font(L"laudcn2.ttf", 14);

    bool fullscreen = (getStorage()->get(L"fullscreen", 1) != 0);
    bool niceCursor = (getStorage()->get(L"niceCursor", 1) != 0);
    float volume = ((float)getStorage()->get(L"volume", 20)) / 100.0f;
    
    Area area;

    area.add(parentArea);
    area.add(new Window(250, 170, 300, 260, L"blue.bmp"));
    area.add(new Label(&titleFont, 250, 175, 300, 40, Label::ALIGN_CENTER,
                Label::ALIGN_MIDDLE, 255,255,0, msg(L"options")));
    OPTION(260, L"fullscreen", fullscreen);
#ifndef __APPLE__
    OPTION(280, L"niceCursor", niceCursor);
#endif
    
    area.add(new Label(&font, 265, 330, 300, 20, Label::ALIGN_LEFT,
                Label::ALIGN_MIDDLE, 255,255,255, msg(L"volume")));
    area.add(new Slider(360, 332, 160, 16, volume));
    
    ExitCommand exitCmd(area);
    OptionsChangedCommand okCmd(&area, fullscreen, niceCursor, volume);
    area.add(new Button(315, 390, 85, 25, &font, 255,255,0, L"blue.bmp", 
                msg(L"ok"), &okCmd));
    area.add(new Button(405, 390, 85, 25, &font, 255,255,0, L"blue.bmp", 
                msg(L"cancel"), &exitCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
    area.add(new KeyAccel(SDLK_RETURN, &okCmd));
    area.run();
}

