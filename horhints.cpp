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


#include "horhints.h"

#include "main.h"
#include "sound.h"
#include "utils.h"


#define HINTS_COLS   3
#define HINTS_ROWS   8
#define TILE_GAP_X   4
#define TILE_GAP_Y   4
#define TILE_X       348
#define TILE_Y       68
#define TILE_WIDTH   48
#define TILE_HEIGHT  48


HorHints::HorHints(IconSet &is, Rules &r): iconSet(is)
{
    reset(r);
}


HorHints::HorHints(IconSet &is, Rules &rl, std::istream &stream): iconSet(is)
{
    const int qty = readInt(stream);

    for (int i = 0; i < qty; i++) {
        int no = readInt(stream);
        numbersArr.push_back(no);
        Rule *r = getRule(rl, no);
        const int excluded = readInt(stream);
        if (excluded) {
            excludedRules.push_back(r);
            rules.push_back(nullptr);
        } else {
            excludedRules.push_back(nullptr);
            rules.push_back(r);
        }
    }

    showExcluded = readInt(stream);
    
    int x, y;
    SDL_GetMouseState(&x, &y);
    highlighted = getRuleNo(x, y);
}


void HorHints::reset(Rules &r)
{
    rules.clear();
    excludedRules.clear();
    numbersArr.clear();
    
    int no = 0;
    for (auto rule : r)
    {
        if (rule->getShowOpts() == Rule::SHOW_HORIZ) {
            rules.push_back(rule);
            excludedRules.push_back(nullptr);
            numbersArr.push_back(no);
        }
        no++;
    }
    
    showExcluded = false;

    int x, y;
    SDL_GetMouseState(&x, &y);
    highlighted = getRuleNo(x, y);
}

void HorHints::draw()
{
    for (int i = 0; i < HINTS_ROWS; i++)
        for (int j = 0; j < HINTS_COLS; j++)
            drawCell(j, i, true);
}

void HorHints::drawCell(int col, int row, bool addToUpdate)
{
    const int x = TILE_X + col * (TILE_WIDTH*3 + TILE_GAP_X);
    const int y = TILE_Y + row * (TILE_HEIGHT + TILE_GAP_Y);

    Rule *r = nullptr;
    const int no = row * HINTS_COLS + col;
    if (no < (int)rules.size())
    {
        if (showExcluded)
            r = excludedRules[no];
        else
            r = rules[no];
    }
    if (r)
        r->draw(x, y, iconSet, no == highlighted);
    else
    {
        SDL_Surface *t = iconSet.getEmptyHintIcon();
        SDL_Surface *s = makeSWSurface(t->w * 3, t->h);
        for (int i = 0; i < 3; i++)
        {
            blitDraw(0 + (i * t->w), 0, t, s);
        }
        screen.drawScaled(x, y, s);
        SDL_FreeSurface(s);
    }
    
    if (addToUpdate)
        screen.addRegionToUpdate(x, y, TILE_WIDTH*3, TILE_HEIGHT);
}


bool HorHints::onMouseButtonDown(int button, int x, int y)
{
    if (button != 3) 
        return false;

    const int no = getRuleNo(x, y);
    if (no < 0) return false;
    const int row = no / HINTS_COLS;
    const int col = no - row * HINTS_COLS;
 
    if (showExcluded) {
        Rule *r = excludedRules[no];
        if (r) {
            sound->play(L"whizz.wav");
            rules[no] = r;
            excludedRules[no] = nullptr;
            drawCell(col, row);
        }
    } else {
        Rule *r = rules[no];
        if (r) {
            sound->play(L"whizz.wav");
            rules[no] = nullptr;
            excludedRules[no] = r;
            drawCell(col, row);
        }
    }

    return true;
}


void HorHints::toggleExcluded()
{
    showExcluded = !showExcluded;
    draw();
}


bool HorHints::onMouseMove(int x, int y)
{
    const int no = getRuleNo(x, y);

    if (no != highlighted) {
        const int old = highlighted;
        highlighted = no;
        if (isActive(old)) {
            const int row = old / HINTS_COLS;
            const int col = old - row * HINTS_COLS;
            drawCell(col, row);
        }
        if (isActive(no)) {
            const int row = no / HINTS_COLS;
            const int col = no - row * HINTS_COLS;
            drawCell(col, row);
        }
    }

    return false;
}


int HorHints::getRuleNo(int x, int y)
{
    if (! isInRect(x, y, TILE_X, TILE_Y, (TILE_WIDTH*3 + TILE_GAP_X) * HINTS_COLS,
                (TILE_HEIGHT + TILE_GAP_Y) * HINTS_ROWS))
        return -1;

    x = scaleDown(x) - TILE_X;
    y = scaleDown(y) - TILE_Y;

    const int col = x / (TILE_WIDTH*3 + TILE_GAP_X);
    if (col * (TILE_WIDTH*3 + TILE_GAP_X) + TILE_WIDTH*3 < x)
        return -1;
    const int row = y / (TILE_HEIGHT + TILE_GAP_Y);
    if (row * (TILE_HEIGHT + TILE_GAP_Y) + TILE_HEIGHT < y)
        return -1;
 
    const int no = row * HINTS_COLS + col;
    if (no >= (int)rules.size())
        return -1;

    return no;
}

bool HorHints::isActive(int ruleNo)
{
    if ((ruleNo < 0) || (ruleNo >= (int)rules.size()))
        return false;
    Rule *r = showExcluded ? excludedRules[ruleNo] : rules[ruleNo];
    return r != nullptr;
}


void HorHints::save(std::ostream &stream)
{
    const int cnt = numbersArr.size();
    writeInt(stream, cnt);
    for (int i = 0; i < cnt; i++) {
        writeInt(stream, numbersArr[i]);
        writeInt(stream, rules[i] ? 0 : 1);
    }
    writeInt(stream, showExcluded ? 1 : 0);
}

