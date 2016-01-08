#include "puzzle.h"
#include "main.h"
#include "utils.h"
#include "sound.h"


#define FIELD_OFFSET_X    12
#define FIELD_OFFSET_Y    68
#define FIELD_GAP_X       4
#define FIELD_GAP_Y       4
#define FIELD_TILE_WIDTH  48
#define FIELD_TILE_HEIGHT 48


Puzzle::Puzzle(IconSet &is, SolvedPuzzle &s, Possibilities *p):
                        iconSet(is), solved(s)
{
    possib = p;
 
    reset();
}


Puzzle::~Puzzle()
{
}

void Puzzle::reset()
{
    valid = true;
    win = false;

    int x, y;
    SDL_GetMouseState(&x, &y);
    getCellNo(x, y, hCol, hRow, subHNo);
}

void Puzzle::draw()
{
    for (int i = 0; i < PUZZLE_SIZE; i++)
        for (int j = 0; j < PUZZLE_SIZE; j++)
            drawCell(i, j, true);
}
    
void Puzzle::drawCell(int col, int row, bool addToUpdate)
{
    int posX = FIELD_OFFSET_X + col * (FIELD_TILE_WIDTH + FIELD_GAP_X);
    int posY = FIELD_OFFSET_Y + row * (FIELD_TILE_HEIGHT + FIELD_GAP_Y);

    if (possib->isDefined(col, row)) {
        int element = possib->getDefined(col, row);
        if (element > 0)
            screen.draw(posX, posY, iconSet.getLargeIcon(row, element, 
                        (hCol == col) && (hRow == row)));
    } else {
        screen.draw(posX, posY, iconSet.getEmptyFieldIcon());
        int x = posX;
        int y = posY + (FIELD_TILE_HEIGHT / 6);
        for (int i = 0; i < 6; i++) {
            if (possib->isPossible(col, row, i + 1))
                screen.draw(x, y, iconSet.getSmallIcon(row, i + 1, 
                            (hCol == col) && (hRow == row) && (i + 1 == subHNo)));
            if (i == 2) {
                x = posX;
                y += (FIELD_TILE_HEIGHT / 3);
            } else
                x += (FIELD_TILE_WIDTH / 3);
        }
    }
    if (addToUpdate)
        screen.addRegionToUpdate(posX, posY, FIELD_TILE_WIDTH, 
                FIELD_TILE_HEIGHT);
}


void Puzzle::drawRow(int row, bool addToUpdate)
{
    for (int i = 0; i < PUZZLE_SIZE; i++)
        drawCell(i, row, addToUpdate);
}


bool Puzzle::onMouseButtonDown(int button, int x, int y)
{
    int col, row, element;

    if (! getCellNo(x, y, col, row, element))
        return false;
    
    if (! possib->isDefined(col, row)) {
        /*if (button == 3) {
            for (int i = 1; i <= PUZZLE_SIZE; i++)
                possib->makePossible(col, row, i);
            drawCell(col, row);
        }
    } else {*/
        if (element == -1)
            return false;
        if (button == 1) {
            if (possib->isPossible(col, row, element)) {
                possib->set(col, row, element);
                sound->play(L"laser.wav");
            }
        } else if (button == 3) {
            if (possib->isPossible(col, row, element)) {
                possib->exclude(col, row, element);
                sound->play(L"whizz.wav");
            }
            /*else
                possib->makePossible(col, row, element);*/
        }
        drawRow(row);
    }

    bool valid = possib->isValid(solved);
    if (! valid)
        onFail();
    else
        if (possib->isSolved() && valid)
            onVictory();
    
    return true;
}


void Puzzle::onFail()
{
    if (failCommand)
        failCommand->doAction();
}


void Puzzle::onVictory()
{
    if (winCommand)
        winCommand->doAction();
}

bool Puzzle::getCellNo(int x, int y, int &col, int &row, int &subNo)
{
    col = row = subNo = -1;
    
    if (! isInRect(x, y, FIELD_OFFSET_X, FIELD_OFFSET_Y, 
                (FIELD_TILE_WIDTH + FIELD_GAP_X) * PUZZLE_SIZE,
                (FIELD_TILE_HEIGHT + FIELD_GAP_Y) * PUZZLE_SIZE))
        return false;

    x = x - FIELD_OFFSET_X;
    y = y - FIELD_OFFSET_Y;

    col = x / (FIELD_TILE_WIDTH + FIELD_GAP_X);
    if (col * (FIELD_TILE_WIDTH + FIELD_GAP_X) + FIELD_TILE_WIDTH < x)
        return false;
    row = y / (FIELD_TILE_HEIGHT + FIELD_GAP_Y);
    if (row * (FIELD_TILE_HEIGHT + FIELD_GAP_Y) + FIELD_TILE_HEIGHT < y)
        return false;

    x = x - col * (FIELD_TILE_WIDTH + FIELD_GAP_X);
    y = y - row * (FIELD_TILE_HEIGHT + FIELD_GAP_Y) 
        - FIELD_TILE_HEIGHT / 6;
    if ((y < 0) || (y >= (FIELD_TILE_HEIGHT / 3) * 2))
        return true;
    int cCol = x / (FIELD_TILE_WIDTH / 3);
    if (cCol >= 3) {
        col = row = -1;
        return false;
    }
    int cRow = y / (FIELD_TILE_HEIGHT / 3);
    subNo = cRow * 3 + cCol + 1;
    
    return true;
}

bool Puzzle::onMouseMove(int x, int y)
{
    int oldCol = hCol;
    int oldRow = hRow;
    int oldElement = subHNo;
    
    getCellNo(x, y, hCol, hRow, subHNo);
    if ((hCol != oldCol) || (hRow != oldRow) || (subHNo != oldElement)) {
        if ((oldCol != -1) && (oldRow != -1))
            drawCell(oldCol, oldRow);
        if ((hCol != -1) && (hRow != -1))
            drawCell(hCol, hRow);
    }
    
    return false;
}

void Puzzle::setCommands(Command *win, Command *fail)
{
    winCommand = win;
    failCommand = fail;
}

