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
    
    int i, j, k;
    initLong(19650218UL);
    i=1; j=0;
    k = (RAND_N>key_length ? RAND_N : key_length);
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



Random::~Random()
{
}

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
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= RAND_N) { /* generate N words at one time */
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

/* generates a random number on [0,0x7fffffff]-interval */
long Random::genInt31()
{
    return (long)(genInt32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double Random::genReal1(void)
{
    return genInt32()*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double Random::genReal2()
{
    return genInt32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double Random::genReal3(void)
{
    return (((double)genInt32()) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double Random::genReal53(void)
{
    unsigned long a=genInt32()>>5, b=genInt32()>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
}

/* generate integer random number on [0, range) int interval */
int Random::genInt(int range)
{
    int v = (int)(genReal2() * range);
if (v == range) printf("ooops!\n");
    return v;
}

