// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2018-02-11 by Jordan Evens <jordan.evens@gmail.com>

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


#include "opensave.h"

#include "convert.h"
#include "exceptions.h"
#include "messages.h"
#include "unicode.h"
#include "utils.h"
#include "widgets.h"

#include <ctime>
#include <fstream>
#ifdef WIN32
#define UNICODE
#include <shlobj.h>
#endif


#define MAX_SLOTS 10


class SavedGame
{
    private:
        std::wstring fileName;
        bool exists;
        std::wstring name;
        // prevent generation by compiler
        SavedGame& operator=(const SavedGame&);

    public:
        explicit SavedGame(const std::wstring &fileName);
        SavedGame(const SavedGame &s): fileName(s.fileName), name(s.name) {
            exists = s.exists;
        }

    public:
        const std::wstring& getFileName() { return fileName; }
        std::wstring getName() { return exists ? name : msg(L"empty"); }
        bool isExists() { return exists; }
};


SavedGame::SavedGame(const std::wstring &s): fileName(s)
{
    exists = false;
    
    try {
        std::ifstream stream(toMbcs(fileName).c_str(), std::ifstream::in | 
                std::ifstream::binary);
        if (stream.fail())
            throw Exception(L"Can't open file");
        name = readString(stream);
        stream.close();
        exists = true;
    } catch (...) { }
}



class OkCommand: public Command
{
    private:
        Area &area;
        bool *ok;
    
    public:
        OkCommand(Area &a, bool *o): area(a) { ok = o; }
        
        void doAction() override {
            *ok = true;
            area.finishEventLoop();
        }
};



class SaveCommand: public Command
{
    private:
        SavedGame &savedGame;
        Area *parentArea;
        bool *saved;
        Font *font;
        std::wstring defaultName;
        Game *game;

    public:
        SaveCommand(SavedGame &sg, Font *f, Area *area, bool *s,
                const std::wstring &dflt, Game *g): savedGame(sg), defaultName(dflt) 
        {
            parentArea = area;
            saved = s;
            font = f;
            game = g;
        }
        
    public:
        void doAction() override {
            Area area;
            area.add(parentArea, false);
            area.add(new Window(170, 280, 460, 100, L"blue.bmp"));
            std::wstring name;
            if (savedGame.isExists())
                name = savedGame.getName();
            else
                name = defaultName;
            area.add(new Label(font, 180, 300, 255,255,0, msg(L"enterGame")));
            area.add(new InputField(340, 300, 280, 26, L"blue.bmp", name, 20,  
                        255,255,0,  font));
            ExitCommand exitCmd(area);
            OkCommand okCmd(area, saved);
            area.add(new Button(310, 340, 80, 25, font, 255,255,0, L"blue.bmp", 
                        msg(L"ok"), &okCmd));
            area.add(new Button(400, 340, 80, 25, font, 255,255,0, L"blue.bmp", 
                        msg(L"cancel"), &exitCmd));
            area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
            area.add(new KeyAccel(SDLK_RETURN, &okCmd));
            area.run();

            if (*saved) {
                *saved = false;
                try {
                    std::ofstream stream(toMbcs(savedGame.getFileName()).
                        c_str(), std::ofstream::out | std::ofstream::binary);
                    if (stream.fail())
                        throw Exception(L"Error creating save file");
                    writeString(stream, name);
                    game->save(stream);
                    if (stream.fail())
                        throw Exception(L"Error saving game");
                    stream.close();
                    *saved = true;
                } catch (...) { 
                    showMessageWindow(&area, L"redpattern.bmp", 300, 80, font,
                            255,255,255, msg(L"saveError"));
                }
                parentArea->finishEventLoop();
            } else {
                parentArea->updateMouse();
                parentArea->draw();
            }
        }
};


