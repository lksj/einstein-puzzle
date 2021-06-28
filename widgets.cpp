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


#include "widgets.h"

#include "main.h"
#include "sound.h"
#include "utils.h"


//////////////////////////////////////////////////////////////////
//
// BoundedWidget
//
//////////////////////////////////////////////////////////////////


BoundedWidget::BoundedWidget(int x, int y, int w, int h, bool t):
    left(x), top(y), width(w), height(h), transparent(t)
{
}


int BoundedWidget::getLeft()
{
    return scaleUp(left);
}

int BoundedWidget::getTop()
{
    return scaleUp(top);
}

int BoundedWidget::getWidth()
{
    return width;
}

int BoundedWidget::getHeight()
{
    return height;
}


//////////////////////////////////////////////////////////////////
//
// TileWidget
//
//////////////////////////////////////////////////////////////////


TileWidget::TileWidget(int x, int y, int w, int h, bool t):
    BoundedWidget(x, y, w, h, t)
{
    scale = -1.0;
    image = nullptr;
    sImage = nullptr;
}


TileWidget::~TileWidget()
{
    SDL_FreeSurface(image);
    SDL_FreeSurface(sImage);
}


SDL_Surface* TileWidget::getImage()
{
    return sImage;
}


void TileWidget::draw()
{
    if (!sImage || screen.getScale() != scale)
    {
        rescale();
    }
    
    SDL_Surface *s = getImage();
    
    if (s)
    {
        screen.draw(getLeft(), getTop(), s);
        screen.addRegionToUpdate(left, top, width, height);
    }
}


void TileWidget::rescale()
{
    SDL_FreeSurface(sImage);
    
    if (image)
    {
        scale = screen.getScale();
        sImage = scaleUp(image);
        
        if (transparent)
        {
            SDL_SetColorKey(sImage, SDL_SRCCOLORKEY, getCornerPixel(image));
        }
    }
}


//////////////////////////////////////////////////////////////////
//
// HighlightableWidget
//
//////////////////////////////////////////////////////////////////


HighlightableWidget::HighlightableWidget(int x, int y, int w, int h, bool t):
    TileWidget(x, y, w, h, t)
{
    highlighted = nullptr;
    sHighlighted = nullptr;
    mouseInside = false;
}


HighlightableWidget::~HighlightableWidget()
{
    SDL_FreeSurface(highlighted);
    SDL_FreeSurface(sHighlighted);
}


SDL_Surface* HighlightableWidget::getImage()
{
    return (mouseInside ? sHighlighted : sImage);
}


void HighlightableWidget::rescale()
{
    TileWidget::rescale();
    SDL_FreeSurface(sHighlighted);
    
    if (highlighted)
    {
        sHighlighted = scaleUp(highlighted);
    
        if (transparent)
        {
            SDL_SetColorKey(sHighlighted, SDL_SRCCOLORKEY, getCornerPixel(highlighted));
        }
    }
}


//////////////////////////////////////////////////////////////////
//
// ClickableWidget
//
//////////////////////////////////////////////////////////////////


ClickableWidget::ClickableWidget(int x, int y, int w, int h, bool t):
    HighlightableWidget(x, y, w, h, t)
{
}


void ClickableWidget::doClick()
{
    sound->play(L"click.wav");
    handleClick();
}


bool ClickableWidget::onMouseButtonDown(int button, int x, int y)
{
    if (isInRect(x, y, left, top, width, height)) {
        doClick();
        
        return true;
    } else
        return false;
}


bool ClickableWidget::onMouseMove(int x, int y)
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
// TextHighlightWidget
//
//////////////////////////////////////////////////////////////////


TextHighlightWidget::TextHighlightWidget(int x, int y, int w, int h, Font *f, 
        int fR, int fG, int fB, int hR, int hG, int hB, bool transparent):
    ClickableWidget(x, y, w, h, transparent)
{
    font = f;
    
    red = fR;
    green = fG;
    blue = fB;
    hRed = hR;
    hGreen = hG;
    hBlue = hB;
}


TextHighlightWidget::TextHighlightWidget(int x, int y, int w, int h, Font *f, 
        int r, int g, int b, bool transparent):
    ClickableWidget(x, y, w, h, transparent)
{
    font = f;
    
    red = r;
    green = g;
    blue = b;
    hRed = r;
    hGreen = g;
    hBlue = b;
    
    adjustBrightness(&hRed, &hGreen, &hBlue, 1.5);
}


void TextHighlightWidget::draw()
{
    HighlightableWidget::draw();
    
    int r = red;
    int g = green;
    int b = blue;

    if (mouseInside)
    {
        r = hRed;
        g = hGreen;
        b = hBlue;
    }
    
    int tW, tH;
    font->getSize(getText(), tW, tH);
    tW = scaleDown(tW);
    tH = scaleDown(tH);
    font->draw(left + ((width - tW) / 2), top + ((height - tH) / 2), r, g, b, true, getText());
    
    screen.addRegionToUpdate(left, top, width, height);
}


