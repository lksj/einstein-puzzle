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


#ifndef __RANDOM_H__
#define __RANDOM_H__


class Random
{
    private:
#define RAND_N 624
        unsigned long mt[RAND_N]; /* the array for the state vector  */
        int mti; /* mti==RAND_N+1 means mt[RAND_N] is not initialized */
    
    public:
        Random();
        explicit Random(unsigned long int seed);
        Random(int keys[], int length);
        ~Random();

    public:
        /* generates a random number on [0,0xffffffff]-interval */
        unsigned long int genInt32();
        /* generates a random number on [0,1]-real-interval */
        double genReal2(); 
        /* generate integer random number on [0, range) int interval */
        int genInt(int range);

    private:
        void initLong(unsigned long int s);
};


#endif

