
#include <cstdlib>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <time.h>     
#if GEM5 
    #include "m5op.h"
#endif
#include <inttypes.h>

#if Intel_PCM     
    #include "cpucounters.h"        // Intell PCM monitoring tool
#endif
using namespace std;

/* This is the control panel of the benchmark */
//#define     GEM5            0                   // Running on Gem5 or not
#define     DEBUG           0                   // Print debug results or not
#define     PRINT           0                   // Print results or not
#define     P               8                   // Number of processors
#define     SIM_LIMIT       500024                   // Number of k2 loops to simulate
#define     WARMUP_LIMIT    1                  // Number of k2 loops to use for warmup
#define     N               8*1024		// Square matrix dimension N*N
#define     TILE            16                  // Tile size
#define     NUM_CP          0                   // Not applicable here
//#define     Intel_PCM       1                   // Running on Intel PCM counters or not
#define     NUM_BARRIERS    4                   // Number of barriers
#define     Benchmark_Mode  0                   //0: base, 1: Recompute, 2: Logging, 3: Naive Checkpointing
#define	    NUMBER_OF_TRIALS	3		//Number of times to repeat the whole benchmark (this is to reduce results randomness)
#define     USE_DYNAMIC_ARRAY   1               //This allocates the arrays dynamically using "new". This allows having huge matrices

pthread_mutex_t SyncLock[NUM_BARRIERS];         /* mutex */
pthread_cond_t  SyncCV[NUM_BARRIERS];           /* condition variable */
int             SyncCount[NUM_BARRIERS];        /* number of processors at the barrier so far */

const int n     = N;                            // matrix dimension
const int tile  = TILE;		                    // tile size
int firstTime;                    
struct timespec t1[P], t2[P], d[P];

#if USE_DYNAMIC_ARRAY
alignas(64) int **c, **a, **b;    // the two matrices a and b. c is the resultant matrix
#else
alignas(64) int c[n][n], a[n][n], b[n][n];    // the two matrices a and b. c is the resultant matrix
#endif
alignas(64) int lastJJ[P*16];         // the last computed iteration of k2 loop
alignas(64) int lastII[P*16];         	// the last computed iteration of i loop
alignas(64) int lastKK[P*16];         // the last computed iteration of k2 loop
alignas(64) int lastJJLog[P*16];        // log for the last computed iteration of i loop
alignas(64) int lastIILog[P*16];        // log for the last computed iteration of i loop
alignas(64) int lastKKLog[P*16];       // log for the last computed iteration of k2 loop
alignas(64) int insideTxII[P*16];               // Flag for entering logging region for updating II index
alignas(64) int insideTxKK[P*16];               // Flag for entering logging region for updating KK index
uint64_t start, end;                            // Used for printing the ticks on real system

inline uint64_t rdtsc()
{
    unsigned long a, d;
    asm volatile ("cpuid; rdtsc" : "=a" (a), "=d" (d) : : "ebx", "ecx");
    return a | ((uint64_t)d << 32);
}

void  diff(struct timespec * difference, struct timespec start, struct timespec end)
{
    if ((end.tv_nsec-start.tv_nsec)<0) {
        difference->tv_sec = end.tv_sec-start.tv_sec-1;
        difference->tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        difference->tv_sec = end.tv_sec-start.tv_sec;
        difference->tv_nsec = end.tv_nsec-start.tv_nsec;
    }
}

void Barrier(int idx) {
    int ret;

    pthread_mutex_lock(&SyncLock[idx]); /* Get the thread lock */
    SyncCount[idx]++;
    if(SyncCount[idx] == P) {
        ret = pthread_cond_broadcast(&SyncCV[idx]);
        assert(ret == 0);
    } else {
        ret = pthread_cond_wait(&SyncCV[idx], &SyncLock[idx]); 
        assert(ret == 0);
    }
    pthread_mutex_unlock(&SyncLock[idx]);
}