//////////////////////////////////////////////////////////////////
//
// Button
//
//////////////////////////////////////////////////////////////////


Button::Button(int x, int y, int w, int h, Font *font, 
        int fR, int fG, int fB, int hR, int hG, int hB,
        const std::wstring &text, Command *cmd):
    TextHighlightWidget(x, y, w, h, font, fR, fG, fB, hR, hG, hB, true),
    text(text)
{
    image = nullptr;
    highlighted = nullptr;
    
    command = cmd;
}


Button::Button(int x, int y, int w, int h, Font *font, 
        int r, int g, int b, const std::wstring &bg, 
        const std::wstring &text, bool bevel, Command *cmd):
    TextHighlightWidget(x, y, w, h, font, r, g, b, !bevel),
    text(text)
{  
    if (bevel)
    {
        image = makeBox(width, height, bg);
    }
    else
    {
        image = makeSWSurface(width, height);
        drawTiled(bg, image);
    }
    
    highlighted = adjustBrightness(image, 1.5, false);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, getCornerPixel(image));
    SDL_SetColorKey(highlighted, SDL_SRCCOLORKEY, getCornerPixel(highlighted));
    
    command = cmd;
}


Button::Button(int x, int y, int w, int h, Font *font, 
        int r, int g, int b, const std::wstring &bg, 
        const std::wstring &text, Command *cmd):
    TextHighlightWidget(x, y, w, h, font, r, g, b),
    text(text)
{  
    image = makeBox(width, height, bg);
    highlighted = adjustBrightness(image, 1.5, false);
    
    command = cmd;
}


std::wstring Button::getText()
{
    return text;
}


