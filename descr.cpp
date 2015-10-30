#include "descr.h"

#include <string>
#include <list>
#include <vector>
#include <map>

#include "widgets.h"
#include "unicode.h"
#include "messages.h"
#include "convert.h"
#include "utils.h"
#include "tokenizer.h"


#define WIDTH		600
#define HEIGHT		500
#define CLIENT_WIDTH	570
#define CLIENT_HEIGHT	390
#define START_X		115
#define START_Y		100


class TextPage
{
    private:
        std::vector<Widget*> widgets;

    public:
        TextPage() { };
        ~TextPage();

    public:
        Widget* getWidget(int no) { return widgets[no]; };
        int getWidgetsCount() const { return widgets.size(); };
        void add(Widget *widget) { widgets.push_back(widget); };
        bool isEmpty() const { return ! widgets.size(); };
};


class TextParser
{
    private:
        Tokenizer tokenizer;
        std::vector<TextPage*> pages;
        Font &font;
        int spaceWidth;
        int charHeight;
        std::map<std::wstring, SDL_Surface*> images;
        int offsetX;
        int offsetY;
        int pageWidth;
        int pageHeight;

    public:
        TextParser(const std::wstring &text, Font &font,
                int x, int y, int width, int height);
        ~TextParser();

    public:
        TextPage* getPage(unsigned int no);

    private:
        void addLine(TextPage *page, std::wstring &line, int &curPosY, 
                int &lineWidth);
        void parseNextPage();
        bool isImage(const std::wstring &name);
        std::wstring keywordToImage(const std::wstring &name);
        SDL_Surface* getImage(const std::wstring &name);
};


class CursorCommand;


// Otobrazhaet pravila igry na ekran
class Description
{
    private:
        typedef std::list<Widget *> WidgetsList;
        WidgetsList widgets;

        CursorCommand *prevCmd;		// Sobytie na nazhatie knopki <PREV>
        CursorCommand *nextCmd;		// Sobytie na nazhatie knopki <PREV>

        Area area;			// Mesto gde risovat'
        //std::vector<RulesPage *> pages;	// Spisok stranits teksta
        unsigned int currentPage;	// Tekuschaja stranitsa dlja prosmotra

        Font *titleFont;		// Shrift dlja risovanija zagolovka okna
        Font *buttonFont;		// Shrift dlja risovanija knopok v okne
        Font *textFont;			// Shrift dlja risovanija teksta

        unsigned int textHeight;	// Vysota stroki teksta
        TextParser *text;
        
    public:
        Description(Area *parentArea);
        ~Description();

    public:
        void run();
        void updateInfo();	// Vyvodit informatsiju na stranitsu
        TextPage *getPage(unsigned int no) { return text->getPage(no); };

    private:
        void printPage();		// Vyvodit tekuschuju stranitsu pravil
        void deleteWidgets();		// Udaljaet neispol'zuemye metki i kartinki iz oblasti vyvoda informatsii (Area)
};


//Nuzhen pri nazhatii na knopku <NEXT> ili <PREV> v dialoge spiska pravil
class CursorCommand: public Command
{
    private:
        int step;			// Cherez skol'ko stranits listat'
        Description &description;	// Ukazatel' na ob"ekt Description
        unsigned int *value;		// Ukazatel' na tekuschij nomer stranitsy

    public:
        CursorCommand(int step, Description &d, unsigned int *v);
        virtual ~CursorCommand() { };
        
    public:
        virtual void doAction();	// Obrabatyvaet sobytija
};



void showDescription(Area *parentArea)
{
    Description d(parentArea);
    d.run();
}



Description::Description(Area *parentArea)
{
    currentPage = 0;
    //area.add(parentArea, false);
    titleFont = new Font(L"nova.ttf", 26);
    buttonFont = new Font(L"laudcn2.ttf", 14);
    textFont = new Font(L"laudcn2.ttf", 16);
    textHeight = (int)(textFont->getHeight(L"A") * 1.0);
    text = new TextParser(msg(L"rulesText"), *textFont, START_X, START_Y, 
                CLIENT_WIDTH, CLIENT_HEIGHT);
    prevCmd = new CursorCommand(-1, *this, &currentPage);
    nextCmd = new CursorCommand(1, *this, &currentPage);
}

Description::~Description()
{
    deleteWidgets();
    delete text;
    delete titleFont;
    delete buttonFont;
    delete textFont;
}


void Description::deleteWidgets()
{
    for (WidgetsList::iterator i = widgets.begin(); 
            i != widgets.end(); i++)
        area.remove(*i);
    widgets.clear();
}


void Description::updateInfo()
{
    deleteWidgets();
    printPage();
    area.draw();
}

