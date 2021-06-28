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


#include "game.h"

#include "descr.h"
#include "font.h"
#include "horhints.h"
#include "main.h"
#include "messages.h"
#include "opensave.h"
#include "options.h"
#include "puzzle.h"
#include "sound.h"
#include "topscores.h"
#include "utils.h"
#include "verthints.h"
#include "widgets.h"


class GameBackground: public Area
{
    public:
        GameBackground();
        void draw() override;
};

GameBackground::GameBackground()
{
    add(new Picture(8, 10, L"title.bmp"));
    add(new ManagedLabel(L"nova.ttf", 28, 20, 20, 255, 255, 0, msg(L"einsteinPuzzle"), true));
}

void GameBackground::draw()
{
    // draw background
    screen.drawWallpaper(L"rain.bmp");
    
    Area::draw();
}


class ToggleHintCommand: public Command
{
    private:
        VertHints *verHints;
        HorHints *horHints;

    public:
        ToggleHintCommand(VertHints *v, HorHints *h) {
            verHints = v;
            horHints = h;
        }
        
        void doAction() override {
            verHints->toggleExcluded();
            horHints->toggleExcluded();
        }
};


class Watch: public TimerHandler, public Widget
{
    private:
        Uint32 lastRun;
        Uint32 elapsed;
        bool stoped;
        int lastUpdate;
        Font *font;
    
    public:
        Watch();
        explicit Watch(std::istream &stream);
        virtual ~Watch();

    public:
        void onTimer() override;
        void stop();
        void start();
        void draw() override;
        int getElapsed() { return elapsed; }
        void save(std::ostream &stream);
        void reset();
};


Watch::Watch()
{
    lastRun = elapsed = lastUpdate = 0;
    stop();
    font = new Font(L"luximb.ttf", 16);
}

Watch::Watch(std::istream &stream)
{
    elapsed = readInt(stream);
    lastUpdate = 0;
    stop();
    font = new Font(L"luximb.ttf", 16);
}

Watch::~Watch()
{
    delete font;
}

void Watch::onTimer()
{
    if (stoped)
        return;
    
    const Uint32 now = SDL_GetTicks();
    elapsed += now - lastRun;
    lastRun = now;

    const int seconds = elapsed / 1000;
    if (seconds != lastUpdate)
        draw();
}

void Watch::stop()
{
    stoped = true;
}

void Watch::start()
{
    stoped = false;
    lastRun = SDL_GetTicks();
}

void Watch::draw()
{
    const int time = elapsed / 1000;
    const std::wstring s = secToStr(time);
    
    const int x = 700;
    const int y = 24;
    int w, h;
    font->getSize(s, w, h);
    SDL_Rect rect = { scaleUp(x-2), scaleUp(y-2), w+scaleUp(4), h+scaleUp(4) };
    SDL_FillRect(screen.getSurface(), &rect, 
            SDL_MapRGB(screen.getSurface()->format, 0, 0, 255));
    font->draw(x, y, 255,255,255, true, s);
    screen.addRegionToUpdate(x-2, y-2, w+4, h+4);
    
    lastUpdate = time;
}

void Watch::save(std::ostream &stream)
{
    writeInt(stream, elapsed);
}

void Watch::reset()
{
    elapsed = lastUpdate = 0;
    lastRun = SDL_GetTicks();
}


class PauseGameCommand: public Command
{
    private:
        Area *gameArea;
        Watch *watch;
        Widget *background;

    public:
        PauseGameCommand(Area *a, Watch *w, Widget *bg) { 
            gameArea = a; 
            watch = w;
            background = bg;
        }
        
        void doAction() override {
            watch->stop();
            Area area;
            area.add(background, false);
            Font font(L"laudcn2.ttf", 16);
            area.add(new Window(280, 275, 240, 50, L"greenpattern.bmp", 6));
            area.add(new Label(&font, 280, 275, 240, 50, Label::ALIGN_CENTER,
                Label::ALIGN_MIDDLE, 255,255,0, msg(L"paused")));
            area.add(new AnyKeyAccel());
            area.run();
            sound->play(L"click.wav");
            gameArea->updateMouse();
            gameArea->draw();
            watch->start();
        }
};


class WinCommand: public Command
{
    private:
        Area *gameArea;
        Watch *watch;
        Game *game;

    public:
        WinCommand(Area *a, Watch *w, Game *g) { 
            gameArea = a; 
            watch = w;
            game = g;
        }
        
