#ifndef __PUZZLE_H__
#define __PUZZLE_H__


#include "iconset.h"
#include "puzgen.h"
#include "widgets.h"


class Puzzle: public Widget
{
    private:
        Possibilities *possib;
        IconSet &iconSet;
        bool valid;
        bool win;
        SolvedPuzzle &solved;
        int hCol, hRow;
        int subHNo;
        Command *winCommand, *failCommand;
        
    public:
        Puzzle(IconSet &is, SolvedPuzzle &solved, Possibilities *possib);
        virtual ~Puzzle();

    public:
        virtual void draw();
        void drawRow(int row, bool addToUpdate=true);
        void drawCell(int col, int row, bool addToUpdate=true);
        Possibilities* getPossibilities() { return possib; };
        virtual bool onMouseButtonDown(int button, int x, int y);
        bool isValid() const { return valid; };
        bool victory() const { return win; };
        void onFail();
        void onVictory();
        bool getCellNo(int x, int y, int &col, int &row, int &subNo);
        virtual bool onMouseMove(int x, int y);
        void setCommands(Command *winCommand, Command *failCommand);
        void reset();
};


#endif

