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


#include "verthints.h"

#include "main.h"
#include "puzgen.h"
#include "sound.h"
#include "utils.h"


#define TILE_NUM     15
#define TILE_GAP     4
#define TILE_X       12
#define TILE_Y       495
#define TILE_WIDTH   48
#define TILE_HEIGHT  48


VertHints::VertHints(IconSet &is, Rules &r): iconSet(is)
{
    reset(r);
}


VertHints::VertHints(IconSet &is, Rules &rl, std::istream &stream): iconSet(is)
{
    int qty = readInt(stream);

    for (int i = 0; i < qty; i++) {
        int no = readInt(stream);
        numbersArr.push_back(no);
        Rule *r = getRule(rl, no);
        int excluded = readInt(stream);
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

void VertHints::reset(Rules &r)
{
    rules.clear();
    excludedRules.clear();
    numbersArr.clear();
    
    int no = 0;
    for (auto rule : r)
    {
        if (rule->getShowOpts() == Rule::SHOW_VERT) {
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

void VertHints::draw()
{
    for (int i = 0; i < TILE_NUM; i++)
        drawCell(i, true);
}


void VertHints::drawCell(int col, bool addToUpdate)
{
    int x = TILE_X + col * (TILE_WIDTH + TILE_GAP);
    int y = TILE_Y;

    Rule *r = nullptr;
    if (col < (int)rules.size()) {
        if (showExcluded)
            r = excludedRules[col];
        else
            r = rules[col];
    }
    if (r)
        r->draw(x, y, iconSet, highlighted == col);
    else {
        SDL_Surface *t = iconSet.getEmptyHintIcon();
        SDL_Surface *s = makeSWSurface(t->w, t->h * 2);

        blitDraw(0, 0, t, s);
        blitDraw(0, t->h, t, s);
        screen.drawScaled(x, y, s);
        SDL_FreeSurface(s);
    }
    
    if (addToUpdate)
        screen.addRegionToUpdate(x, y, TILE_WIDTH, TILE_HEIGHT*2);
}


bool VertHints::onMouseButtonDown(int button, int x, int y)
{
    if (button != 3) 
        return false;
 
    int no = getRuleNo(x, y);
    if (no < 0) return false;
    
    if (no < (int)rules.size()) {
        if (showExcluded) {
            Rule *r = excludedRules[no];
            if (r) {
                sound->play(L"whizz.wav");
                rules[no] = r;
                excludedRules[no] = nullptr;
                drawCell(no);
            }
        } else {
            Rule *r = rules[no];
            if (r) {
                sound->play(L"whizz.wav");
                rules[no] = nullptr;
                excludedRules[no] = r;
                drawCell(no);
            }
        }
    }

    return true;
}


void VertHints::toggleExcluded()
{
    showExcluded = !showExcluded;
    draw();
}


bool VertHints::onMouseMove(int x, int y)
{
    int no = getRuleNo(x, y);

    if (no != highlighted) {
        int old = highlighted;
        highlighted = no;
        if (isActive(old)) 
            drawCell(old);
        if (isActive(no))
            drawCell(no);
    }

    return false;
}


int VertHints::getRuleNo(int x, int y)
{
    if (! isInRect(x, y, TILE_X, TILE_Y, (TILE_WIDTH + TILE_GAP) * TILE_NUM,
                TILE_HEIGHT * 2))
        return -1;

    x = scaleDown(x) - TILE_X;
    y = scaleDown(y) - TILE_Y;

    int no = x / (TILE_WIDTH + TILE_GAP);
    if (no * (TILE_WIDTH + TILE_GAP) + TILE_WIDTH < x)
        return -1;

    return no;
}

bool VertHints::isActive(int ruleNo)
{
    if ((ruleNo < 0) || (ruleNo >= (int)rules.size()))
        return false;
    Rule *r = showExcluded ? excludedRules[ruleNo] : rules[ruleNo];
    return r != nullptr;
}


void VertHints::save(std::ostream &stream)
{
    int cnt = numbersArr.size();
    writeInt(stream, cnt);
    for (int i = 0; i < cnt; i++) {
        writeInt(stream, numbersArr[i]);
        writeInt(stream, rules[i] ? 0 : 1);
    }
    writeInt(stream, showExcluded ? 1 : 0);
}