        void doAction() override {
            sound->play(L"applause.wav");
            watch->stop();
            Font font(L"laudcn2.ttf", 20);
            showMessageWindow(gameArea, L"marble1.bmp", 
                    500, 70, &font, 255,0,0, msg(L"won"));
            gameArea->draw();
            TopScores scores;
            const int score = watch->getElapsed() / 1000;
            int pos = -1;
            if (! game->isHinted()) {
                if ((! scores.isFull()) || (score < scores.getMaxScore())) {
                    const std::wstring name = enterNameDialog(gameArea);
                    pos = scores.add(name, score);
                }
            }
            showScoresWindow(gameArea, &scores, pos);
            gameArea->finishEventLoop();
        }
};

class OkDlgCommand: public Command
{
    private:
        bool &res;
        Area *area;

    public:
        OkDlgCommand(Area *a, bool &r): res(r) { 
            area = a; 
        }
        
        void doAction() override {
            res = true; 
            area->finishEventLoop();
        }
};

class FailCommand: public Command
{
    private:
        Area *gameArea;
        Game *game;

    public:
        FailCommand(Area *a, Game *g) { gameArea = a;  game = g; }
        
        void doAction() override {
            sound->play(L"glasbk2.wav");
            bool restart = false;
            bool newGame = false;
            Font font(L"laudcn2.ttf", 24);
            Font btnFont(L"laudcn2.ttf", 14);
            Area area;
            area.add(gameArea);
            area.add(new Window(220, 240, 360, 140, L"redpattern.bmp", 6));
            area.add(new Label(&font, 250, 230, 300, 100, Label::ALIGN_CENTER,
                        Label::ALIGN_MIDDLE, 255,255,0, msg(L"lost")));
            OkDlgCommand newGameCmd(&area, newGame);
            area.add(new Button(250, 340, 90, 25, &btnFont, 255,255,0, 
                        L"redpattern.bmp", msg(L"startNew"), &newGameCmd));
            OkDlgCommand restartCmd(&area, restart);
            area.add(new Button(350, 340, 90, 25, &btnFont, 255,255,0, 
                        L"redpattern.bmp", msg(L"tryAgain"), &restartCmd));
            ExitCommand exitCmd(area);
            area.add(new Button(450, 340, 90, 25, &btnFont, 255,255,0, 
                        L"redpattern.bmp", msg(L"exit"), &exitCmd));
            area.run();
            if (restart || newGame) {
                if (newGame)
                    game->newGame();
                else
                    game->restart();
                gameArea->draw();
                gameArea->updateMouse();
            } else
                gameArea->finishEventLoop();
        }
};


class CheatAccel: public Widget
{
    private:
        Command *command;
        std::wstring typed;
        std::wstring cheat;

    public:
        CheatAccel(const std::wstring& s, Command *cmd): cheat(s) {
            command = cmd;
        }

    public:
        bool onKeyDown(SDLKey key, unsigned char ch) override {
            if ((key >= SDLK_a) && (key <= SDLK_z)) {
                const wchar_t s = L'a' + key - SDLK_a;
                typed += s;
                if (typed.length() == cheat.length()) {
                    if (command && (typed == cheat))
                        command->doAction();
                } else {
                    const int pos = typed.length() - 1;
                    if (typed[pos] == cheat[pos])
                        return false;
                }
            }
            if (typed.length() > 0) 
                typed = L"";
            return false;
        }
};


class CheatCommand: public Command
{
    private:
        Area *gameArea;

    public:
        explicit CheatCommand(Area *a) { gameArea = a; }
        
        void doAction() override {
            Font font(L"nova.ttf", 30);
            showMessageWindow(gameArea, L"darkpattern.bmp", 
                    500, 100, &font, 255,255,255, 
                    msg(L"iddqd"));
            gameArea->draw();
        }
};


class SaveGameCommand: public Command
{
    private:
        Area *gameArea;
        Watch *watch;
        Widget *background;
        Game *game;

    public:
        SaveGameCommand(Area *a, Watch *w, Widget *bg, Game *g) { 
            gameArea = a; 
            watch = w;
            background = bg;
            game = g;
        }
        
        void doAction() override {
            watch->stop();

            Area area;
            area.add(background, false);
            saveGame(&area, game);
            
            gameArea->updateMouse();
            gameArea->draw();
            watch->start();
        }
};


class GameOptionsCommand: public Command
{
    private:
        Area *gameArea;

    public:
        explicit GameOptionsCommand(Area *a) { 
            gameArea = a; 
        }
        
        void doAction() override {
            showOptionsWindow(gameArea);
            gameArea->updateMouse();
            gameArea->draw();
        }
};


class HelpCommand: public Command
{
    private:
        Area *gameArea;
        Watch *watch;
        Widget *background;

    public:
        HelpCommand(Area *a, Watch *w, Widget *b) { 
            gameArea = a;
            watch = w;
            background = b;
        }
        
        void doAction() override {
            watch->stop();
            Area area;
            area.add(background, false);
            area.draw();
            showDescription(&area);
            gameArea->updateMouse();
            gameArea->draw();
            watch->start();
        }
};