void Description::run()
{
    area.add(new Window(100, 50, WIDTH, HEIGHT, L"blue.bmp"));
    area.add(new Label(titleFont, 250, 60, 300, 40, Label::ALIGN_CENTER, Label::ALIGN_MIDDLE, 255, 255, 0, msg(L"rules")));
    area.add(new Button(110, 515, 80, 25, buttonFont, 255, 255, 0, L"blue.bmp", msg(L"prev"), prevCmd));
    area.add(new Button(200, 515, 80, 25, buttonFont, 255, 255, 0, L"blue.bmp", msg(L"next"), nextCmd));
    ExitCommand exitCmd(area);
    area.add(new Button(610, 515, 80, 25, buttonFont, 255, 255, 0, L"blue.bmp", msg(L"close"), &exitCmd));
    area.add(new KeyAccel(SDLK_ESCAPE, &exitCmd));
    printPage();
    area.run();
}

void Description::printPage()
{
    TextPage *page = text->getPage(currentPage);
    if (! page)
        return;
    int len = page->getWidgetsCount();
    for (int i = 0; i < len; i++) {
        Widget *w = page->getWidget(i);
        if (w) {
            widgets.push_back(w);
            area.add(w);
        }
    }
}

CursorCommand::CursorCommand(int s, Description &d, unsigned int *v):
                description(d)
{
    step = s;
    value = v;
}

void CursorCommand::doAction()
{
    if ((! *value) && (0 > step))
        return;
    unsigned int newPageNo = *value + step;
    TextPage *page = description.getPage(newPageNo);
    if (page) {
        *value = newPageNo;
        description.updateInfo();
    }
}


TextPage::~TextPage()
{
    for (std::vector<Widget*>::iterator i = widgets.begin();
            i != widgets.end(); i++)
        delete *i;
}


TextParser::TextParser(const std::wstring &text, Font &font,
        int x, int y, int width, int height): tokenizer(text), font(font)
{
    spaceWidth = font.getWidth(L' ');
    charHeight = font.getWidth(L'A');
    offsetX = x;
    offsetY = y;
    pageWidth = width;
    pageHeight = height;
}


TextParser::~TextParser()
{
    for (std::vector<TextPage*>::iterator i = pages.begin();
            i != pages.end(); i++)
        delete *i;
    for (std::map<std::wstring, SDL_Surface*>::iterator i = images.begin();
            i != images.end(); i++)
        delete (*i).second;
}


void TextParser::addLine(TextPage *page, std::wstring &line, int &curPosY,
        int &lineWidth)
{
    if (0 < line.length()) {
        page->add(new Label(&font, offsetX, offsetY + curPosY,
                    255,255,255, line, false));
        line.clear();
        curPosY += 10 + charHeight;
        lineWidth = 0;
    }
}

bool TextParser::isImage(const std::wstring &name)
{
    int len = name.length();
    return (3 < len) && (L'$' == name[0]) && (L'$' == name[len - 1]);
}

std::wstring TextParser::keywordToImage(const std::wstring &name)
{
    return name.substr(1, name.length() - 2);
}

SDL_Surface* TextParser::getImage(const std::wstring &name)
{
    SDL_Surface *img = images[name];
    if (! img) {
        img = loadImage(name);
        images[name] = img;
    }
    return img;
}

void TextParser::parseNextPage()
{
    if (tokenizer.isFinished())
        return;
    
    int curPosY = 0;
    int lineWidth = 0;
    TextPage *page = new TextPage();
    std::wstring line;

    while (true) {
        Token t = tokenizer.getNextToken();
        if (Token::Eof == t.getType())
            break;
        if (Token::Para == t.getType()) {
            if (0 < line.length())
                addLine(page, line, curPosY, lineWidth);
            if (! page->isEmpty())
                curPosY += 10;
        } else if (Token::Word == t.getType()) {
            const std::wstring &word = t.getContent();
            if (isImage(word)) {
                addLine(page, line, curPosY, lineWidth);
                SDL_Surface *image = getImage(keywordToImage(word));
                if ((image->h + curPosY < pageHeight) || page->isEmpty()) {
                    int x = offsetX + (pageWidth - image->w) / 2;
                    page->add(new Picture(x, offsetY + curPosY, image));
                    curPosY += image->h;
                } else {
                    tokenizer.unget(t);
                    break;
                }
            } else {
                int width = font.getWidth(word);
                if (lineWidth + width > pageWidth) {
                    if (! lineWidth) {
                        line = word;
                        addLine(page, line, curPosY, lineWidth);
                    } else {
                        addLine(page, line, curPosY, lineWidth);
                        if (curPosY >= pageHeight) {
                            tokenizer.unget(t);
                            break;
                        }
                        line = word;
                        lineWidth = width;
                    }
                } else {
                    lineWidth += width;
                    if (line.size()) {
                        line += L' ';
                        lineWidth += spaceWidth;
                    }
                    line += word;
                }
            }
        }
        if (curPosY >= pageHeight)
            break;
    }
    addLine(page, line, curPosY, lineWidth);
    if (! page->isEmpty())
        pages.push_back(page);
    else
        delete page;
}


TextPage* TextParser::getPage(unsigned int no)
{
    while ((! tokenizer.isFinished()) && (pages.size() <= no))
        parseNextPage();
    if (pages.size() <= no)
        return NULL;
    else
        return pages[no];
}

