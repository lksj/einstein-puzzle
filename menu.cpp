// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-05-06 by Jordan Evens <jordan.evens@gmail.com>

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


#include <vector>
#include "main.h"
#include "utils.h"
#include "widgets.h"
#include "topscores.h"
#include "opensave.h"
#include "game.h"
#include "descr.h"
#include "options.h"
#include "messages.h"



class MenuBackground: public Area
{
    public:
        MenuBackground();
};


MenuBackground::MenuBackground()
{
    add(new Picture(0, 0, L"nova.bmp"));
    std::wstring s(msg(L"einsteinFlowix"));
    add(new ManagedLabel(L"nova.ttf", 28, 0, 30, screen.getWidth(), 0,
                            Label::ALIGN_CENTER, Label::ALIGN_TOP, 255, 255, 255, s));
    
    s = L"http://games.flowix.com";
    add(new ManagedLabel(L"luximb.ttf", 16, 0, 60, screen.getWidth(), 0,
                            Label::ALIGN_CENTER, Label::ALIGN_TOP, 255, 255, 0, s));
}


class NewGameCommand: public Command
{
    private:
        Area *area;
    
    public:
        NewGameCommand(Area *a) { area = a; };

        virtual void doAction() {
            Game game;
            game.run();
            area->updateMouse();
            area->draw();
        };
};


class LoadGameCommand: public Command
{
    private:
        Area *area;
    
    public:
        LoadGameCommand(Area *a) { area = a; };

        virtual void doAction() {
            Game *game = loadGame(area);
            if (game) {
                game->run();
                delete game;
            }
            area->updateMouse();
            area->draw();
        };
};



class TopScoresCommand: public Command
{
    private:
        Area *area;
    
    public:
        TopScoresCommand(Area *a) { area = a; };

        virtual void doAction() {
            TopScores scores;
            showScoresWindow(area, &scores);
            area->updateMouse();
            area->draw();
        };
};


class RulesCommand: public Command
{
    private:
        Area *area;
    
    public:
        RulesCommand(Area *a) { area = a; };

        virtual void doAction() {
            showDescription(area);
            area->updateMouse();
            area->draw();
        };
};


class OptionsCommand: public Command
{
    private:
        Area *area;
    
    public:
        OptionsCommand(Area *a) { area = a; };

        virtual void doAction() {
            showOptionsWindow(area);
            area->updateMouse();
            area->draw();
        };
};


class AboutCommand: public Command
{
    private:
        Area *parentArea;
    
    public:
        AboutCommand(Area *a) { parentArea = a; };

        virtual void doAction() {
            Area area;
            Font titleFont(L"nova.ttf", 26);
            Font font(L"laudcn2.ttf", 14);
            Font urlFont(L"luximb.ttf", 16);

#define LABEL(pos, c, f, text) area.add(new Label(&f, 220, pos, 360, 20, \
            Label::ALIGN_CENTER, Label::ALIGN_MIDDLE, 255,255,c, text));
            area.add(parentArea);
            area.add(new Window(220, 160, 360, 280, L"blue.bmp"));
            area.add(new Label(&titleFont, 250, 165, 300, 40, Label::ALIGN_CENTER,
                        Label::ALIGN_MIDDLE, 255,255,0, msg(L"about")));
            LABEL(240, 255, font, msg(L"einsteinPuzzle"))
            LABEL(260, 255, font, msg(L"version"))
            LABEL(280, 255, font, msg(L"copyright"))
            LABEL(330, 0, urlFont, L"http://games.flowix.com")
#undef LABEL
            ExitCommand exitCmd(area);
            area.add(new Button(360, 400, 80, 25, &font, 255,255,0, L"blue.bmp", 
                        msg(L"ok"), &exitCmd));
            area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
            area.add(new KeyAccel(SDLK_RETURN, &exitCmd));
            area.run();

            parentArea->updateMouse();
            parentArea->draw();
        };
};


static Button* menuButton(int y, Font *font, const std::wstring &text, 
        Command *cmd=NULL)
{
    return new Button(550, y, 220, 30, font, 0,240,240, 30,255,255, text, cmd);
}


void menu()
{
    Area area;
    Font font(L"laudcn2.ttf", 20);

    area.add(new MenuBackground());
    area.draw();
        
    NewGameCommand newGameCmd(&area);
    area.add(menuButton(340, &font, msg(L"newGame"), &newGameCmd));
    LoadGameCommand loadGameCmd(&area);
    area.add(menuButton(370, &font, msg(L"loadGame"), &loadGameCmd));
    TopScoresCommand topScoresCmd(&area);
    area.add(menuButton(400, &font, msg(L"topScores"), &topScoresCmd));
    RulesCommand rulesCmd(&area);
    area.add(menuButton(430, &font, msg(L"rules"), &rulesCmd));
    OptionsCommand optionsCmd(&area);
    area.add(menuButton(460, &font, msg(L"options"), &optionsCmd));
    AboutCommand aboutCmd(&area);
    area.add(menuButton(490, &font, msg(L"about"), &aboutCmd));
    ExitCommand exitMenuCmd(area);
    area.add(menuButton(520, &font, msg(L"exit"), &exitMenuCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitMenuCmd));
    
    area.draw();
    screen.addRegionToUpdate(0, 0, screen.getWidth(), screen.getHeight());
    screen.flush();

    area.run();
}

