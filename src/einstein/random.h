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
        Random(unsigned long int seed);
        Random(int keys[], int length);
        ~Random();

    public:
        /* generates a random number on [0,0xffffffff]-interval */
        unsigned long int genInt32();
        /* generates a random number on [0,0x7fffffff]-interval */
        long int genInt31();
        /* generates a random number on [0,1]-real-interval */
        double genReal1(); 
        /* generates a random number on [0,1)-real-interval */
        double genReal2(); 
        /* generates a random number on (0,1)-real-interval */
        double genReal3(); 
        /* generates a random number on [0,1) with 53-bit resolution*/
        double genReal53(); 
        /* generate integer random number on [0, range) int interval */
        int genInt(int range);

    private:
        void initLong(unsigned long int s);
};


#endif

