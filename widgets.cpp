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


#include "widgets.h"
#include "main.h"
#include "utils.h"
#include "sound.h"

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////
//
// Button
//
//////////////////////////////////////////////////////////////////


Button::Button(int x, int y, const std::wstring &name, Command *cmd, 
        bool transparent)
{
    image = loadImage(name, transparent);
    highlighted = adjustBrightness(image, 1.5, transparent);

    left = x;
    top = y;
    width = image->w;
    height = image->h;

    mouseInside = false;
    command = cmd;
}


Button::Button(int x, int y, int w, int h, Font *font, 
        int fR, int fG, int fB, int hR, int hG, int hB,
        const std::wstring &text, Command *cmd)
{
    left = x;
    top = y;
    width = w;
    height = h;

    SDL_Surface *s = screen.getRegion(left, top, width, height);
    
    int tW, tH;
    font->getSize(text, tW, tH);
    font->draw(s, (width - tW) / 2, (height - tH) / 2, fR, fG, fB, true, text);
    image = SDL_DisplayFormat(s);
    SDL_FreeSurface(s);
    
    s = screen.getRegion(left, top, width, height);
    font->draw(s, (width - tW) / 2, (height - tH) / 2, hR, hG, hB, true, text);
    highlighted = SDL_DisplayFormat(s);
    SDL_FreeSurface(s);
    
    mouseInside = false;
    command = cmd;
}


Button::Button(int x, int y, int w, int h, Font *font, 
        int r, int g, int b, const std::wstring &bg, 
        const std::wstring &text, bool bevel, Command *cmd)
{
    left = x;
    top = y;
    width = w;
    height = h;

    if (bevel)
    {
        image = makeBox(width, height, bg);
    }
    else
    {
        image = makeSWSurface(width, height);
        drawTiled(bg, image);
    }
    
    int tW, tH;
    font->getSize(text, tW, tH);
    font->draw(image, (width - tW) / 2, (height - tH) / 2, r, g, b, true, text);
    
    highlighted = adjustBrightness(image, 1.5, false);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, getCornerPixel(image));
    SDL_SetColorKey(highlighted, SDL_SRCCOLORKEY, getCornerPixel(highlighted));
    
    mouseInside = false;
    command = cmd;
}



Button::Button(int x, int y, int w, int h, Font *font, 
        int r, int g, int b, const std::wstring &bg, 
        const std::wstring &text, Command *cmd)
{
    left = x;
    top = y;
    width = w;
    height = h;

    image = makeBox(width, height, bg);
    
    int tW, tH;
    font->getSize(text, tW, tH);
    font->draw(image, (width - tW) / 2, (height - tH) / 2, r, g, b, true, text);
    
    highlighted = adjustBrightness(image, 1.5, false);
    
    mouseInside = false;
    command = cmd;
}


Button::~Button()
{
    SDL_FreeSurface(image);
    SDL_FreeSurface(highlighted);
}


void Button::draw()
{
    if (mouseInside)
        screen.draw(left, top, highlighted);
    else
        screen.draw(left, top, image);
    screen.addRegionToUpdate(left, top, width, height);
}


void Button::getBounds(int &l, int &t, int &w, int &h)
{
    l = left;
    t = top;
    w = width;
    h = height;
}


bool Button::onMouseButtonDown(int button, int x, int y)
{
    if (isInRect(x, y, left, top, width, height)) {
        sound->play(L"click.wav");
        if (command)
            command->doAction();
        return true;
    } else
        return false;
}


bool Button::onMouseMove(int x, int y)
{
    bool in = isInRect(x, y, left, top, width, height);
    if (in != mouseInside) {
        mouseInside = in;
        draw();
    }
    return false;
}


//////////////////////////////////////////////////////////////////
//
// KeyAccel
//
//////////////////////////////////////////////////////////////////


KeyAccel::KeyAccel(SDLKey sym, Command *cmd)
{
    command = cmd;
    key = sym;
}


bool KeyAccel::onKeyDown(SDLKey k, unsigned char ch)
{
    if (key == k) {
        if (command)
            command->doAction();
        return true;
    } else
        return false;
}


//////////////////////////////////////////////////////////////////
//
// Area
//
//////////////////////////////////////////////////////////////////


Area::Area()
{
    timer = NULL;
}

Area::~Area()
{
    for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++) {
        Widget *w = *i;
        if (w && w->destroyByArea() && (! notManagedWidgets.count(w)))
            delete w;
    }
}

void Area::add(Widget *widget, bool managed)
{
    if (!contains(widget))
    {
        widgets.push_back(widget);
    }
    if (! managed)
        notManagedWidgets.insert(widget);
    widget->setParent(this);
}

