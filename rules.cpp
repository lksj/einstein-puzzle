#include "puzgen.h"
#include "utils.h"
#include "main.h"
#include "convert.h"
#include "unicode.h"


static std::wstring getThingName(int row, int thing)
{
    std::wstring s;
    s += (wchar_t)(L'A' + row);
    s += toString(thing);
    return s;
}



class NearRule: public Rule
{
    private:
        int thing1[2];
        int thing2[2];
        
    public:
        NearRule(SolvedPuzzle puzzle);
        NearRule(std::istream &stream);
        virtual bool apply(Possibilities &pos);
        virtual std::wstring getAsText();

    private:
        bool applyToCol(Possibilities &pos, int col, int nearRow, int nearNum,
            int thisRow, int thisNum);
        virtual void draw(int x, int y, IconSet &iconSet, bool highlighted);
        virtual ShowOptions getShowOpts() { return SHOW_HORIZ; };
        virtual void save(std::ostream &stream);
};


NearRule::NearRule(SolvedPuzzle puzzle)
{
    int col1 = rndGen.genInt(PUZZLE_SIZE);
    thing1[0] = rndGen.genInt(PUZZLE_SIZE);
    thing1[1] = puzzle[thing1[0]][col1];

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
    
    thing2[0] = rndGen.genInt(PUZZLE_SIZE);
    thing2[1] = puzzle[thing2[0]][col2];
}


NearRule::NearRule(std::istream &stream)
{
    thing1[0] = readInt(stream);
    thing1[1] = readInt(stream);
    thing2[0] = readInt(stream);
    thing2[1] = readInt(stream);
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
        if (applyToCol(pos, i, thing1[0], thing1[1], thing2[0], thing2[1]))
            changed = true;
        if (applyToCol(pos, i, thing2[0], thing2[1], thing1[0], thing1[1]))
            changed = true;
    }

    if (changed)
        apply(pos);

    return changed;
}

std::wstring NearRule::getAsText()
{
    return getThingName(thing1[0], thing1[1]) + 
        L" is near to " + getThingName(thing2[0], thing2[1]);
}

void NearRule::draw(int x, int y, IconSet &iconSet, bool h)
{
    SDL_Surface *icon = iconSet.getLargeIcon(thing1[0], thing1[1], h);
    screen.draw(x, y, icon);
    screen.draw(x + icon->h, y, iconSet.getNearHintIcon(h));
    screen.draw(x + icon->h*2, y, iconSet.getLargeIcon(thing2[0], thing2[1], h));
}

void NearRule::save(std::ostream &stream)
{
    writeString(stream, L"near");
    writeInt(stream, thing1[0]);
    writeInt(stream, thing1[1]);
    writeInt(stream, thing2[0]);
    writeInt(stream, thing2[1]);
}


class DirectionRule: public Rule
{
    private:
        int row1, thing1;
        int row2, thing2;
        
    public:
        DirectionRule(SolvedPuzzle puzzle);
        DirectionRule(std::istream &stream);
        virtual bool apply(Possibilities &pos);
        virtual std::wstring getAsText();

    private:
        virtual void draw(int x, int y, IconSet &iconSet, bool highlighted);
        virtual ShowOptions getShowOpts() { return SHOW_HORIZ; };
        virtual void save(std::ostream &stream);
};


DirectionRule::DirectionRule(SolvedPuzzle puzzle)
{
    row1 = rndGen.genInt(PUZZLE_SIZE);
    row2 = rndGen.genInt(PUZZLE_SIZE);
    int col1 = rndGen.genInt(PUZZLE_SIZE - 1);
    int col2 = rndGen.genInt(PUZZLE_SIZE - col1 - 1) + col1 + 1;
    thing1 = puzzle[row1][col1];
    thing2 = puzzle[row2][col2];
}

