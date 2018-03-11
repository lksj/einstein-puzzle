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


#include "random.h"
#include "utils.h"

/* Period parameters */  
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */


Random::Random()
{
    mti = RAND_N+1; /* mti==RAND_N+1 means mt[RAND_N] is not initialized */
    struct timeval tv;
    gettimeofday(&tv);
    unsigned long long int s = tv.tv_sec * 1000000 + tv.tv_usec;
    initLong(s);
}

Random::Random(unsigned long int seed)
{
    mti = RAND_N+1; /* mti==RAND_N+1 means mt[RAND_N] is not initialized */
    initLong(seed);
}


/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
Random::Random(int init_key[], int key_length)
{
    mti = RAND_N+1; /* mti==RAND_N+1 means mt[RAND_N] is not initialized */
    
    initLong(19650218UL);
    int i = 1;
    int j = 0;
    int k = (RAND_N>key_length ? RAND_N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=RAND_N) { mt[0] = mt[RAND_N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=RAND_N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=RAND_N) { mt[0] = mt[RAND_N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}



Random::~Random() = default;

/* initializes mt[RAND_N] with a seed */
void Random::initLong(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<RAND_N; mti++) {
        mt[mti] = 
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}


/* generates a random number on [0,0xffffffff]-interval */
unsigned long Random::genInt32(void)
{
    unsigned long y;

    if (mti >= RAND_N) { /* generate N words at one time */
        static unsigned long mag01[2]={0x0UL, MATRIX_A};
        /* mag01[x] = x * MATRIX_A  for x=0,1 */
        int kk;

        if (mti == RAND_N+1)   /* if init_genrand() has not been called, */
            initLong(5489UL); /* a default initial seed is used */

        for (kk=0;kk<RAND_N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<RAND_N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-RAND_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[RAND_N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[RAND_N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,1)-real-interval */
double Random::genReal2()
{
    return genInt32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generate integer random number on [0, range) int interval */
int Random::genInt(int range)
{
    int v = (int)(genReal2() * range);
if (v == range) printf("ooops!\n");
    return v;
}