bool Area::contains(Widget * widget)
{
    for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
    {
        if (widget == *i)
        {
            return true;
        }
      }
    return false;
}

void Area::remove(Widget *widget)
{
    
    if (contains(widget))
    {
        widgets.remove(widget);
    }
    notManagedWidgets.insert(widget);
}

void Area::handleEvent(const SDL_Event &event)
{
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
                if ((*i)->onMouseButtonDown(event.button.button, 
                            event.button.x, event.button.y))
                    return;
            break;
        
        case SDL_MOUSEBUTTONUP:
            for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
                if ((*i)->onMouseButtonUp(event.button.button, 
                            event.button.x, event.button.y))
                    return;
            break;
        
        case SDL_MOUSEMOTION:
            for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
                if ((*i)->onMouseMove(event.motion.x, event.motion.y))
                    return;
            break;
        
        case SDL_VIDEOEXPOSE:
            for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
                (*i)->draw();
            break;
        
        case SDL_KEYDOWN:
            for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
                if ((*i)->onKeyDown(event.key.keysym.sym, 
                            (unsigned char)event.key.keysym.unicode))
                    return;
            break;
        
        case SDL_QUIT:
            exit(0);
    }
}

void Area::run()
{
    terminate = false;
    SDL_Event event;
    
    Uint32 lastTimer = 0;
    draw();
    screen.showMouse();
    
    bool runTimer = timer ? true : false;
    bool dispetchEvent;
    while (! terminate) {
        dispetchEvent = true;
        if (! timer) {
            SDL_WaitEvent(&event);
        } else {
            Uint32 now = SDL_GetTicks();
            if (now - lastTimer > time) {
                lastTimer = now;
                runTimer = true;
            }
            if (! SDL_PollEvent(&event)) {
                if (! runTimer) {
                    SDL_Delay(20);
                    continue;
                } else
                    dispetchEvent = false;
            }
        }
        screen.hideMouse();
        if (runTimer) {
            if (timer)
                timer->onTimer();
            runTimer = false;
        }
        if (dispetchEvent)
            handleEvent(event);
        if (! terminate) {
            screen.showMouse();
            screen.flush();
        }
    }
}

void Area::finishEventLoop()
{
    terminate = true;
}


void Area::draw()
{
    for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
        (*i)->draw();
}


void Area::setTimer(Uint32 interval, TimerHandler *t)
{
    time = interval;
    timer = t;
}


void Area::updateMouse()
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    
    for (WidgetsList::iterator i = widgets.begin(); i != widgets.end(); i++)
        if ((*i)->onMouseMove(x, y))
                    return;
}



//////////////////////////////////////////////////////////////////
//
// AnyKeyAccel
//
//////////////////////////////////////////////////////////////////



AnyKeyAccel::AnyKeyAccel()
{
    command = NULL;
}

AnyKeyAccel::AnyKeyAccel(Command *cmd)
{
    command = cmd;
}

AnyKeyAccel::~AnyKeyAccel()
{
}

bool AnyKeyAccel::onKeyDown(SDLKey key, unsigned char ch)
{
    if (((key >= SDLK_NUMLOCK) && (key <= SDLK_COMPOSE)) || 
            (key == SDLK_TAB) || (key == SDLK_UNKNOWN))
        return false;

    if (command)
        command->doAction();
    else
        area->finishEventLoop();
    return true;
}

bool AnyKeyAccel::onMouseButtonDown(int button, int x, int y)
{
    if (command)
        command->doAction();
    else
        area->finishEventLoop();
    return true;
}



//////////////////////////////////////////////////////////////////
//
// Window
//
//////////////////////////////////////////////////////////////////



Window::Window(int x, int y, int w, int h, const std::wstring &bg, 
                bool frameWidth, bool raised)
{
    left = x;
    top = y;
    width = w;
    height = h;
    
    SDL_Surface *win = makeSWSurface(width, height);
    
    drawTiled(bg, win);

    SDL_LockSurface(win);
    double k = 2.6;
    double f = 0.1;
    for (int i = 0; i < frameWidth; i++) {
        double ltK, rbK;
        if (raised) {
            ltK = k;  rbK = f;
        } else {
            ltK = f;  rbK = k;
        }
        for (int j = i; j < height - i - 1; j++)
            adjustBrightness(win, i, j, ltK);
        for (int j = i; j < width - i; j++)
            adjustBrightness(win, j, i, ltK);
        for (int j = i+1; j < height - i; j++)
            adjustBrightness(win, width - i - 1, j, rbK);
        for (int j = i; j < width - i - 1; j++)
            adjustBrightness(win, j, height - i - 1, rbK);
        k -= 0.2;
        f += 0.1;
    }
    SDL_UnlockSurface(win);
    
    SDL_Surface *s = scaleUp(win);
    
    background = SDL_DisplayFormat(s);
    SDL_FreeSurface(win);
    SDL_FreeSurface(s);
}


