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


#include "convert.h"
#include "main.h"
#include "puzgen.h"
#include "utils.h"


static std::wstring getThingName(int row, int thing)
{
    std::wstring s;
    s += (wchar_t)(L'A' + row);
    s += toString(thing);
    return s;
}


class DrawableRule: public Rule
{
    protected:
        int row1, thing1;
        int row2, thing2;
        std::wstring ruleType;

    protected:
        DrawableRule();
        DrawableRule(std::istream &stream, const std::wstring& ruleType);
        void save(std::ostream &stream) override;
        virtual SDL_Surface* getImage(IconSet &iconSet, bool highlighted) = 0;
        void draw(int x, int y, IconSet &iconSet, bool highlighted) override;
};


DrawableRule::DrawableRule()
{
}


DrawableRule::DrawableRule(std::istream &stream, const std::wstring& ruleType):
    ruleType(ruleType)
{
    row1 = readInt(stream);
    thing1 = readInt(stream);
    row2 = readInt(stream);
    thing2 = readInt(stream);
}


void DrawableRule::draw(int x, int y, IconSet &iconSet, bool highlighted)
{
    SDL_Surface *s = getImage(iconSet, highlighted);
    screen.drawScaled(x, y, s);
    SDL_FreeSurface(s);
}


void DrawableRule::save(std::ostream &stream)
{
    writeString(stream, ruleType);
    writeInt(stream, row1);
    writeInt(stream, thing1);
    writeInt(stream, row2);
    writeInt(stream, thing2);
}


class HorizontalRule: public DrawableRule
{
    protected:
        HorizontalRule();
        HorizontalRule(std::istream &stream, const std::wstring& ruleType);
        virtual SDL_Surface* getLeftIcon(IconSet &iconSet, bool highlighted);
        virtual SDL_Surface* getMiddleIcon(IconSet &iconSet, bool highlighted) = 0;
        virtual SDL_Surface* getRightIcon(IconSet &iconSet, bool highlighted);
        SDL_Surface* getImage(IconSet &iconSet, bool highlighted) override;
};


HorizontalRule::HorizontalRule() = default;


HorizontalRule::HorizontalRule(std::istream &stream, const std::wstring& ruleType):
    DrawableRule(stream, ruleType)
{
}


SDL_Surface* HorizontalRule::getLeftIcon(IconSet &iconSet, bool h)
{
    return iconSet.getLargeIcon(row1, thing1, h);
}


SDL_Surface* HorizontalRule::getRightIcon(IconSet &iconSet, bool h)
{
    return iconSet.getLargeIcon(row2, thing2, h);
}


SDL_Surface* HorizontalRule::getImage(IconSet &iconSet, bool h)
{
    SDL_Surface *l = getLeftIcon(iconSet, h);
    SDL_Surface *s = makeSWSurface(l->w * 3, l->h);

    blitDraw(0, 0, l, s);
    blitDraw(l->w, 0, getMiddleIcon(iconSet, h), s);
    blitDraw(l->w * 2, 0, getRightIcon(iconSet, h), s);
    
    return s;
}


class NearRule: public HorizontalRule
{
    public:
        explicit NearRule(SolvedPuzzle puzzle);
        explicit NearRule(std::istream &stream);
        bool apply(Possibilities &pos) override;
        std::wstring getAsText() override;
    
    protected:
        SDL_Surface* getMiddleIcon(IconSet &iconSet, bool highlighted) override;
    
    private:
        bool applyToCol(Possibilities &pos, int col, int nearRow, int nearNum,
            int thisRow, int thisNum);
        ShowOptions getShowOpts() override { return SHOW_HORIZ; }
};


NearRule::NearRule(SolvedPuzzle puzzle)
{
    ruleType = L"near";
    int col1 = rndGen.genInt(PUZZLE_SIZE);
    row1 = rndGen.genInt(PUZZLE_SIZE);
    thing1 = puzzle[row1][col1];

    int col2;
    if (col1 == 0)
        col2 = 1;
    else
        if (col1 == PUZZLE_SIZE-1)
            col2 = PUZZLE_SIZE-2;
        else
            if (rndGen.genInt(2))
                col2 = col1 + 1;
            else
                col2 = col1 - 1;
    
    row2 = rndGen.genInt(PUZZLE_SIZE);
    thing2 = puzzle[row2][col2];
}


NearRule::NearRule(std::istream &stream):
    HorizontalRule(stream, L"near")
{
}


