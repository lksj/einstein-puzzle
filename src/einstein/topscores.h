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
        bool isFull() { return scores.size() >= MAX_SCORES; };
};


void showScoresWindow(Area *area, TopScores *scores, int highlightPos=-1);
std::wstring enterNameDialog(Area *area);


#endif

