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

