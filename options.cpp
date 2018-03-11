// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2018-02-10 by Jordan Evens <jordan.evens@gmail.com>

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


#include "options.h"
#include "storage.h"
#include "main.h"
#include "messages.h"
#include "sound.h"
#include "utils.h"

class CancelCommand : public ExitCommand
{
    public:
        explicit CancelCommand(Area& a)
            : ExitCommand(a)
        {
        }

        void doAction() override
        {
            // need to make sure that we revert any changes that we made
            float volume = ((float)getStorage()->get(L"volume", 20)) / 100.0f;
            sound->setVolume(volume);
            ExitCommand::doAction();
        }
};

class OptionsChangedCommand: public Command
{
    private:
        bool &fullscreen;
        bool &niceCursor;
        int &screenSize;
        float &volume;
        Area *area;
    
    public:
        OptionsChangedCommand(Area *a, bool &fs, bool &ns, int &ss, float &v): 
            fullscreen(fs), niceCursor(ns), screenSize(ss), volume(v) {
            area = a;
        }

        void doAction() override {
            bool oldFullscreen = (getStorage()->get(L"fullscreen", 1) != 0);
            bool oldCursor = (getStorage()->get(L"niceCursor", 1) != 0);
            int oldSize = ((float)getStorage()->get(L"screenSize", 1));
            float oldVolume = (float)getStorage()->get(L"volume", 20) / 100.0f;
            if (fullscreen != oldFullscreen) {
                getStorage()->set(L"fullscreen", fullscreen);
                screen.setMode(fullscreen);
            }
#ifndef __APPLE__
            if (niceCursor != oldCursor) {
                getStorage()->set(L"niceCursor", niceCursor);
                screen.setCursor(niceCursor);
            }
#endif
            if (screenSize != oldSize) {
                getStorage()->set(L"screenSize", (int)screenSize);
                screen.setSize(screenSize);
            }
            if (volume != oldVolume) {
                getStorage()->set(L"volume", (int)(volume * 100.0f));
                sound->setVolume(volume);
            }
            getStorage()->flush();
            area->finishEventLoop();
        }
};


#define LABEL(y, s) \
    area.add(new Label(&font, 300, y, 300, 20, Label::ALIGN_LEFT, \
                Label::ALIGN_MIDDLE, 255,255,255, msg(s)));
#define CHECKBOX(y, var) \
    area.add(new Checkbox(265, y, 20, 20, &font, 255,255,255, L"blue.bmp", \
                var));
#define OPTION(y, s, var) LABEL(y, s) CHECKBOX(y, var)

//This class exists solely so that the click noise for the OK & Cancel button
//will be played at the volume that is set in the Slider
class OptionsButton: public Button
{  
    public:
        OptionsButton(int x, int y, int w, int h, Font *font, 
                                    int r, int g, int b, const std::wstring &bg, 
                                    const std::wstring &text, Command *cmd):
            Button(x, y, w, h, font, r, g, b, bg, text, cmd)
        {
        }
        
        void doClick() override
        {
            handleClick();
            sound->play(L"click.wav");
        }
};

//This class exists solely so that the click noise for the OK button
//will be played at the volume that is set in the Slider
class VolumeSlider: public Slider
{
    public:
        VolumeSlider(int x, int y, int w, int h, float &v)
            : Slider(x, y, w, h, v)
        {
        }
        
        void changeValue(float v) override
        {
            // set volume right away so we can tell what we set it to
            sound->setVolume(v);
            Slider::changeValue(v);
        }
};

void showOptionsWindow(Area *parentArea)
{
    Font titleFont(L"nova.ttf", 26);
    Font font(L"laudcn2.ttf", 14);

    bool fullscreen = (getStorage()->get(L"fullscreen", 1) != 0);
    bool niceCursor = (getStorage()->get(L"niceCursor", 1) != 0);
    int screenSize = ((int)getStorage()->get(L"screenSize", 1));
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
    area.add(new Label(&font, 265, 305, 300, 18, Label::ALIGN_LEFT,
                Label::ALIGN_MIDDLE, 255,255,255, msg(L"screenSize")));
    area.add(new CycleButton(360, 305, 160, 18, &font, screenSize, screen.getModeList()));
   
    area.add(new Label(&font, 265, 330, 300, 20, Label::ALIGN_LEFT,
                Label::ALIGN_MIDDLE, 255,255,255, msg(L"volume")));
    area.add(new VolumeSlider(360, 332, 160, 16, volume));
    
    CancelCommand exitCmd(area);
    OptionsChangedCommand okCmd(&area, fullscreen, niceCursor, screenSize, volume);
    area.add(new OptionsButton(315, 390, 85, 25, &font, 255,255,0, L"blue.bmp",
                msg(L"ok"), &okCmd));
    area.add(new OptionsButton(405, 390, 85, 25, &font, 255,255,0, L"blue.bmp",
                msg(L"cancel"), &exitCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
    area.add(new KeyAccel(SDLK_RETURN, &okCmd));
    area.run();
}

