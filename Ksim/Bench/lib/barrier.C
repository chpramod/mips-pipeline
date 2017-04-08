EXTERN_ENV;

#define LINE_SIZE	128

typedef unsigned int uint;

typedef volatile uint semaphore;

/* 
 * MAX_SEMAPHORES * MAX_BARRIERS * sizeof(semaphore) should always be a
 * multiple of a page in size. Which means that MAX_SEMAPHORES must be a
 * multiple of 2. 
 */
#define MAX_SEMAPHORES	(LINE_SIZE / sizeof(semaphore))
#define BARRIER_PAD	((PAGE_SIZE - LINE_SIZE) / sizeof(uint))
#define MAX_BAR_PROCS	256

/* These should be uncached loads & stores, if possible */
#define CLEAR(n)	((n) = 0)
#define SET(n)		((n) = 1)
#define WAIT(n)		while ((n) == 0)

uint pad1[PAGE_SIZE / sizeof(uint)];
struct {
   semaphore flag[MAX_SEMAPHORES];
   uint	     pad[BARRIER_PAD];
} Barrier[MAX_BAR_PROCS];
uint pad2[PAGE_SIZE / sizeof(uint)];

void
TreeBarrierInit (uint numProcs)
{
   uint i, j;

   for (i = 0; i < numProcs; ++i) {
#ifdef PLACE
      SYS_PLACE_RANGE(&(Barrier[i].flag[0]),
                      &(Barrier[i].flag[MAX_SEMAPHORES]), i);
#endif
      for (j = 0; j < MAX_SEMAPHORES; ++j) {
	 CLEAR(Barrier[i].flag[j]);
      }
   }
}

void
TreeBarrier (uint procId, uint numProcs)
{
   uint i, mask;
   
   SYNC_OP;
   for (i = 0, mask = 1; (mask & procId) != 0; ++i, mask <<= 1) {
      WAIT(Barrier[procId].flag[i]);
      CLEAR(Barrier[procId].flag[i]);
   }
   if (procId < (numProcs - 1)) {
      SET(Barrier[procId + mask].flag[i]);
      WAIT(Barrier[procId].flag[MAX_SEMAPHORES - 1]);
      CLEAR(Barrier[procId].flag[MAX_SEMAPHORES - 1]);
   }
   for (mask >>= 1; mask > 0; mask >>= 1) {
      SET(Barrier[procId - mask].flag[MAX_SEMAPHORES - 1]);
   }
}

