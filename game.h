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


#ifndef __GAME_H__
#define __GAME_H__


#include "horhints.h"
#include "puzzle.h"
#include "verthints.h"


class Watch;



class Game
{
    private:
        SolvedPuzzle solvedPuzzle;
        Rules rules;
        Possibilities *possibilities;
        VertHints *verHints;
        HorHints *horHints;
        IconSet iconSet;
        Puzzle *puzzle;
        Watch *watch;
        bool hinted;
        SolvedPuzzle savedSolvedPuzzle;
        Rules savedRules;

    public:
        Game();
        explicit Game(std::istream &stream);
        ~Game();

    public:
        SolvedPuzzle& getSolvedPuzzle() { return solvedPuzzle; }
        Rules& getRules() { return rules; }
        Possibilities* getPossibilities() { return possibilities; }
        VertHints* getVerHints() { return verHints; }
        HorHints* getHorHints() { return horHints; }
        void save(std::ostream &stream);
        void run();
        bool isHinted() { return hinted; }
        void setHinted() { hinted = true; }
        void restart();
        void newGame();

    private:
        void deleteRules();
        void pleaseWait();
        void genPuzzle();
        void resetVisuals();
        // prevent generation by compiler
        Game(const Game&);
};

#endif

