#include "topscores.h"
#include "storage.h"
#include "utils.h"
#include "font.h"
#include "convert.h"
#include "messages.h"


TopScores::TopScores()
{
    Storage *storage = getStorage();
    
    for (int i = 0; i < MAX_SCORES; i++) {
        int score = storage->get(L"top_score_" + toString(i), -1);
        if (score < 0)
            break;
        std::wstring name = storage->get(L"top_name_" + toString(i), L"");
        add(name, score);
    }
    
    modifed = false;
}


TopScores::~TopScores()
{
    save();
}

int TopScores::add(const std::wstring &name, int score)
{
    if (score >= getMaxScore() || (scores.size() < 1)) {
        if (! isFull()) {
            Entry e = { name, score };
            scores.push_back(e);
            modifed = true;
            return scores.size() - 1;
        }
        return -1;
    }
    
    int pos = 0;
    for (ScoresList::iterator i = scores.begin(); i != scores.end(); i++) {
        Entry &e = *i;
        if (e.score > score) {
            Entry ne = { name, score };
            scores.insert(i, ne);
            modifed = true;
            break;
        }
        pos++;
    }

    while (scores.size() > MAX_SCORES) {
        modifed = true;
        scores.erase(--scores.end());
    }

    return modifed ? pos : -1;
}

void TopScores::save()
{
    if (! modifed)
        return;

    Storage *storage = getStorage();
    int no = 0;
    
    for (ScoresList::iterator i = scores.begin(); i != scores.end(); i++) {
        Entry &e = *i;
        storage->set(L"top_name_" + toString(no), e.name);
        storage->set(L"top_score_" + toString(no), e.score);
        no++;
    }
    
    storage->flush();
    modifed = false;
}

TopScores::ScoresList& TopScores::getScores()
{
    return scores;
}

int TopScores::getMaxScore()
{
    if (scores.size() < 1)
        return -1;
    ScoresList::iterator i = scores.end();
    i--;
    return (*i).score;
}


class ScoresWindow: public Window
{
    public:
        ScoresWindow(int x, int y, TopScores *scores, int highlight);
};


ScoresWindow::ScoresWindow(int x, int y, TopScores *scores, int highlight): 
                Window(x, y, 320, 350, L"blue.bmp")
{
    Font titleFont(L"nova.ttf", 26);
    Font entryFont(L"laudcn2.ttf", 14);
    Font timeFont(L"luximb.ttf", 14);
    
    std::wstring txt = msg(L"topScores");
    int w = titleFont.getWidth(txt);
    titleFont.draw(background, (320 - w) / 2, 15, 255,255,0, true, txt);

    TopScores::ScoresList &list = scores->getScores();
    int no = 1;
    int pos = 70;
    for (TopScores::ScoresList::iterator i = list.begin(); 
            i != list.end(); i++) 
    {
        TopScores::Entry &e = *i;
        std::wstring s(toString(no) + L".");
        int w = entryFont.getWidth(s);
        int c = ((no - 1) == highlight) ? 0 : 255;
        entryFont.draw(background, 30 - w, pos, 255,255,c, true, s);
        SDL_Rect rect = { 40, pos-20, 180, 40 };
        SDL_SetClipRect(background, &rect);
        entryFont.draw(background, 40, pos, 255,255,c, true, e.name);
        SDL_SetClipRect(background, NULL);
        s = secToStr(e.score);
        w = timeFont.getWidth(s);
        timeFont.draw(background, 305-w, pos, 255,255,c, true, s);
        pos += 20;
        no++;
    }
}


void showScoresWindow(Area *parentArea, TopScores *scores, int highlight)
{
    Area area;

    Font font(L"laudcn2.ttf", 16);
    area.add(parentArea);
    area.add(new ScoresWindow(240, 125, scores, highlight));
    ExitCommand exitCmd(area);
    area.add(new Button(348, 430, 90, 25, &font, 255,255,0, L"blue.bmp", 
                msg(L"ok"), &exitCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
    area.run();
}


std::wstring enterNameDialog(Area *parentArea)
{
    Area area;
    
    Font font(L"laudcn2.ttf", 16);
    area.add(parentArea);
    area.add(new Window(170, 280, 460, 100, L"blue.bmp"));
    Storage *storage = getStorage();
    std::wstring name = storage->get(L"lastName", msg(L"anonymous"));
    area.add(new Label(&font, 180, 300, 255,255,0, msg(L"enterName")));
    area.add(new InputField(350, 300, 270, 26, L"blue.bmp", name, 20,  
                255,255,0,  &font));
    ExitCommand exitCmd(area);
    area.add(new Button(348, 340, 90, 25, &font, 255,255,0, L"blue.bmp", 
                msg(L"ok"), &exitCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
    area.add(new KeyAccel(SDLK_RETURN, &exitCmd));
    area.run();
    storage->set(L"lastName", name);
    return name;
}