Game::Game()
{
    genPuzzle();

    possibilities = new Possibilities();
    openInitial(*possibilities, rules);
    
    puzzle = new Puzzle(iconSet, solvedPuzzle, possibilities);
    verHints = new VertHints(iconSet, rules);
    horHints = new HorHints(iconSet, rules);
    watch = new Watch();
}

Game::Game(std::istream &stream)
{
    pleaseWait();

    loadPuzzle(solvedPuzzle, stream);
    loadRules(rules, stream);
    memcpy(savedSolvedPuzzle, solvedPuzzle, sizeof(solvedPuzzle));
    savedRules = rules;
    possibilities = new Possibilities(stream);
    puzzle = new Puzzle(iconSet, solvedPuzzle, possibilities);
    verHints = new VertHints(iconSet, rules, stream);
    horHints = new HorHints(iconSet, rules, stream);
    watch = new Watch(stream);
    hinted = true;
}

Game::~Game()
{
    delete watch;
    delete possibilities;
    delete verHints;
    delete horHints;
    delete puzzle;
    deleteRules();
}

void Game::save(std::ostream &stream)
{
    savePuzzle(solvedPuzzle, stream);
    saveRules(rules, stream);
    possibilities->save(stream);
    verHints->save(stream);
    horHints->save(stream);
    watch->save(stream);
}

void Game::deleteRules()
{
    for (auto& rule : rules)
        delete rule;
    rules.clear();
}

void Game::pleaseWait()
{
    screen.drawWallpaper(L"rain.bmp");
    Window window(230, 260, 340, 80, L"greenpattern.bmp", 6);
    window.draw();
    Font font(L"laudcn2.ttf", 16);
    Label label(&font, 280, 275, 240, 50, Label::ALIGN_CENTER,
                Label::ALIGN_MIDDLE, 255,255,0, msg(L"loading"));
    label.draw();
    screen.addRegionToUpdate(0, 0, screen.getWidth(), screen.getHeight());
    screen.flush();
}

void Game::genPuzzle()
{
    pleaseWait();
    
    int horRules, verRules;
    do {
        if (!rules.empty())
            deleteRules();
        ::genPuzzle(solvedPuzzle, rules);
        getHintsQty(rules, verRules, horRules);
    } while ((horRules > 24) || (verRules > 15));

    memcpy(savedSolvedPuzzle, solvedPuzzle, sizeof(solvedPuzzle));
    savedRules = rules;
    
    hinted = false;
}

void Game::resetVisuals()
{
    possibilities->reset();
    openInitial(*possibilities, rules);
    puzzle->reset();
    verHints->reset(rules);
    horHints->reset(rules);
    watch->reset();
}

void Game::newGame()
{
    genPuzzle();
    resetVisuals();
}

void Game::restart()
{
    memcpy(solvedPuzzle, savedSolvedPuzzle, sizeof(solvedPuzzle));
    rules = savedRules;
    
    resetVisuals();
    hinted = true;
}

#define BUTTON(x, y, text, cmd) \
    area.add(new Button(x, y, 94, 30, &btnFont, 255,255,0, \
                L"btn.bmp", msg(text), false, cmd));

void Game::run()
{
    Area area;
    Font btnFont(L"laudcn2.ttf", 14);
    
    area.setTimer(300, watch);

    GameBackground *background = new GameBackground();
    area.add(background);
    CheatCommand cheatCmd(&area);
    area.add(new CheatAccel(L"iddqd", &cheatCmd));
    WinCommand winCmd(&area, watch, this);
    FailCommand failCmd(&area, this);
    puzzle->setCommands(&winCmd, &failCmd);
    area.add(puzzle, false);
    area.add(verHints, false);
    area.add(horHints, false);
    
    PauseGameCommand pauseGameCmd(&area, watch, background);
    BUTTON(12, 400, L"pause", &pauseGameCmd)
    area.add(new KeyAccel(SDLK_SPACE, &pauseGameCmd));
    ToggleHintCommand toggleHintsCmd(verHints, horHints);
    BUTTON(119, 400, L"switch", &toggleHintsCmd)
    SaveGameCommand saveCmd(&area, watch, background, this);
    BUTTON(12, 440, L"save", &saveCmd)
    GameOptionsCommand optionsCmd(&area);
    BUTTON(119, 440, L"options", &optionsCmd)
    ExitCommand exitGameCmd(area);
    BUTTON(226, 400, L"exit", &exitGameCmd)
    area.add(new KeyAccel(SDLK_ESCAPE, &exitGameCmd));
    HelpCommand helpCmd(&area, watch, background);
    BUTTON(226, 440, L"help", &helpCmd)
    area.add(watch, false);

    watch->start();
    area.run();
}

