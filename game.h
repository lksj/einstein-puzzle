#ifndef __GAME_H__
#define __GAME_H__


#include <iostream>
#include "puzgen.h"
#include "verthints.h"
#include "horhints.h"
#include "puzzle.h"



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
        Game(std::istream &stream);
        ~Game();

    public:
        SolvedPuzzle& getSolvedPuzzle() { return solvedPuzzle; };
        Rules& getRules() { return rules; };
        Possibilities* getPossibilities() { return possibilities; };
        VertHints* getVerHints() { return verHints; };
        HorHints* getHorHints() { return horHints; };
        void save(std::ostream &stream);
        void run();
        bool isHinted() { return hinted; };
        void setHinted() { hinted = true; };
        void restart();
        void newGame();

    private:
        void deleteRules();
        void pleaseWait();
        void genPuzzle();
        void resetVisuals();
};

#endif