Window::~Window()
{
    SDL_FreeSurface(background);
}


void Window::draw()
{
    screen.drawDirect(left, top, background);
    screen.addRegionToUpdate(left, top, width, height);
}



//////////////////////////////////////////////////////////////////
//
// Label
//
//////////////////////////////////////////////////////////////////



Label::Label(Font *f, int x, int y, int r, int g, int b, std::wstring s,
        bool sh): text(s)
{
    font = f;
    left = x;
    top = y;
    red = r;
    green = g;
    blue = b;
    hAlign = ALIGN_LEFT;
    vAlign = ALIGN_TOP;
    shadow = sh;
}


Label::Label(Font *f, int x, int y, int w, int h, HorAlign hA, VerAlign vA, 
        int r, int g, int b, const std::wstring &s): 
                text(s)
{
    font = f;
    left = x;
    top = y;
    red = r;
    green = g;
    blue = b;
    hAlign = hA;
    vAlign = vA;
    width = w;
    height = h;
    shadow = true;
}


void Label::draw()
{
    int w, h, x, y;
    font->getSize(text, w, h);

    switch (hAlign) {
        case ALIGN_RIGHT: x = left + width - w; break;
        case ALIGN_CENTER: x = left + (width - w) / 2; break;
        default: x = left;
    }
    
    switch (vAlign) {
        case ALIGN_BOTTOM: y = top + height - h; break;
        case ALIGN_MIDDLE: y = top + (height - h) / 2; break;
        default: y = top;
    }
    
    font->draw(x, y, red,green,blue, shadow, text);
    screen.addRegionToUpdate(x, y, w, h);
}



//////////////////////////////////////////////////////////////////
//
// InputField
//
//////////////////////////////////////////////////////////////////


InputField::InputField(int x, int y, int w, int h, const std::wstring &background, 
        std::wstring &s, int maxLen, int r, int g, int b, Font *f): 
                Window(x, y, w, h, background, 1, false), text(s)
{
    maxLength = maxLen;
    red = r;
    green = g;
    blue = b;
    font = f;
    moveCursor(text.length());
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    SDL_EnableUNICODE(1);
}

InputField::~InputField()
{
    SDL_EnableKeyRepeat(0, 0);
}

void InputField::draw()
{
    Window::draw();
    int pad = (height/4);

    SDL_Rect rect = { left+1, top+1, width-2, height-2 };
    screen.setClipRect(&rect);
    
    font->draw(left+pad, top+pad, red,green,blue, true, text);

    if (cursorVisible) {
        int pos = 0;
        if (cursorPos > 0)
        {
            font->setScaled(true);
            pos += font->getWidth(text.substr(0, cursorPos));
            font->setScaled(false);
        }
        SDL_Surface *s = makeSWSurface(2, height-4);
        
        for (int i = 0; i < s->h; i++) {
            setPixel(s, 0, i, red, green, blue);
            setPixel(s, 1, i, red, green, blue);
        }
        screen.draw(left + pad + screen.reverseScale(pos), top + 2, s);
        SDL_FreeSurface(s);
    }
    
    screen.setClipRect(NULL);
}

void InputField::setParent(Area *a)
{
    Window::setParent(a);
    area->setTimer(100, this);
}


void InputField::onTimer()
{
    Uint32 now = SDL_GetTicks();
    if (now - lastCursor > 1000) {
        cursorVisible = ! cursorVisible;
        lastCursor = now;
        draw();
    }
}


bool InputField::onKeyDown(SDLKey key, unsigned char translatedChar)
{
    switch (key) {
        case SDLK_BACKSPACE:
            if (cursorPos > 0) {
                text.erase(cursorPos - 1, 1);
                moveCursor(cursorPos - 1);
            } else
                moveCursor(cursorPos);
            draw();
            return true;

        case SDLK_LEFT:
            if (cursorPos > 0)
                moveCursor(cursorPos - 1);
            else
                moveCursor(cursorPos);
            draw();
            return true;

        case SDLK_RIGHT:
            if (cursorPos < (int)text.length())
                moveCursor(cursorPos + 1);
            else
                moveCursor(cursorPos);
            draw();
            return true;

        case SDLK_HOME:
            moveCursor(0);
            draw();
            return true;

        case SDLK_END:
            moveCursor(text.length());
            draw();
            return true;

        case SDLK_DELETE:
            if (cursorPos < (int)text.length())
                text.erase(cursorPos, 1);
            moveCursor(cursorPos);
            draw();
            return true;

        default: ;
    }
    
    if (translatedChar > 31)
        onCharTyped(translatedChar);
    return false;
}