bool NearRule::applyToCol(Possibilities &pos, int col, int nearRow, int nearNum,
        int thisRow, int thisNum)
{
    bool hasLeft, hasRight;
    
    if (col == 0)
        hasLeft = false;
    else
        hasLeft = pos.isPossible(col - 1, nearRow, nearNum);
    if (col == PUZZLE_SIZE-1)
        hasRight = false;
    else
        hasRight = pos.isPossible(col + 1, nearRow, nearNum);
    
    if ((! hasRight) && (! hasLeft) && pos.isPossible(col, thisRow, thisNum)) {
        pos.exclude(col, thisRow, thisNum);
        return true;
    } else
        return false;
}


bool NearRule::apply(Possibilities &pos)
{
    bool changed = false;
    
    for (int i = 0; i < PUZZLE_SIZE; i++) {
        if (applyToCol(pos, i, row1, thing1, row2, thing2))
            changed = true;
        if (applyToCol(pos, i, row2, thing2, row1, thing1))
            changed = true;
    }

    if (changed)
        apply(pos);

    return changed;
}

std::wstring NearRule::getAsText()
{
    return getThingName(row1, thing1) + 
        L" is near to " + getThingName(row2, thing2);
}


SDL_Surface* NearRule::getMiddleIcon(IconSet &iconSet, bool h)
{
    return iconSet.getNearHintIcon(h);
}


class DirectionRule: public HorizontalRule
{
    public:
        explicit DirectionRule(SolvedPuzzle puzzle);
        explicit DirectionRule(std::istream &stream);
        bool apply(Possibilities &pos) override;
        std::wstring getAsText() override;
    
    protected:
        SDL_Surface* getMiddleIcon(IconSet &iconSet, bool highlighted) override;
    
    private:
        ShowOptions getShowOpts() override { return SHOW_HORIZ; }
};


DirectionRule::DirectionRule(SolvedPuzzle puzzle)
{
    ruleType = L"direction";
    row1 = rndGen.genInt(PUZZLE_SIZE);
    row2 = rndGen.genInt(PUZZLE_SIZE);
    int col1 = rndGen.genInt(PUZZLE_SIZE - 1);
    int col2 = rndGen.genInt(PUZZLE_SIZE - col1 - 1) + col1 + 1;
    thing1 = puzzle[row1][col1];
    thing2 = puzzle[row2][col2];
}


DirectionRule::DirectionRule(std::istream &stream):
    HorizontalRule(stream, L"direction")
{
}


bool DirectionRule::apply(Possibilities &pos)
{
    bool changed = false;

    for (int i = 0; i < PUZZLE_SIZE; i++) {
        if (pos.isPossible(i, row2, thing2)) {
            pos.exclude(i, row2, thing2);
            changed = true;
        }
        if (pos.isPossible(i, row1, thing1))
            break;
    }
    
    for (int i = PUZZLE_SIZE-1; i >= 0; i--) {
        if (pos.isPossible(i, row1, thing1)) {
            pos.exclude(i, row1, thing1);
            changed = true;
        }
        if (pos.isPossible(i, row2, thing2))
            break;
    }
    
    return changed;
}

std::wstring DirectionRule::getAsText()
{
    return getThingName(row1, thing1) + 
        L" is from the left of " + getThingName(row2, thing2);
}


SDL_Surface* DirectionRule::getMiddleIcon(IconSet &iconSet, bool h)
{
    return iconSet.getSideHintIcon(h);
}


class OpenRule: public Rule
{
    private:
        int col, row, thing;
        
    public:
        explicit OpenRule(SolvedPuzzle puzzle);
        explicit OpenRule(std::istream &stream);
        bool apply(Possibilities &pos) override;
        std::wstring getAsText() override;
        bool applyOnStart() override { return true; }
        void draw(int x, int y, IconSet &iconSet, bool highlighted) override { }
        ShowOptions getShowOpts() override { return SHOW_NOTHING; }
        void save(std::ostream &stream) override;
};


OpenRule::OpenRule(SolvedPuzzle puzzle)
{
    col = rndGen.genInt(PUZZLE_SIZE);
    row = rndGen.genInt(PUZZLE_SIZE);
    thing = puzzle[row][col];
}

OpenRule::OpenRule(std::istream &stream)
{
    col = readInt(stream);
    row = readInt(stream);
    thing = readInt(stream);
}

