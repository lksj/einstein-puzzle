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


#ifndef __TOPSCORES_H__
#define __TOPSCORES_H__


#include <list>
#include <string>
#include "widgets.h"


#define MAX_SCORES 10


class TopScores
{
    public:
        typedef struct {
            std::wstring name;
            int score;
        } Entry;
        typedef std::list<Entry> ScoresList;
   
    private:
        ScoresList scores;
        bool modifed;
        
    public:
        TopScores();
        ~TopScores();

    public:
        int add(const std::wstring &name, int scores);
        void save();
        ScoresList& getScores();
        int getMaxScore();
        bool isFull() { return scores.size() >= MAX_SCORES; }
};


void showScoresWindow(Area *area, TopScores *scores, int highlightPos=-1);
std::wstring enterNameDialog(Area *area);


#endif