bool InputField::onKeyUp(SDLKey key)
{
    return false;
}

void InputField::onCharTyped(unsigned char ch)
{
    if ((int)text.length() < maxLength) {
        wchar_t buf[2];
        buf[0] = ch;
        buf[1] = 0;
        text.insert(cursorPos, buf);
        moveCursor(cursorPos + 1);
    } else
        moveCursor(cursorPos);
    draw();
}
        
void InputField::moveCursor(int pos)
{
    lastCursor = SDL_GetTicks();
    cursorVisible = true;
    cursorPos = pos;
}


//////////////////////////////////////////////////////////////////
//
// Checkbox
//
//////////////////////////////////////////////////////////////////



Checkbox::Checkbox(int x, int y, int w, int h, Font *f, 
        int r, int g, int b, const std::wstring &bg, 
        bool &chk): checked(chk)
{
    left = x;
    top = y;
    width = w;
    height = h;
    checked = chk;
    font = f;
    red = r;
    green = g;
    blue = b;

    image = makeBox(width, height, bg);
        
    SDL_Surface *s = scaleUp(image);
    SDL_FreeSurface(image);
    image = s;
    
    highlighted = adjustBrightness(image, 1.5, false);
    
    mouseInside = false;
}


Checkbox::~Checkbox()
{
    SDL_FreeSurface(image);
    SDL_FreeSurface(highlighted);
}


void Checkbox::draw()
{
    screen.drawDirect(left, top, (mouseInside ? highlighted : image));
    
    if (checked)
    {
        int r = red;
        int g = green;
        int b = blue;
        
        int tW, tH;
        font->getSize(L"X", tW, tH);
        
        if (mouseInside)
        {
            adjustBrightness(&r, &g, &b, 1.5);
            
        }
        font->draw(left + ((width - tW) / 2), top + ((height - tH) / 2), r, g, b, true, L"X");
    }
    screen.addRegionToUpdate(left, top, width, height);
}


void Checkbox::getBounds(int &l, int &t, int &w, int &h)
{
    l = left;
    t = top;
    w = width;
    h = height;
}


bool Checkbox::onMouseButtonDown(int button, int x, int y)
{
    if (isInRect(x, y, left, top, width, height)) {
        sound->play(L"click.wav");
        checked = ! checked;
        draw();
        return true;
    } else
        return false;
}