DirectionRule::DirectionRule(std::istream &stream)
{
    row1 = readInt(stream);
    thing1 = readInt(stream);
    row2 = readInt(stream);
    thing2 = readInt(stream);
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

void DirectionRule::draw(int x, int y, IconSet &iconSet, bool h)
{
    SDL_Surface *icon = iconSet.getLargeIcon(row1, thing1, h);
    screen.draw(x, y, icon);
    screen.draw(x + icon->h, y, iconSet.getSideHintIcon(h));
    screen.draw(x + icon->h*2, y, iconSet.getLargeIcon(row2, thing2, h));
}

void DirectionRule::save(std::ostream &stream)
{
    writeString(stream, L"direction");
    writeInt(stream, row1);
    writeInt(stream, thing1);
    writeInt(stream, row2);
    writeInt(stream, thing2);
}


class OpenRule: public Rule
{
    private:
        int col, row, thing;
        
    public:
        OpenRule(SolvedPuzzle puzzle);
        OpenRule(std::istream &stream);
        virtual bool apply(Possibilities &pos);
        virtual std::wstring getAsText();
        virtual bool applyOnStart() { return true; };
        virtual void draw(int x, int y, IconSet &iconSet, bool highlighted) { };
        virtual ShowOptions getShowOpts() { return SHOW_NOTHING; };
        virtual void save(std::ostream &stream);
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


class UnderRule: public Rule
{
    private:
        int row1, thing1, row2, thing2;
        
    public:
        UnderRule(SolvedPuzzle puzzle);
        UnderRule(std::istream &stream);
        virtual bool apply(Possibilities &pos);
        virtual std::wstring getAsText();
        virtual void draw(int x, int y, IconSet &iconSet, bool highlighted);
        virtual ShowOptions getShowOpts() { return SHOW_VERT; };
        virtual void save(std::ostream &stream);
};


UnderRule::UnderRule(SolvedPuzzle puzzle)
{
    int col = rndGen.genInt(PUZZLE_SIZE);
    row1 = rndGen.genInt(PUZZLE_SIZE);
    thing1 = puzzle[row1][col];
    do {
        row2 = rndGen.genInt(PUZZLE_SIZE);
    } while (row2 == row1) ;
    thing2 = puzzle[row2][col];
}

UnderRule::UnderRule(std::istream &stream)
{
    row1 = readInt(stream);
    thing1 = readInt(stream);
    row2 = readInt(stream);
    thing2 = readInt(stream);
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

void UnderRule::draw(int x, int y, IconSet &iconSet, bool h)
{
    SDL_Surface *icon = iconSet.getLargeIcon(row1, thing1, h);
    screen.draw(x, y, icon);
    screen.draw(x, y + icon->h, iconSet.getLargeIcon(row2, thing2, h));
}

void UnderRule::save(std::ostream &stream)
{
    writeString(stream, L"under");
    writeInt(stream, row1);
    writeInt(stream, thing1);
    writeInt(stream, row2);
    writeInt(stream, thing2);
}



class BetweenRule: public Rule
{
    private:
        int row1, thing1;
        int row2, thing2;
        int centerRow, centerThing;
        
    public:
        BetweenRule(SolvedPuzzle puzzle);
        BetweenRule(std::istream &stream);
        virtual bool apply(Possibilities &pos);
        virtual std::wstring getAsText();

    private:
        virtual void draw(int x, int y, IconSet &iconSet, bool highlighted);
        virtual ShowOptions getShowOpts() { return SHOW_HORIZ; };
        virtual void save(std::ostream &stream);
};


BetweenRule::BetweenRule(SolvedPuzzle puzzle)
{
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

BetweenRule::BetweenRule(std::istream &stream)
{
    row1 = readInt(stream);
    thing1 = readInt(stream);
    row2 = readInt(stream);
    thing2 = readInt(stream);
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

void BetweenRule::draw(int x, int y, IconSet &iconSet, bool h)
{
    SDL_Surface *icon = iconSet.getLargeIcon(row1, thing1, h);
    screen.draw(x, y, icon);
    screen.draw(x + icon->w, y, iconSet.getLargeIcon(centerRow, centerThing, h));
    screen.draw(x + icon->w*2, y, iconSet.getLargeIcon(row2, thing2, h));
    SDL_Surface *arrow = iconSet.getBetweenArrow(h);
    screen.draw(x + icon->w - (arrow->w - icon->w) / 2, y + 0, arrow);
}

void BetweenRule::save(std::ostream &stream)
{
    writeString(stream, L"between");
    writeInt(stream, row1);
    writeInt(stream, thing1);
    writeInt(stream, row2);
    writeInt(stream, thing2);
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
    for (Rules::iterator i = rules.begin(); i != rules.end(); i++)
        (*i)->save(stream);
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


