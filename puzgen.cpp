#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include "puzgen.h"
#include "exceptions.h"
#include "utils.h"



Possibilities::Possibilities()
{
    reset();
}

Possibilities::Possibilities(std::istream &stream)
{
    for (int row = 0; row < PUZZLE_SIZE; row++)
        for (int col = 0; col < PUZZLE_SIZE; col++)
            for (int element = 0; element < PUZZLE_SIZE; element++)
                pos[col][row][element] = readInt(stream);
}

void Possibilities::reset()
{
    for (int i = 0; i < PUZZLE_SIZE; i++)
        for (int j = 0; j < PUZZLE_SIZE; j++)
            for (int k = 0; k < PUZZLE_SIZE; k++)
                pos[i][j][k] = k + 1;
}

void Possibilities::checkSingles(int row)
{
    int cellsCnt[PUZZLE_SIZE];   // count of elements in cells
    int elsCnt[PUZZLE_SIZE];     // total count of elements in row
    int elements[PUZZLE_SIZE];   // one element of each cell
    int elCells[PUZZLE_SIZE];    // one cell of each element

    memset(cellsCnt, 0, sizeof(cellsCnt));
    memset(elsCnt, 0, sizeof(elsCnt));
    memset(elements, 0, sizeof(elements));
    memset(elCells, 0, sizeof(elCells));
    
    // check if there is only one element left in cell(col, row)
    for (int col = 0; col < PUZZLE_SIZE; col++)
        for (int i = 0; i < PUZZLE_SIZE; i++) {
            if (pos[col][row][i]) {
                elsCnt[i]++;
                elCells[i] = col;
                cellsCnt[col]++;
                elements[col] = i + 1;
            }
        }

    bool changed = false;
    
    // check for cells with single element
    for (int col = 0; col < PUZZLE_SIZE; col++) {
        if ((cellsCnt[col] == 1) && (elsCnt[elements[col] - 1] != 1)) {
            // there is only one element in cell but it used somewhere else
            int e = elements[col] - 1;
            for (int i = 0; i < PUZZLE_SIZE; i++)
                if (i != col)
                    pos[i][row][e] = 0;
            changed = true;
        }
    }

    // check for single element without exclusive cell
    for (int el = 0; el < PUZZLE_SIZE; el++)
        if ((elsCnt[el] == 1) && (cellsCnt[elCells[el]] != 1)) {
            int col = elCells[el];
            for (int i = 0; i < PUZZLE_SIZE; i++)
                if (i != el)
                    pos[col][row][i] = 0;
            changed = true;
        }
    
    if (changed)
        checkSingles(row);
}

void Possibilities::exclude(int col, int row, int element)
{
    if (! pos[col][row][element - 1])
        return;

    pos[col][row][element - 1] = 0;

    checkSingles(row);
}

void Possibilities::set(int col, int row, int element)
{
    for (int i = 0; i < PUZZLE_SIZE; i++)
        if ((i != element - 1))
            pos[col][row][i] = 0;
        else
            pos[col][row][i] = element;
    
    for (int j = 0; j < PUZZLE_SIZE; j++)
        if (j != col)
            pos[j][row][element - 1] = 0;
    
    checkSingles(row);
}

bool Possibilities::isPossible(int col, int row, int element)
{
    return pos[col][row][element - 1] == element;
}

bool Possibilities::isDefined(int col, int row)
{
    int solvedCnt = 0, unsolvedCnt = 0;
    for (int i = 0; i < PUZZLE_SIZE; i++)
        if (! pos[col][row][i])
            unsolvedCnt++;
        else
            solvedCnt++;
    return ((unsolvedCnt == PUZZLE_SIZE-1) && (solvedCnt == 1));
}


int Possibilities::getDefined(int col, int row)
{
    for (int i = 0; i < PUZZLE_SIZE; i++)
        if (pos[col][row][i])
            return i + 1;
    return 0;
}


bool Possibilities::isSolved()
{
    for (int i = 0; i < PUZZLE_SIZE; i++)
        for (int j = 0; j < PUZZLE_SIZE; j++)
            if (! isDefined(i, j))
                return false;
    return true;
}


bool Possibilities::isValid(SolvedPuzzle &puzzle)
{
    for (int row = 0; row < PUZZLE_SIZE; row++)
        for (int col = 0; col < PUZZLE_SIZE; col++)
            if (! isPossible(col, row, puzzle[row][col]))
                return false;
    return true;
}


int Possibilities::getPosition(int row, int element)
{
    int cnt = 0;
    int lastPos = -1;
    
    for (int i = 0; i < PUZZLE_SIZE; i++)
        if (pos[i][row][element - 1] == element) {
            cnt++;
            lastPos = i;
        }

    return cnt == 1 ? lastPos : -1;
}

void Possibilities::print()
{
    for (int row = 0; row < PUZZLE_SIZE; row++) {
        std::cout << (char)('A' + row) << " ";
        for (int col = 0; col < PUZZLE_SIZE; col++) {
            for (int i = 0; i < PUZZLE_SIZE; i++)
                if (pos[col][row][i])
                    std::cout << pos[col][row][i];
                else
                    std::cout << " ";
            std::cout << "   ";
        }
        std::cout << std::endl;
    }
}

void Possibilities::makePossible(int col, int row, int element)
{
    pos[col][row][element-1] = element;
}

