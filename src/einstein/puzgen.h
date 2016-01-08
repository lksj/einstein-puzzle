#ifndef __PUZGEN_H__
#define __PUZGEN_H__


#include <string>
#include <list>
#include <iostream>
#include "iconset.h"


#define PUZZLE_SIZE 6


typedef short SolvedPuzzle[PUZZLE_SIZE][PUZZLE_SIZE];


class Possibilities
{
    private:
        short pos[PUZZLE_SIZE][PUZZLE_SIZE][PUZZLE_SIZE];
    
    public:
        Possibilities();
        Possibilities(std::istream &stream);

    public:
        void exclude(int col, int row, int element);
        void set(int col, int row, int element);
        bool isPossible(int col, int row, int element);
        bool isDefined(int col, int row);
        int getDefined(int col, int row);
        int getPosition(int row, int element);
        bool isSolved();
        void print();
        bool isValid(SolvedPuzzle &puzzle);
        void makePossible(int col, int row, int element);
        void save(std::ostream &stream);
        void reset();
        void checkSingles(int row);
};


class Rule
{
    public:
        typedef enum {
            SHOW_VERT,
            SHOW_HORIZ,
            SHOW_NOTHING
        } ShowOptions;
    
    public:
        virtual ~Rule() { };

    public:
        virtual std::wstring getAsText() = 0;
        virtual bool apply(Possibilities &pos) = 0;
        virtual bool applyOnStart() { return false; };
        virtual ShowOptions getShowOpts() { return SHOW_NOTHING; };
        virtual void draw(int x, int y, IconSet &iconSet, bool highlight) = 0;
        virtual void save(std::ostream &stream) = 0;
};


typedef std::list<Rule*> Rules;


void genPuzzle(SolvedPuzzle &puzzle, Rules &rules);
void openInitial(Possibilities &possib, Rules &rules);
Rule* genRule(SolvedPuzzle &puzzle);
void getHintsQty(Rules &rules, int &vert, int &horiz);
Rule* getRule(Rules &rules, int no);

void savePuzzle(SolvedPuzzle &puzzle, std::ostream &stream);
void loadPuzzle(SolvedPuzzle &puzzle, std::istream &stream);
void saveRules(Rules &rules, std::ostream &stream);
void loadRules(Rules &rules, std::istream &stream);


#endif