bool OpenRule::apply(Possibilities &pos)
{
    if (! pos.isDefined(col, row)) {
        pos.set(col, row, thing);
        return true;
    } else
        return false;
}

std::wstring OpenRule::getAsText()
{
    return getThingName(row, thing) + L" is at column " + toString(col+1);
}

void OpenRule::save(std::ostream &stream)
{
    writeString(stream, L"open");
    writeInt(stream, col);
    writeInt(stream, row);
    writeInt(stream, thing);
}


class UnderRule: public DrawableRule
{
    protected:
        SDL_Surface* getImage(IconSet &iconSet, bool h) override;
    
    public:
        explicit UnderRule(SolvedPuzzle puzzle);
        explicit UnderRule(std::istream &stream);
        bool apply(Possibilities &pos) override;
        std::wstring getAsText() override;
        ShowOptions getShowOpts() override { return SHOW_VERT; }
};


UnderRule::UnderRule(SolvedPuzzle puzzle)
{
    ruleType = L"under";
    int col = rndGen.genInt(PUZZLE_SIZE);
    row1 = rndGen.genInt(PUZZLE_SIZE);
    thing1 = puzzle[row1][col];
    do {
        row2 = rndGen.genInt(PUZZLE_SIZE);
    } while (row2 == row1) ;
    thing2 = puzzle[row2][col];
}


UnderRule::UnderRule(std::istream &stream):
    DrawableRule(stream, L"under")
{
}

bool UnderRule::apply(Possibilities &pos)
{
    bool changed = false;
 
    for (int i = 0; i < PUZZLE_SIZE; i++) {
        if ((! pos.isPossible(i, row1, thing1)) && 
                pos.isPossible(i, row2, thing2)) 
        {
            pos.exclude(i, row2, thing2);
            changed = true;
        }
        if ((! pos.isPossible(i, row2, thing2)) && 
                pos.isPossible(i, row1, thing1)) 
        {
            pos.exclude(i, row1, thing1);
            changed = true;
        }
    }

    return changed;
}


std::wstring UnderRule::getAsText()
{
    return getThingName(row1, thing1) + L" is the same column as " + 
        getThingName(row2, thing2);
}


SDL_Surface* UnderRule::getImage(IconSet &iconSet, bool h)
{
    SDL_Surface *t = iconSet.getLargeIcon(row1, thing1, h);
    SDL_Surface *s = makeSWSurface(t->w, t->h * 2);

    blitDraw(0, 0, t, s);
    blitDraw(0, t->h, iconSet.getLargeIcon(row2, thing2, h), s);
    
    return s;
}


class BetweenRule: public HorizontalRule
{
    private:
        int centerRow, centerThing;
        
    public:
        explicit BetweenRule(SolvedPuzzle puzzle);
        explicit BetweenRule(std::istream &stream);
        bool apply(Possibilities &pos) override;
        std::wstring getAsText() override;
    
    protected:
        SDL_Surface* getMiddleIcon(IconSet &iconSet, bool highlighted) override;
        SDL_Surface* getImage(IconSet &iconSet, bool higlighted) override;

    private:
        ShowOptions getShowOpts() override { return SHOW_HORIZ; }
        void save(std::ostream &stream) override;
};


BetweenRule::BetweenRule(SolvedPuzzle puzzle)
{
    ruleType = L"between";
    centerRow = rndGen.genInt(PUZZLE_SIZE);
    row1 = rndGen.genInt(PUZZLE_SIZE);
    row2 = rndGen.genInt(PUZZLE_SIZE);
    
    int centerCol = rndGen.genInt(PUZZLE_SIZE - 2) + 1;
    centerThing = puzzle[centerRow][centerCol];
    if (rndGen.genInt(2)) {
        thing1 = puzzle[row1][centerCol - 1];
        thing2 = puzzle[row2][centerCol + 1];
    } else {
        thing1 = puzzle[row1][centerCol + 1];
        thing2 = puzzle[row2][centerCol - 1];
    }
}

BetweenRule::BetweenRule(std::istream &stream):
    HorizontalRule(stream, L"between")
{
    centerRow = readInt(stream);
    centerThing = readInt(stream);
}