static std::wstring getSavesPath()
{
#ifndef WIN32
    std::wstring path(fromMbcs(getenv("HOME")) + 
            std::wstring(L"/.einstein/save"));
#else
    TCHAR szPath[MAX_PATH];
    SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath);
    std::wstring path(&szPath[0]);
    if (path.length() > 0)
        path += L"\\";
    path += L"Einstein\\";
    ensureDirExists(path);
    path += L"save";
#endif
    ensureDirExists(path);

    return path;
}


typedef std::list<SavedGame> SavesList;


static void showListWindow(SavesList &list, Command **commands,
        const std::wstring &title, Area &area, Font *font)
{
    Font titleFont(L"nova.ttf", 26);

    area.add(new Window(250, 90, 300, 420, L"blue.bmp"));
    area.add(new Label(&titleFont, 250, 95, 300, 40, Label::ALIGN_CENTER,
                Label::ALIGN_MIDDLE, 255,255,0, title));
    ExitCommand exitCmd(area);
    area.add(new Button(360, 470, 80, 25, font, 255,255,0, L"blue.bmp", 
                msg(L"close"), &exitCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd)); 

    int pos = 150;
    int no = 0;
    for (auto& game : list)
    {
        area.add(new Button(260, pos, 280, 25, font, 255,255,255, L"blue.bmp", 
                    game.getName(), commands[no++]));
        pos += 30;
    }
    
    area.run();
}


bool saveGame(Area *parentArea, Game *game)
{
    std::wstring path = getSavesPath();
    
    Area area;
    area.add(parentArea, false);
    Font font(L"laudcn2.ttf", 14);
    bool saved = false;
    
    SavesList list;
    Command **commands = new Command*[MAX_SLOTS];
    for (int i = 0; i < MAX_SLOTS; i++) {
        SavedGame sg(path + L"/" + toString(i) + L".sav");
        list.push_back(sg);
        commands[i] = new SaveCommand(*(--(list.end())), &font, 
                &area, &saved, L"game " + toString(i+1), game);
    }
    
    showListWindow(list, commands, msg(L"saveGame"), area, &font);

    for (int i = 0; i < MAX_SLOTS; i++)
        delete commands[i];
    delete[] commands;
   
    return saved;
}


class LoadCommand: public Command
{
    private:
        SavedGame &savedGame;
        Area *parentArea;
        Font *font;
        std::wstring defaultName;
        Game **game;

    public:
        LoadCommand(SavedGame &sg, Font *f, Area *area, Game **g): 
            savedGame(sg)
        {
            parentArea = area;
            font = f;
            game = g;
        }
        
    public:
        void doAction() override {
            try {
                std::ifstream stream(toMbcs(savedGame.getFileName()).c_str(), 
                        std::ifstream::in | std::ifstream::binary);
                if (stream.fail())
                    throw Exception(L"Error opening save file");
                readString(stream);
                Game *g = new Game(stream);
                if (stream.fail())
                    throw Exception(L"Error loading game");
                stream.close();
                *game = g;
            } catch (...) { 
                showMessageWindow(parentArea, L"redpattern.bmp", 300, 80, font,
                        255,255,255, L"Error loadng game");
            }
            parentArea->finishEventLoop();
        }
};


Game* loadGame(Area *parentArea)
{
    std::wstring path = getSavesPath();
    
    Area area;
    area.add(parentArea, false);
    Font font(L"laudcn2.ttf", 14);
    
    Game *newGame = nullptr;
    
    SavesList list;
    Command **commands = new Command*[MAX_SLOTS];
    for (int i = 0; i < MAX_SLOTS; i++) {
        SavedGame sg(path + L"/" + toString(i) + L".sav");
        list.push_back(sg);
        if (sg.isExists())
            commands[i] = new LoadCommand(*(--(list.end())), &font, &area, 
                    &newGame);
        else
            commands[i] = nullptr;
    }
    
    showListWindow(list, commands, msg(L"loadGame"), area, &font);

    for (int i = 0; i < MAX_SLOTS; i++)
        delete commands[i];
    delete[] commands;
   
    return newGame;
}

