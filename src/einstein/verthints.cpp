#include "verthints.h"
#include "main.h"
#include "utils.h"
#include "puzgen.h"
#include "sound.h"


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

void VertHints::reset(Rules &r)
{
    rules.clear();
    excludedRules.clear();
    numbersArr.clear();
    
    int no = 0;
    for (Rules::iterator i = r.begin(); i != r.end(); i++) {
        Rule *rule = *i;
        if (rule->getShowOpts() == Rule::SHOW_VERT) {
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

void VertHints::draw()
{
    for (int i = 0; i < TILE_NUM; i++)
        drawCell(i, true);
}


void VertHints::drawCell(int col, bool addToUpdate)
{
    int x = TILE_X + col * (TILE_WIDTH + TILE_GAP);
    int y = TILE_Y;

    Rule *r = NULL;
    if (col < (int)rules.size()) {
        if (showExcluded)
            r = excludedRules[col];
        else
            r = rules[col];
    }
    if (r)
        r->draw(x, y, iconSet, highlighted == col);
    else {
        screen.draw(x, y, iconSet.getEmptyHintIcon());
        screen.draw(x, y + TILE_HEIGHT, iconSet.getEmptyHintIcon());
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
                excludedRules[no] = NULL;
                drawCell(no);
            }
        } else {
            Rule *r = rules[no];
            if (r) {
                sound->play(L"whizz.wav");
                rules[no] = NULL;
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

    x = x - TILE_X;
    y = y - TILE_Y;

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
    return r != NULL;
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

