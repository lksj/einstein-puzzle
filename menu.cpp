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



class MenuBackground: public Widget
{
    public:
        virtual void draw();
};


void MenuBackground::draw()
{
    SDL_Surface *title = loadImage(L"nova.bmp");
    screen.draw(0, 0, title);
    SDL_FreeSurface(title);
    Font font(L"nova.ttf", 28);
    std::wstring s(msg(L"einsteinFlowix"));
    int width = font.getWidth(s);
    font.draw((screen.getWidth() - width) / 2, 30, 255,255,255, true, s);
    Font urlFont(L"luximb.ttf", 16);
    s = L"http://games.flowix.com";
    width = urlFont.getWidth(s);
    urlFont.draw((screen.getWidth() - width) / 2, 60, 255,255,0, true, s);
    screen.addRegionToUpdate(0, 0, screen.getWidth(), screen.getHeight());
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

