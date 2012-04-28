// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-04-28 by Jordan Evens <jordan.evens@gmail.com>

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


#ifndef __WIDGETS_H__
#define __WIDGETS_H__

#include <string>
#include <list>
#include <set>
#include <SDL/SDL.h>
#include "font.h"
#include <vector>


class Command
{
    public:
        virtual ~Command() { };
        virtual void doAction() = 0;
};


class Area;


class Widget
{
    protected:
        Area *area;
    
    public:
        virtual ~Widget() { };

    public:
        virtual bool onMouseButtonDown(int button, int x, int y) { return false; };
        virtual bool onMouseButtonUp(int button, int x, int y) { return false; };
        virtual bool onMouseMove(int x, int y) { return false; };
        virtual void draw() { };
        virtual void setParent(Area *a) { area = a; };
        virtual bool onKeyDown(SDLKey key, unsigned char ch) { return false; };
        virtual bool destroyByArea() { return true; };
};

class BoundedWidget: public Widget
{
    protected:
        int left, top, width, height;
    
    public:
        int getLeft() const { return left; };
        int getTop() const { return top; };
        int getWidth() const { return width; };
        int getHeight() const { return height; };
        void getBounds(int &l, int &t, int &w, int &h)
        {
            l = left;
            t = top;
            w = width;
            h = height;
        }
};

class Button: public BoundedWidget
{
    protected:
        SDL_Surface *image, *highlighted;
        bool mouseInside;
        Command *command;
        
    public:
        Button(int x, int y, const std::wstring &name, Command *cmd=NULL, 
                bool transparent=true);
        Button(int x, int y, int width, int height, Font *font, 
                int fR, int fG, int fB, int hR, int hG, int hB, 
                const std::wstring &text, Command *cmd=NULL);
        Button(int x, int y, int width, int height, Font *font, 
                int r, int g, int b, const std::wstring &background, 
                const std::wstring &text, Command *cmd=NULL);
        Button(int x, int y, int width, int height, Font *font, 
                int r, int g, int b, const std::wstring &background, 
                const std::wstring &text, bool bevel, Command *cmd=NULL);
        virtual ~Button();

    public:
        virtual void draw();
        virtual bool onMouseButtonDown(int button, int x, int y);
        virtual bool onMouseMove(int x, int y);
        void moveTo(int x, int y) { left = x; top = y; };
};



class KeyAccel: public Widget
{
    protected:
        SDLKey key;
        Command *command;

    public:
        KeyAccel(SDLKey key, Command *command);
        virtual bool onKeyDown(SDLKey key, unsigned char ch);
};


class TimerHandler
{
    public:
        virtual ~TimerHandler() { };
        virtual void onTimer() = 0;
};


class Area: public Widget
{
    private:
        typedef std::list<Widget*> WidgetsList;
        WidgetsList widgets;
        std::set<Widget*> notManagedWidgets;
        bool terminate;
        Uint32 time;
        TimerHandler *timer;

    public:
        Area();
        virtual ~Area();

    public:
        bool contains(Widget * widget);
        void add(Widget *widget, bool manage=true);
        void remove(Widget *widget);
        void handleEvent(const SDL_Event &event);
        void run();
        void finishEventLoop();
        virtual void draw();
        void setTimer(Uint32 interval, TimerHandler *handler);
        void updateMouse();
        virtual bool destroyByArea() { return false; };
};


class ExitCommand: public Command
{
    private:
        Area &area;
    
    public:
        ExitCommand(Area &a): area(a) { }
        
        virtual void doAction() {
            area.finishEventLoop();
        };
};


class AnyKeyAccel: public Widget
{
    protected:
        Command *command;

    public:
        AnyKeyAccel();                  // use exit command by default
        AnyKeyAccel(Command *command);
        virtual ~AnyKeyAccel();

    public:
        virtual bool onKeyDown(SDLKey key, unsigned char ch);
        virtual bool onMouseButtonDown(int button, int x, int y);
};


class Window: public Widget
{
    protected:
        int left, top, width, height;
        SDL_Surface *background;
    