void Initialize() 
{
    int i, j;

    //    srand (time(NULL));    
    srand48(0);
    for(i=0; i<P; i++) {
        lastJJ[i*16] = (i*(n/P))-tile;
        lastII[i*16] = -tile;
        lastKK[i*16] = -tile;
        lastJJLog[i*16] = (i*(n/P))-tile;
        lastIILog[i*16] = -tile;
        lastKKLog[i*16] = -tile;
        insideTxII[i*16] = 0;
        insideTxKK[i*16] = 0;
    }

#if USE_DYNAMIC_ARRAY
    a = new int*[N];
    b = new int*[N];
    c = new int*[N];
    for(int i=0;i<N;i++)
    {
        a[i] = new int[N];
        b[i] = new int[N];
        c[i] = new int[N];
    }
#endif

    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
           // a[i][j] = drand48();
           // b[i][j] = drand48();
            a[i][j] = 2;
            b[i][j] = 3;
            c[i][j] = 0;
            //            a[i][j] = rand() % 20;
            //            b[i][j] = rand() % 20;
            //            c[i][j] = 0;
        }
    }
}

void PrintC() 
{
    int i, j;

    printf("The C matrix:\n");
    for (i=0; i< n; i++) {
        for (j=0; j< n; j++) 
            printf("%0.2f\t",c[i][j]); 
        printf("\n");
    }
    printf("\n");
}

void checkC() 
{
    int i, j;
    bool passed = true;
    for (i=0; i< n; i++) {
        for (j=0; j< n; j++) 
            //if(c[i][j] != ((SIM_LIMIT*16)*6.0)) printf("error in cell c[%d][%d]. It is %0.2f\n",i,j,c[i][j]); 
            if(c[i][j] != ((n)*6.0)) {printf("error in cell c[%d][%d]. It is %0.2f\n",i,j,c[i][j]);passed = false;} 
    }
    if(passed) printf("Passed all tests!\n");
}

void* multiply(void* tmp) {
    /* each thread has a private version of local variables */
    int     tid = (uintptr_t) tmp; 
    int     i, j, k, i2, j2, k2, r, l;  //iterators
    float     sum;                      //sum of multiplication
    int     firstLoop, lastLoop;

    firstLoop = tid*(n/P);
    lastLoop = firstLoop + (n/P);

    /*////////////////////////////////////////////////////////////////////////////////////////////\*/
    /*****************  Normal (i.e. not Recovery mode) tiled multiplication operation mode  **********************/
    /*////////////////////////////////////////////////////////////////////////////////////////////\*/

    for(int i=firstLoop;i<lastLoop;i++)
        for(int j=0;j<n;j++)
        {       
            int sum=0;
            for(int k=0;k<n;k++)
            {
                sum += a[i][k]*b[j][k];
            }
            c[i][j] = sum;
        }


    Barrier(2);
#if GEM5
    if(tid == 1) m5_dumpreset_stats(0,0);
#endif
    Barrier(3);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2[tid]);
}

int main()
{
#if Intel_PCM
        PCM * m = PCM::getInstance();
        PCM::ErrorCode returnResult = m->program();
        if (returnResult != PCM::Success){
                std::cerr << "Intel's PCM couldn't start" << std::endl;
                std::cerr << "Error code: " << returnResult << std::endl; exit(1);
        }

        SystemCounterState before_sstate = getSystemCounterState();
#endif

for(int trial=0;trial<NUMBER_OF_TRIALS;trial++)
{
    pthread_t*     threads;
    pthread_attr_t attr;
    int            ret, dx;
    int i;
    struct timeval begin1, end1;
    int execTime;

    Initialize();

    /* Initialize array of thread structures */
    threads = (pthread_t *) malloc(sizeof(pthread_t) * P);
    assert(threads != NULL);

    /* Initialize thread attribute */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); // sys manages contention

    for(i=0; i<NUM_BARRIERS; i++) {
        /* Initialize mutexs */
        ret = pthread_mutex_init(&SyncLock[i], NULL);
        assert(ret == 0);

        /* Init condition variable */
        ret = pthread_cond_init(&SyncCV[i], NULL);
        assert(ret == 0);
        SyncCount[i] = 0;
    }
    /*
#if GEM5
m5_reset_stats(0,0);
    //    gettimeofday (&begin1, NULL);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
#else
start = rdtsc();
    //    gettimeofday (&begin1, NULL);
#endif
*/


    for(dx=0; dx < P; dx++) {
        ret = pthread_create(&threads[dx], &attr, multiply, (void*)(uintptr_t)dx);
        assert(ret == 0);
    }

    /* Wait for each ofothe threads to terminate */
    for(dx=0; dx < P; dx++) {
        ret = pthread_join(threads[dx], NULL);
        assert(ret == 0);
    }

    for(i=0; i<P; i++) {
        diff(&d[i], t1[i], t2[i]);
        printf("thread %d) Execution Time: %ld sec, %ld nsec\n", i, d[i].tv_sec, d[i].tv_nsec);
    }
	printf("N=%d, c[0][0]=%d, c[%d][%d]=%d\n",N, c[0][0],N-1,N-1,c[N-1][N-1]);

}
#if GEM5
    m5_exit(0);
