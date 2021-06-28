// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-08-03 by Jordan Evens <jordan.evens@gmail.com>

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


Puzzle::Puzzle(IconSet &is, SolvedPuzzle &s, Possibilities *p)
    : possib(p), iconSet(is), valid(true), win(false), solved(s),
        hCol(-1), hRow(-1), subHNo(-1), winCommand(nullptr), failCommand(nullptr)
{
    reset();
}


Puzzle::~Puzzle() = default;

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
    const int posX = FIELD_OFFSET_X + col * (FIELD_TILE_WIDTH + FIELD_GAP_X);
    const int posY = FIELD_OFFSET_Y + row * (FIELD_TILE_HEIGHT + FIELD_GAP_Y);

    if (possib->isDefined(col, row)) {
        const int element = possib->getDefined(col, row);
        if (element > 0)
        {
            screen.drawScaled(posX, posY, iconSet.getLargeIcon(row, element, (hCol == col) && (hRow == row)));
        }
    } else {
        SDL_Surface* emptyFieldIcon = iconSet.getEmptyFieldIcon();
        SDL_Surface* newTile = scaleUp(emptyFieldIcon);
        SDL_Rect src = { 0, 0, newTile->w / 3, newTile->h / 3 };
        SDL_Rect dst = { 0, (FIELD_TILE_HEIGHT / 6), src.w, src.h };
        for (int i = 0; i < 6; i++) {
            if (possib->isPossible(col, row, i + 1))
            {
                SDL_Surface *origIcon = iconSet.getLargeIcon(row, i + 1, (hCol == col) && (hRow == row) && (i + 1 == subHNo));
                SDL_Surface *smallIcon = scaleTo(origIcon, src.w, src.h);
                for (int w = 0; w < smallIcon->w; w++)
                {
                    setPixel(smallIcon, w, 0, 0, 0, 0);
                    setPixel(smallIcon, w, smallIcon->h-1, 0, 0, 0);
                }
                for (int h = 0; h < smallIcon->h; h++)
                {
                    setPixel(smallIcon, 0, h, 0, 0, 0);
                    setPixel(smallIcon, smallIcon->w-1, h, 0, 0, 0);
                }
                SDL_BlitSurface(smallIcon, &src, newTile, &dst);
                SDL_FreeSurface(smallIcon);
            }
            if (i == 2) {
                dst.x = 0;
                dst.y += src.h;
            } else
                dst.x += src.w;
        }
        screen.draw(scaleUp(posX), scaleUp(posY), newTile);
        SDL_FreeSurface(newTile);
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

    const bool valid = possib->isValid(solved);
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

    x = scaleDown(x) - FIELD_OFFSET_X;
    y = scaleDown(y) - FIELD_OFFSET_Y;

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