bool BetweenRule::apply(Possibilities &pos)
{
    bool changed = false;

    if (pos.isPossible(0, centerRow, centerThing)) {
        changed = true;
        pos.exclude(0, centerRow, centerThing);
    }
    
    if (pos.isPossible(PUZZLE_SIZE-1, centerRow, centerThing)) {
        changed = true;
        pos.exclude(PUZZLE_SIZE-1, centerRow, centerThing);
    }

    bool goodLoop;
    do {
        goodLoop = false;
        
        for (int i = 1; i < PUZZLE_SIZE-1; i++) {
            if (pos.isPossible(i, centerRow, centerThing)) {
                if (! ((pos.isPossible(i-1, row1, thing1) && 
                            pos.isPossible(i+1, row2, thing2)) ||
                        (pos.isPossible(i-1, row2, thing2) && 
                            pos.isPossible(i+1, row1, thing1))))
                {
                    pos.exclude(i, centerRow, centerThing);
                    goodLoop = true;
                }
            }
        }

        for (int i = 0; i < PUZZLE_SIZE; i++) {
            bool leftPossible, rightPossible;

            if (pos.isPossible(i, row2, thing2)) {
                if (i < 2)
                    leftPossible = false;
                else
                    leftPossible = (pos.isPossible(i-1, centerRow, centerThing)
                            && pos.isPossible(i-2, row1, thing1));
                if (i >= PUZZLE_SIZE - 2)
                    rightPossible = false;
                else
                    rightPossible = (pos.isPossible(i+1, centerRow, centerThing)
                            && pos.isPossible(i+2, row1, thing1));
                if ((! leftPossible) && (! rightPossible)) {
                    pos.exclude(i, row2, thing2);
                    goodLoop = true;
                }
            }

            if (pos.isPossible(i, row1, thing1)) {
                if (i < 2)
                    leftPossible = false;
                else
                    leftPossible = (pos.isPossible(i-1, centerRow, centerThing)
                            && pos.isPossible(i-2, row2, thing2));
                if (i >= PUZZLE_SIZE - 2)
                    rightPossible = false;
                else
                    rightPossible = (pos.isPossible(i+1, centerRow, centerThing)
                            && pos.isPossible(i+2, row2, thing2));
                if ((! leftPossible) && (! rightPossible)) {
                    pos.exclude(i, row1, thing1);
                    goodLoop = true;
                }
            }
        }

        if (goodLoop)
            changed = true;
    } while (goodLoop);

    return changed;
}

std::wstring BetweenRule::getAsText()
{
    return getThingName(centerRow, centerThing) + 
        L" is between " + getThingName(row1, thing1) + L" and " +
        getThingName(row2, thing2);
}


SDL_Surface* BetweenRule::getMiddleIcon(IconSet &iconSet, bool h)
{
    return iconSet.getLargeIcon(centerRow, centerThing, h);
}


SDL_Surface* BetweenRule::getImage(IconSet &iconSet, bool h)
{
    SDL_Surface *s = HorizontalRule::getImage(iconSet, h);
    
    SDL_Surface *arrow = iconSet.getBetweenArrow(h);
    blitDraw(s->h - (arrow->w - s->h) / 2, 0, arrow, s);
    
    return s;
}

void BetweenRule::save(std::ostream &stream)
{
    HorizontalRule::save(stream);
    writeInt(stream, centerRow);
    writeInt(stream, centerThing);
}



Rule* genRule(SolvedPuzzle &puzzle)
{
    int a = rndGen.genInt(14);
    switch (a) {
        case 0:
        case 1:
        case 2:
        case 3: return new NearRule(puzzle);
        case 4: return new OpenRule(puzzle);
        case 5:
        case 6: return new UnderRule(puzzle);
        case 7:
        case 8:
        case 9:
        case 10: return new DirectionRule(puzzle);
        case 11:
        case 12:
        case 13: return new BetweenRule(puzzle);
        default: return genRule(puzzle);
    }
}


void saveRules(Rules &rules, std::ostream &stream)
{
    writeInt(stream, rules.size());
    for (auto& rule : rules)
        rule->save(stream);
}


void loadRules(Rules &rules, std::istream &stream)
{
    int no = readInt(stream);

    for (int i = 0; i < no; i++) {
        std::wstring ruleType = readString(stream);
        Rule *r;
        if (ruleType == L"near") 
            r = new NearRule(stream);
        else if (ruleType == L"open") 
            r = new OpenRule(stream);
        else if (ruleType == L"under") 
            r = new UnderRule(stream);
        else if (ruleType == L"direction") 
            r = new DirectionRule(stream);
        else if (ruleType == L"between") 
            r = new BetweenRule(stream);
        else
            throw Exception(L"invalid rule type " + ruleType);
        rules.push_back(r);
    }
}


