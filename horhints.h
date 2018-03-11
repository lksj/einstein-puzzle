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


#ifndef __HORHINTS_H__
#define __HORHINTS_H__


#include <vector>
#include "iconset.h"
#include "puzgen.h"
#include "widgets.h"



class HorHints: public Widget
{
    private:
        IconSet &iconSet;
        typedef std::vector<Rule*> RulesArr;
        RulesArr rules;
        RulesArr excludedRules;
        std::vector<int> numbersArr;
        bool showExcluded;
        int highlighted;
    
    public:
        HorHints(IconSet &is, Rules &rules);
        HorHints(IconSet &is, Rules &rules, std::istream &stream);

    public:
        void draw() override;
        void drawCell(int col, int row, bool addToUpdate=true);
        bool onMouseButtonDown(int button, int x, int y) override;
        void toggleExcluded();
        int getRuleNo(int x, int y);
        bool onMouseMove(int x, int y) override;
        bool isActive(int ruleNo);
        void save(std::ostream &stream);
        void reset(Rules &rules);
};


#endif

