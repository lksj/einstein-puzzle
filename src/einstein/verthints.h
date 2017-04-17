#ifndef __VERTHINTS_H__
#define __VERTHINTS_H__


#include <vector>
#include "iconset.h"
#include "puzgen.h"
#include "widgets.h"



class VertHints: public Widget
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
        VertHints(IconSet &is, Rules &rules);
        VertHints(IconSet &is, Rules &rules, std::istream &stream);

    public:
        virtual void draw();
        void drawCell(int col, bool addToUpdate=true);
        virtual bool onMouseButtonDown(int button, int x, int y);
        void toggleExcluded();
        int getRuleNo(int x, int y);
        virtual bool onMouseMove(int x, int y);
        bool isActive(int ruleNo);
        void save(std::ostream &stream);
        void reset(Rules &rules);
};


#endif

