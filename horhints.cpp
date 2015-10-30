#include "horhints.h"
#include "main.h"
#include "utils.h"
#include "sound.h"


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
    int qty = readInt(stream);

    for (int i = 0; i < qty; i++) {
        int no = readInt(stream);
        numbersArr.push_back(no);
        Rule *r = getRule(rl, no);
        int excluded = readInt(stream);
        if (excluded) {
            excludedRules.push_back(r);
            rules.push_back(NULL);
        } else {
            excludedRules.push_back(NULL);
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
    for (Rules::iterator i = r.begin(); i != r.end(); i++) {
        Rule *rule = *i;
        if (rule->getShowOpts() == Rule::SHOW_HORIZ) {
            rules.push_back(rule);
            excludedRules.push_back(NULL);
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
    int x = TILE_X + col * (TILE_WIDTH*3 + TILE_GAP_X);
    int y = TILE_Y + row * (TILE_HEIGHT + TILE_GAP_Y);

    Rule *r = NULL;
    int no = row * HINTS_COLS + col;
    if (no < (int)rules.size())
        if (showExcluded)
            r = excludedRules[no];
        else
            r = rules[no];
    if (r)
        r->draw(x, y, iconSet, no == highlighted);
    else
        for (int i = 0; i < 3; i++)
            screen.draw(x + TILE_HEIGHT*i, y, iconSet.getEmptyHintIcon());
    
    if (addToUpdate)
        screen.addRegionToUpdate(x, y, TILE_WIDTH*3, TILE_HEIGHT);
}


bool HorHints::onMouseButtonDown(int button, int x, int y)
{
    if (button != 3) 
        return false;

    int no = getRuleNo(x, y);
    if (no < 0) return false;
    int row = no / HINTS_COLS;
    int col = no - row * HINTS_COLS;
 
    if (showExcluded) {
        Rule *r = excludedRules[no];
        if (r) {
            sound->play(L"whizz.wav");
            rules[no] = r;
            excludedRules[no] = NULL;
            drawCell(col, row);
        }
    } else {
        Rule *r = rules[no];
        if (r) {
            sound->play(L"whizz.wav");
            rules[no] = NULL;
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
    int no = getRuleNo(x, y);

    if (no != highlighted) {
        int old = highlighted;
        highlighted = no;
        if (isActive(old)) {
            int row = old / HINTS_COLS;
            int col = old - row * HINTS_COLS;
            drawCell(col, row);
        }
        if (isActive(no)) {
            int row = no / HINTS_COLS;
            int col = no - row * HINTS_COLS;
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

    x = x - TILE_X;
    y = y - TILE_Y;

    int col = x / (TILE_WIDTH*3 + TILE_GAP_X);
    if (col * (TILE_WIDTH*3 + TILE_GAP_X) + TILE_WIDTH*3 < x)
        return -1;
    int row = y / (TILE_HEIGHT + TILE_GAP_Y);
    if (row * (TILE_HEIGHT + TILE_GAP_Y) + TILE_HEIGHT < y)
        return -1;
 
    int no = row * HINTS_COLS + col;
    if (no >= (int)rules.size())
        return -1;

    return no;
}

bool HorHints::isActive(int ruleNo)
{
    if ((ruleNo < 0) || (ruleNo >= (int)rules.size()))
        return false;
    Rule *r = showExcluded ? excludedRules[ruleNo] : rules[ruleNo];
    return r != NULL;
}


void HorHints::save(std::ostream &stream)
{
    int cnt = numbersArr.size();
    writeInt(stream, cnt);
    for (int i = 0; i < cnt; i++) {
        writeInt(stream, numbersArr[i]);
        writeInt(stream, rules[i] ? 0 : 1);
    }
    writeInt(stream, showExcluded ? 1 : 0);
}