void Button::handleClick()
{
    if (command)
    {
        command->doAction();
    }
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


Area::Area():
    terminate(true),
    timer(nullptr)
{
}

Area::~Area()
{
    for (auto w : widgets)
    {
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
    for (auto& i : widgets)
    {
        if (widget == i)
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

void Area::setVisible(Widget *widget, bool visible)
{
    if (visible)
    {
        add(widget);
    }
    else
    {
        remove(widget);
    }
}

void Area::handleEvent(const SDL_Event &event)
{
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            for (auto& widget : widgets)
                if (widget->onMouseButtonDown(event.button.button, 
                            event.button.x, event.button.y))
                    return;
            break;
        
        case SDL_MOUSEBUTTONUP:
            for (auto& widget : widgets)
                if (widget->onMouseButtonUp(event.button.button, 
                            event.button.x, event.button.y))
                    return;
            break;
        
        case SDL_MOUSEMOTION:
            for (auto& widget : widgets)
                if (widget->onMouseMove(event.motion.x, event.motion.y))
                    return;
            break;
        
        case SDL_VIDEOEXPOSE:
            for (auto& widget : widgets)
                widget->draw();
            break;
        
        case SDL_KEYDOWN:
            for (auto& widget : widgets)
                if (widget->onKeyDown(event.key.keysym.sym, 
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
    
    bool runTimer = timer;
    while (! terminate) {
        bool dispetchEvent = true;
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
    for (auto& widget : widgets)
        widget->draw();
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
    
    for (auto& widget : widgets)
        if (widget->onMouseMove(x, y))
                    return;
}



//////////////////////////////////////////////////////////////////
//
// AnyKeyAccel
//
//////////////////////////////////////////////////////////////////



AnyKeyAccel::AnyKeyAccel()
{
    command = nullptr;
}

AnyKeyAccel::AnyKeyAccel(Command *cmd)
{
    command = cmd;
}

AnyKeyAccel::~AnyKeyAccel() = default;

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
                int frameWidth, bool raised):
    TileWidget(x, y, w, h)
{
    image = makeSWSurface(width, height);
    
    drawTiled(bg, image);

    SDL_LockSurface(image);
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
            adjustBrightness(image, i, j, ltK);
        for (int j = i; j < width - i; j++)
            adjustBrightness(image, j, i, ltK);
        for (int j = i+1; j < height - i; j++)
            adjustBrightness(image, width - i - 1, j, rbK);
        for (int j = i; j < width - i - 1; j++)
            adjustBrightness(image, j, height - i - 1, rbK);
        k -= 0.2;
        f += 0.1;
    }
    SDL_UnlockSurface(image);
}


//////////////////////////////////////////////////////////////////
//
// Label
//
//////////////////////////////////////////////////////////////////



Label::Label(Font *f, int x, int y, int r, int g, int b, const std::wstring& s,
        bool sh):
    BoundedWidget(x, y, f->getWidth(s), f->getHeight(s)), text(s)
{
    font = f;
    red = r;
    green = g;
    blue = b;
    hAlign = ALIGN_LEFT;
    vAlign = ALIGN_TOP;
    shadow = sh;
}


Label::Label(Font *f, int x, int y, int w, int h, HorAlign hA, VerAlign vA, 
        int r, int g, int b, const std::wstring &s): 
    BoundedWidget(x, y, w, h), text(s)
{
    font = f;
    red = r;
    green = g;
    blue = b;
    hAlign = hA;
    vAlign = vA;
    shadow = true;
}


void Label::draw()
{
    int w, h, x, y;
    font->getSize(text, w, h);
    w = scaleDown(w);
    h = scaleDown(h);

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
// ManagedLabel
//
//////////////////////////////////////////////////////////////////


ManagedLabel::ManagedLabel(const std::wstring& fontName, int ptSize, int x, int y,
                int r, int g, int b, const std::wstring& text, bool shadow):
    Label(new Font(fontName, ptSize), x, y, r, g, b, text, shadow)
{
}


ManagedLabel::ManagedLabel(const std::wstring& fontName, int ptSize, int x, int y, int w, int h,
        HorAlign hA, VerAlign vA, int r, int g, int b, const std::wstring &s): 
    Label(new Font(fontName, ptSize), x, y, w, h, hA, vA, r, g, b, s)
{
}


ManagedLabel::~ManagedLabel()
{
    delete font;
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
            pos += font->getWidth(text.substr(0, cursorPos));
        }
        SDL_Surface *s = makeSWSurface(2, height-4);
        
        for (int i = 0; i < s->h; i++) {
            setPixel(s, 0, i, red, green, blue);
            setPixel(s, 1, i, red, green, blue);
        }
        screen.drawScaled(left + pad + scaleDown(pos), top + 2, s);
        SDL_FreeSurface(s);
    }
    
    screen.setClipRect(nullptr);
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
        bool &chk):
    TextHighlightWidget(x, y, w, h, f, r, g, b), 
    checked(chk)
{
    image = makeBox(width, height, bg);
    highlighted = adjustBrightness(image, 1.5, false);
}


std::wstring Checkbox::getText()
{
    return (checked ? L"X" : L"");
}


void Checkbox::handleClick()
{
    checked = ! checked;
    draw();
}


//////////////////////////////////////////////////////////////////////////////
//
// Picture
//
//////////////////////////////////////////////////////////////////////////////

Picture::Picture(int x, int y, const std::wstring &name, bool transparent):
    TileWidget(x, y, 0, 0, false)
{
    image = loadImage(name, transparent);
    width = image->w;
    height = image->h;
}


Picture::Picture(int x, int y, SDL_Surface *img):
    TileWidget(x, y, img->w, img->h)
{
    image = SDL_DisplayFormat(img);
}


//////////////////////////////////////////////////////////////////
//
// Slider
//
//////////////////////////////////////////////////////////////////

Slider::Slider(int x, int y, int w, int h, float &v):
    BoundedWidget(x, y, w, h),
    background(nullptr),
    slider(nullptr),
    activeSlider(nullptr),
    highlight(false),
    dragging(false),
    dragOffsetX(0),
    value(v)
{
    createSlider(height);
}

Slider::~Slider()
{
    SDL_FreeSurface(background);
    SDL_FreeSurface(slider);
    SDL_FreeSurface(activeSlider);
}

void Slider::draw()
{
    if (! background)
        createBackground();
    screen.draw(getLeft(), getTop(), background);
    screen.addRegionToUpdate(left, top, width, height);
    int posX = scaleUp(valueToX(value));
    SDL_Surface *s = highlight ? activeSlider : slider;
    screen.draw(getLeft() + posX, getTop(), s);
}

void Slider::createBackground()
{
    background = screen.createSubimage(left, top, width, height);
    int y = background->h / 2;
    SDL_LockSurface(background);
    drawBevel(background, 0, y - scaleUp(2), background->w, scaleUp(4), false, 1);
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

void Slider::changeValue(float v)
{
    value = v;
}

bool Slider::onMouseButtonDown(int button, int x, int y)
{
    bool in = isInRect(x, y, left, top, width, height);
    if (in) {
        int sliderX = valueToX(value);
        bool hl = isInRect(x, y, left + sliderX, top, height, height);
        if (hl) {
            dragging = true;
            dragOffsetX = scaleDown(x) - left - sliderX;
        }
        else
        {
            if (isInRect(x, y, left, (top + (height / 2) - 2), width, 4))
            {
                changeValue(xToValue(scaleDown(x) - left - (height / 2)));
                sound->play(L"click.wav");
                draw();
            }
        }
    }
    return in;
}

bool Slider::onMouseButtonUp(int button, int x, int y)
{
    if (dragging) {
        dragging = false;
        sound->play(L"click.wav");
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
        float val = xToValue(scaleDown(x) - left - dragOffsetX);
        if (val != value) {
            changeValue(val);
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

CycleButton::CycleButton(int x, int y, int w, int h, Font *f, int &v,
        const std::vector<std::wstring>& o)
    : TextHighlightWidget(x, y, w, h, f, 255, 255, 0),
        value(v), options(o)
{
    image = makeBox(width, height, L"blue.bmp");
    highlighted = adjustBrightness(image, 1.5, false);
}


std::wstring CycleButton::getText()
{
    return options[value];
}

void CycleButton::handleClick()
{
    value = (value + 1) % (options.size());
    draw();
}