#endif
#if Intel_PCM
        SystemCounterState after_sstate = getSystemCounterState();

        /*std::cout << "Instructions per clock:" << getIPC(before_sstate,after_sstate) << std::endl;
        std::cout << "Instructions Retired:" << getInstructionsRetired(before_sstate,after_sstate) << std::endl;
        std::cout << "Cycles:" << getCycles(before_sstate,after_sstate) << std::endl;
        std::cout << "MC reads:" << getBytesReadFromMC(before_sstate,after_sstate)/64 << endl;
        std::cout << "MC writes:" << getBytesWrittenToMC(before_sstate,after_sstate)/64 << endl;
        std::cout << "Bytes read from EDC:" << getBytesReadFromEDC(before_sstate,after_sstate) << endl;
        std::cout << "Bytes written to EDC:" << getBytesWrittenToEDC(before_sstate,after_sstate) << endl;
*/	

	printf("%" PRIu64 "\tInstructions per clock\n",getIPC(before_sstate,after_sstate));
        printf("%" PRIu64 "\tInstructions Retired\n",getInstructionsRetired(before_sstate,after_sstate));
        printf("%" PRIu64 "\tCycles\n",getCycles(before_sstate,after_sstate));
        printf("%" PRIu64 "\tMC reads\n",getBytesReadFromMC(before_sstate,after_sstate)/64);
        printf("%" PRIu64 "\tMC writes\n",getBytesWrittenToMC(before_sstate,after_sstate)/64);
        printf("%" PRIu64 "\tBytes read from EDC\n",getBytesReadFromEDC(before_sstate,after_sstate));
        printf("%" PRIu64 "\tBytes written to EDC\n",getBytesWrittenToEDC(before_sstate,after_sstate));
        printf("%" PRIu64 "\tL3 misses\n",getL3CacheMisses(before_sstate,after_sstate));
        printf("%" PRIu64 "\tL3 hits\n",getL3CacheHits(before_sstate,after_sstate));
        m->cleanup();
	

#endif

    /*
#if GEM5
clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
diff(&d, t1, t2);
m5_dumpreset_stats(0,0);
printf("Execution Time: %ld sec, %ld nsec\n", d.tv_sec, d.tv_nsec);
    //    gettimeofday (&end1, NULL);
    //    execTime = 1e6 * (end1.tv_sec - begin1.tv_sec) + (end1.tv_usec - begin1.tv_usec);
    //    printf("time it took: %d\n", execTime);
    m5_exit(0);
#else
    //    gettimeofday (&end1, NULL);
    //    execTime = 1e6 * (end1.tv_sec - begin1.tv_sec) + (end1.tv_usec - begin1.tv_usec);
    //    printf("time it took: %d\n", execTime);
    end = rdtsc();
    printf("%ld ticks\n", end - start);
#endif
*/
#if DEBUG
checkC();
#endif

#if PRINT
    PrintC();
#endif

    return 0;
}

