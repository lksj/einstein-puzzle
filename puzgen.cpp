// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

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


#include <cstdlib>
#include <cstdio>
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
        for (auto& po : pos)
                for (int element = 0; element < PUZZLE_SIZE; element++)
                po[row][element] = readInt(stream);
}

void Possibilities::reset()
{
    for (auto& po : pos)
        for (auto& j : po)
            for (int k = 0; k < PUZZLE_SIZE; k++)
                j[k] = k + 1;
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

void Possibilities::print()
{
    for (int row = 0; row < PUZZLE_SIZE; row++) {
        std::cout << (char)('A' + row) << " ";
        for (auto& po : pos)
        {
            for (int i = 0; i < PUZZLE_SIZE; i++)
                if (po[row][i])
                    std::cout << po[row][i];
                else
                    std::cout << " ";
            std::cout << "   ";
        }
        std::cout << std::endl;
    }
}

void Possibilities::save(std::ostream &stream)
{
    for (int row = 0; row < PUZZLE_SIZE; row++)
        for (auto& po : pos)
                for (int element = 0; element < PUZZLE_SIZE; element++)
                writeInt(stream, po[row][element]);
}


static void shuffle(short arr[PUZZLE_SIZE])
{
    for (int i = 0; i < 30; i++) {
        int a = (int)(((double)PUZZLE_SIZE)*rand()/(RAND_MAX+1.0));
        if ((a < 0) || (a >= PUZZLE_SIZE)) {
            std::cerr << "Index error" << std::endl;
            exit(1);
        }
        int b = (int)(((double)PUZZLE_SIZE)*rand()/(RAND_MAX+1.0));
        if ((b < 0) || (b >= PUZZLE_SIZE)) {
            std::cerr << "Index error" << std::endl;
            exit(1);
        }
        int c = arr[a];
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
        for (auto rule : rules)
        {
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
        for (Rules::iterator i = rules.begin(); i != rules.end(); ++i) {
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
            for (auto& i : rules)
                if (i->getAsText() == s) {
                    delete rule;
                    rule = nullptr;
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
    for (auto& i : puzzle)
    {
        for (int j = 0; j < PUZZLE_SIZE; j++)
            i[j] = j + 1;
        shuffle(i);
    }

    genRules(puzzle, rules);
    removeRules(puzzle, rules);
//printPuzzle(puzzle);
//printRules(rules);
}


void openInitial(Possibilities &possib, Rules &rules)
{
    for (auto r : rules)
    {
        if (r->applyOnStart())
            r->apply(possib);
    }
}


void getHintsQty(Rules &rules, int &vert, int &horiz)
{
    vert = 0;
    horiz = 0;

    for (auto& rule : rules)
    {
        Rule::ShowOptions so = rule->getShowOpts();
        switch (so) {
            case Rule::SHOW_VERT: vert++; break;
            case Rule::SHOW_HORIZ: horiz++; break;
            default: ;
        }
    }
}

void savePuzzle(SolvedPuzzle &puzzle, std::ostream &stream)
{
    for (auto& row : puzzle)
        for (short col : row)
            writeInt(stream, col);
}

void loadPuzzle(SolvedPuzzle &puzzle, std::istream &stream)
{
    for (auto& row : puzzle)
        for (short& col : row)
            col = readInt(stream);
}


Rule* getRule(Rules &rules, int no)
{
    int j = 0;
    for (auto& rule : rules)
    {
        if (j == no)
            return rule;
        j++;
    }
    throw Exception(L"Rule is not found");
}


/*int main(int argc, char *argv[])
{
    srand(time(nullptr));
    
    Rules rules;
    Puzzle puzzle;
    
    genPuzzle(puzzle, rules);
    printPuzzle(puzzle);
    printRules(rules);
    
    return 0;
}*/