    public:
        Window(int x, int y, int w, int h, const std::wstring &background, 
                bool frameWidth=4, bool raised=true);
        virtual ~Window();

    public:
        virtual void draw();
};


class Label: public Widget
{
    public:
        typedef enum {
            ALIGN_LEFT,
            ALIGN_CENTER,
            ALIGN_RIGHT
        } HorAlign;
        
        typedef enum {
            ALIGN_TOP,
            ALIGN_MIDDLE,
            ALIGN_BOTTOM
        } VerAlign;
    
    protected:
        Font *font;
        std::wstring text;
        int left, top, width, height;
        int red, green, blue;
        HorAlign hAlign;
        VerAlign vAlign;
        bool shadow;

    public:
        Label(Font *font, int x, int y, int r, int g, int b, 
                std::wstring text, bool shadow=true);
        Label(Font *font, int x, int y, int width, int height,
                HorAlign hAlign, VerAlign vAlign, int r, int g, int b, 
                const std::wstring &text);

    public:
        virtual void draw();
};


class InputField: public Window, public TimerHandler
{
    private:
        std::wstring &text;
        int maxLength;
        int cursorPos;
        int red, green, blue;
        Font *font;
        Uint32 lastCursor;
        bool cursorVisible;
        char lastChar;
        Uint32 lastKeyUpdate;
    
    public:
        InputField(int x, int y, int w, int h, const std::wstring &background, 
                std::wstring &name, int maxLength, int r, int g, int b, Font *font);
        ~InputField();
        
    public:
        virtual void draw();
        virtual void setParent(Area *a);
        virtual void onTimer();
        virtual bool onKeyDown(SDLKey key, unsigned char ch);
        virtual bool onKeyUp(SDLKey key);
        virtual void onCharTyped(unsigned char ch);

    private:
        void moveCursor(int pos);
};


class Checkbox: public BoundedWidget
{
    protected:
        SDL_Surface *image, *highlighted;
        bool &checked;
        bool mouseInside;
        Font* font;
        int red, green, blue;
        
    public:
        Checkbox(int x, int y, int width, int height, Font *font, 
                int r, int g, int b, const std::wstring &background,
                bool &checked);
        virtual ~Checkbox();

    public:
        virtual void draw();
        virtual bool onMouseButtonDown(int button, int x, int y);
        virtual bool onMouseMove(int x, int y);
        void moveTo(int x, int y) { left = x; top = y; };
};

class Picture: public BoundedWidget
{
    protected:
        SDL_Surface *image;
        
    public:
        Picture(int x, int y, const std::wstring &name, bool transparent=true);
        Picture(int x, int y, SDL_Surface *image);
        virtual ~Picture();

    public:
        virtual void draw();
        void moveX(const int newX);
};


class Slider: public Widget
{
    private:
        int left, top, width, height;
        float &value;
        SDL_Surface *background;
        SDL_Surface *slider;
        SDL_Surface *activeSlider;
        bool highlight;
        bool dragging;
        int dragOffsetX;

    public:
        Slider(int x, int y, int width, int height, float &value);
        virtual ~Slider();

    public:
        virtual void draw();
        virtual bool onMouseButtonDown(int button, int x, int y);
        virtual bool onMouseButtonUp(int button, int x, int y);
        virtual bool onMouseMove(int x, int y);

    private:
        void createBackground();
        void createSlider(int size);
        int valueToX(float value);
        float xToValue(int pos);
};


class CycleButton: public BoundedWidget
{
    protected:
        SDL_Surface *image, *highlighted;
        bool mouseInside;
        Font *font;
        int &value;
        std::vector<std::wstring> options;
        void drawTiles();
        
    public:
        CycleButton(int x, int y, int width, int height, Font *font, int &value, std::vector<std::wstring> options);
        virtual ~CycleButton();

    public:
        virtual void draw();
        virtual bool onMouseButtonDown(int buttons, int x, int y);
        virtual bool onMouseMove(int x, int y);
        void moveTo(int x, int y) { left = x; top = y; };
};

#endif