void Possibilities::save(std::ostream &stream)
{
    for (int row = 0; row < PUZZLE_SIZE; row++)
        for (int col = 0; col < PUZZLE_SIZE; col++)
            for (int element = 0; element < PUZZLE_SIZE; element++)
                writeInt(stream, pos[col][row][element]);
}


static void shuffle(short arr[PUZZLE_SIZE])
{
    int a, b, c;
    
    for (int i = 0; i < 30; i++) {
        a = (int)(((double)PUZZLE_SIZE)*rand()/(RAND_MAX+1.0));
        if ((a < 0) || (a >= PUZZLE_SIZE)) {
            std::cerr << "Index error" << std::endl;
            exit(1);
        }
        b = (int)(((double)PUZZLE_SIZE)*rand()/(RAND_MAX+1.0));        
        if ((b < 0) || (b >= PUZZLE_SIZE)) {
            std::cerr << "Index error" << std::endl;
            exit(1);
        }
        c = arr[a];
        arr[a] = arr[b];
        arr[b] = c;
    }
}


static bool canSolve(SolvedPuzzle &puzzle, Rules &rules)
{
    Possibilities pos;
    bool changed = false;
    
    do {
        changed = false;
        for (Rules::iterator i = rules.begin(); i != rules.end(); i++) {
            Rule *rule = *i;
            if (rule->apply(pos)) {
                changed = true;
                if (! pos.isValid(puzzle)) {
std::cout << "after error:" << std::endl;
pos.print();
                    throw Exception(L"Invalid possibilities after rule " +
                        rule->getAsText());
                }
            }
        }
    } while (changed);

    bool res = pos.isSolved();
    return res;
}


static void removeRules(SolvedPuzzle &puzzle, Rules &rules)
{
    bool possible;
    
    do {
        possible = false;
        for (Rules::iterator i = rules.begin(); i != rules.end(); i++) {
            Rule *rule = *i;
            Rules excludedRules = rules;
            excludedRules.remove(rule);
            if (canSolve(puzzle, excludedRules)) {
                possible = true;
                rules.remove(rule);
                delete rule;
                break;
            }
        }
    } while (possible);
}


static void genRules(SolvedPuzzle &puzzle, Rules &rules)
{
    bool rulesDone = false;

    do {
        Rule *rule = genRule(puzzle);
        if (rule) {
            std::wstring s = rule->getAsText();
            for (std::list<Rule*>::iterator i = rules.begin(); 
                    i != rules.end(); i++) 
                if ((*i)->getAsText() == s) {
                    delete rule;
                    rule = NULL;
                    break;
                }
            if (rule) {
//printf("adding rule %s\n", rule->getAsText().c_str());
                rules.push_back(rule);
                rulesDone = canSolve(puzzle, rules);
            }
        }
    } while (! rulesDone);
}


/*static void printPuzzle(SolvedPuzzle &puzzle)
{
    for (int i = 0; i < PUZZLE_SIZE; i++) {
        char prefix = 'A' + i;
        for (int j = 0; j < PUZZLE_SIZE; j++) {
            if (j)
                std::cout << "  ";
            std::cout << prefix << puzzle[i][j];
        }
        std::cout << std::endl;
    }
}


static void printRules(Rules &rules)
{
    for (Rules::iterator i = rules.begin(); i != rules.end(); i++)
        std::cout << (*i)->getAsText() << std::endl;;
}*/


void genPuzzle(SolvedPuzzle &puzzle, Rules &rules)
{
    for (int i = 0; i < PUZZLE_SIZE; i++) {
        for (int j = 0; j < PUZZLE_SIZE; j++) 
            puzzle[i][j] = j + 1;
        shuffle(puzzle[i]);
    }

    genRules(puzzle, rules);
    removeRules(puzzle, rules);
//printPuzzle(puzzle);
//printRules(rules);
}


void openInitial(Possibilities &possib, Rules &rules)
{
    for (Rules::iterator i = rules.begin(); i != rules.end(); i++) {
        Rule *r = *i;
        if (r->applyOnStart())
            r->apply(possib);
    }
}


void getHintsQty(Rules &rules, int &vert, int &horiz)
{
    vert = 0;
    horiz = 0;

    for (Rules::iterator i = rules.begin(); i != rules.end(); i++) {
        Rule::ShowOptions so = (*i)->getShowOpts();
        switch (so) {
            case Rule::SHOW_VERT: vert++; break;
            case Rule::SHOW_HORIZ: horiz++; break;
            default: ;
        }
    }
}

void savePuzzle(SolvedPuzzle &puzzle, std::ostream &stream)
{
    for (int row = 0; row < PUZZLE_SIZE; row++)
        for (int col = 0; col < PUZZLE_SIZE; col++)
            writeInt(stream, puzzle[row][col]);
}

void loadPuzzle(SolvedPuzzle &puzzle, std::istream &stream)
{
    for (int row = 0; row < PUZZLE_SIZE; row++)
        for (int col = 0; col < PUZZLE_SIZE; col++)
            puzzle[row][col] = readInt(stream);
}


Rule* getRule(Rules &rules, int no)
{
    int j = 0;
    for (Rules::iterator i = rules.begin(); i != rules.end(); i++) {
        if (j == no)
            return *i;
        j++;
    }
    throw Exception(L"Rule is not found");
}


/*int main(int argc, char *argv[])
{
    srand(time(NULL));
    
    Rules rules;
    Puzzle puzzle;
    
    genPuzzle(puzzle, rules);
    printPuzzle(puzzle);
    printRules(rules);
    
    return 0;
}*/