bool Checkbox::onMouseMove(int x, int y)
{
    bool in = isInRect(x, y, left, top, width, height);
    if (in != mouseInside) {
        mouseInside = in;
        draw();
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////
//
// Picture
//
//////////////////////////////////////////////////////////////////////////////

Picture::Picture(int x, int y, const std::wstring &name, bool transparent)
{
    SDL_Surface *s = loadImage(name, transparent);
    width = s->w;
    height = s->h;
    image = scaleUp(s);
    SDL_FreeSurface(s);
    left = x;
    top = y;
}

Picture::Picture(int x, int y, SDL_Surface *img)
{
    image = scaleUp(img);
    left = x;
    top = y;
    width = img->w;
    height = img->h;
}

Picture::~Picture() 
{ 
    SDL_FreeSurface(image); 
}

void Picture::draw()
{
    screen.drawDirect(left, top, image);
    screen.addRegionToUpdate(left, top, width, height);
}

void Picture::moveX(const int newX) 
{ 
    left = newX; 
}

void Picture::getBounds(int &l, int &t, int &w, int &h)
{
    l = left;
    t = top;
    w = width;
    h = height;
}


//////////////////////////////////////////////////////////////////
//
// Slider
//
//////////////////////////////////////////////////////////////////

Slider::Slider(int x, int y, int w, int h, float &v): value(v)
{
    left = x;
    top = y;
    width = w;
    height = h;
    background = NULL;
    createSlider(height);
    highlight = false;
    dragging = false;
}

Slider::~Slider()
{
    if (background)
        SDL_FreeSurface(background);
    if (slider)
        SDL_FreeSurface(slider);
    if (activeSlider)
        SDL_FreeSurface(activeSlider);
}

void Slider::draw()
{
    if (! background)
        createBackground();
    screen.drawDirect(left, top, background);
    screen.addRegionToUpdate(left, top, width, height);
    int posX = valueToX(value);
    SDL_Surface *s = highlight ? activeSlider : slider;
    screen.drawDirect(left + posX, top, s);
}

void Slider::createBackground()
{
    background = screen.createSubimage(left, top, width, height);
    int y = background->h / 2;
    SDL_LockSurface(background);
    drawBevel(background, 0, y - screen.doScale(2), background->w, screen.doScale(4), false, 1);
    SDL_UnlockSurface(background);
}

void Slider::createSlider(int size)
{
    SDL_Surface *image = makeBox(size, size, L"blue.bmp");

    SDL_Surface *s = scaleUp(image);
    SDL_FreeSurface(image);
    image = s;
    
    activeSlider = adjustBrightness(image, 1.5, false);
    slider = SDL_DisplayFormat(image);
    
    SDL_FreeSurface(image);
}


bool Slider::onMouseButtonDown(int button, int x, int y)
{
    bool in = isInRect(x, y, left, top, width, height);
    if (in) {
        int sliderX = valueToX(value);
        bool hl = isInRect(x, y, left + sliderX, top, height, height);
        if (hl) {
            dragging = true;
            dragOffsetX = screen.reverseScale(x) - left - sliderX;
        }
    }
    return in;
}

bool Slider::onMouseButtonUp(int button, int x, int y)
{
    if (dragging) {
        dragging = false;
        return true;
    } else
        return false;
}


int Slider::valueToX(float value) 
{
    if (value < 0)
        value = 0.0f;
    if (value > 1)
        value = 1.0f;
    return (int)(((float)(width - height)) * value);
}


float Slider::xToValue(int pos) 
{
    if (0 > pos)
        pos = 0;
    if (width - height < pos)
        pos = width - height;
    return (float)pos / (float)(width - height);
}



bool Slider::onMouseMove(int x, int y)
{
    if (dragging) {
        float val = xToValue(screen.reverseScale(x) - left - dragOffsetX);
        if (val != value) {
            value = val;
            draw();
        }
        return true;
    }
    
    bool in = isInRect(x, y, left, top, width, height);
    if (in) {
        int sliderX = valueToX(value);
        bool hl = isInRect(x, y, left + sliderX, top, height, height);
        if (hl != highlight) {
            highlight = hl;
            draw();
        }
    } else
        if (highlight) {
            highlight = false;
            draw();
        }
    return in;
}

//////////////////////////////////////////////////////////////////
//
// CycleButton
//
//////////////////////////////////////////////////////////////////

CycleButton::CycleButton(int x, int y, int w, int h, Font *f, int &v, std::vector<std::wstring> o): value(v)
{
    left = x;
    top = y;
    width = w;
    height = h;
    font = f;
    options = o;
    
    image = NULL;
    highlighted = NULL;
    drawTiles();
  
    mouseInside = false;
}


CycleButton::~CycleButton()
{
    SDL_FreeSurface(image);
    SDL_FreeSurface(highlighted);
}


void CycleButton::draw()
{
    if (mouseInside)
        screen.drawDirect(left, top, highlighted);
    else
        screen.drawDirect(left, top, image);
    screen.addRegionToUpdate(left, top, width, height);
}

void CycleButton::drawTiles()
{
    if (image)
    {
        SDL_FreeSurface(image);
        SDL_FreeSurface(highlighted);
    }
    image = makeBox(width, height, L"blue.bmp");

    std::wstring text = options[value];
    int r = 255;
    int g = 255;
    int b = 0;
    
    SDL_Surface *s = scaleUp(image);
    SDL_FreeSurface(image);
    image = s;
    
    int tW, tH;
    font->setScaled(true);
    font->getSize(text, tW, tH);
    font->draw(image, (image->w - tW) / 2, (image->h - tH) / 2, r, g, b, true, text);
    font->setScaled(false);
    
    highlighted = adjustBrightness(image, 1.5, false);
    
    draw();
}

void CycleButton::getBounds(int &l, int &t, int &w, int &h)
{
    l = left;
    t = top;
    w = width;
    h = height;
}


bool CycleButton::onMouseButtonDown(int button, int x, int y)
{
    if (isInRect(x, y, left, top, width, height)) {
        sound->play(L"click.wav");
        value = (value + 1) % (options.size());
      
        drawTiles();
        return true;
    } else
        return false;
}


bool CycleButton::onMouseMove(int x, int y)
{
    bool in = isInRect(x, y, left, top, width, height);
    if (in != mouseInside) {
        mouseInside = in;
        draw();
    }
    return false;
}
