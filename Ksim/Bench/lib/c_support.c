#include "mipc.h"
#include <stdio.h>

void
_Page_Flush_D(unsigned vaddr)
{
   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_PAGE_FLUSH_D));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_Page_Flush_S(unsigned vaddr)
{
   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_PAGE_FLUSH_S));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_Hit_WB_Inval_D(unsigned vaddr)
{
   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_HIT_WB_INVAL_D));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}
 
void
_Hit_WB_Inval_S(unsigned vaddr)
{
   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));   
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_HIT_WB_INVAL_S));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}
 
void
_Index_WB_Inval_D(unsigned index, unsigned way)
{
   asm volatile("add\t$6,$0,%0" :: "r" (way));
   asm volatile("add\t$5,$0,%0" :: "r" (index));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_INDEX_WB_INVAL_D));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_Index_WB_Inval_S(unsigned index, unsigned way)
{
   asm volatile("add\t$6,$0,%0" :: "r" (way));
   asm volatile("add\t$5,$0,%0" :: "r" (index));   
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_INDEX_WB_INVAL_S));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_PlaceRange(unsigned start, unsigned stop, unsigned node)
{
   asm volatile ("add\t$7,$0,%0" :: "r" (node));
   asm volatile ("add\t$6,$0,%0" :: "r" (stop));
   asm volatile ("add\t$5,$0,%0" :: "r" (start));
   asm volatile ("li\t$4,%0" :: "n" (MIPC_ANL_PLACE_RANGE));
   asm volatile ("li\t$2,%0" :: "n" (SYS_backdoor)); 
   asm volatile ("syscall" ::);
}

void
_Create(unsigned function, unsigned ex)
{
   asm volatile("add\t$6,$0,%0" :: "r" (ex));
   asm volatile("add\t$5,$0,%0" :: "r" (function));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_CREATE));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_Wait_for_end(unsigned how_many)
{
   asm volatile("add\t$5,$0,%0" :: "r" (how_many));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_WAIT_FOR_END));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

unsigned
_Syscall(unsigned number)
{
   unsigned retVal;

   asm volatile ("li\t$2,%0" :: "n" (SYS_backdoor)); 
   asm volatile ("syscall" ::);
   asm volatile ("add\t%0,$4,$0" : "=r" (retVal) :);

   return retVal;
}

void
_IOMap(unsigned vaddr, unsigned long long paddr)
{
/* Long longs must be in even/odd register pairs in MIPS, so vaddr should already be in r4 and paddr should already be in r6 and r7.  But when I assembled it it did not appear that way, so I have tried to make sure below */
/* I checked the assembled code and found that long longs are passed correctly. - paddr are passed in r6(a2) and r7(a3). so I deleted the lines which expained above. this makes life easier. */ 

   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_IO_MAP));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_IOMapGeneral(unsigned vaddr, unsigned details, unsigned long long paddr)
{
/* Long longs must be in even/odd register pairs in MIPS, so vaddr should already be in r4 and paddr should already be in r6 and r7.  I'm moving details (starts in r5 to r8 */
/* With the same reason as _IOMap, I deleted some lines. */

   asm volatile("add\t$8,$0,%0" :: "r" (details));
   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_IO_MAP_GENERAL));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

void
_MemoryMap(unsigned vaddress, unsigned paddress, unsigned node, unsigned space, unsigned flavor)
{
   asm volatile("add\t$9,$0,%0" :: "r" (flavor));
   asm volatile("add\t$8,$0,%0" :: "r" (space));
   asm volatile("add\t$7,$0,%0" :: "r" (node));
   asm volatile("add\t$6,$0,%0" :: "r" (paddress));
   asm volatile("add\t$5,$0,%0" :: "r" (vaddress));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_MEMORY_MAP));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
}

unsigned long long
_GetPhysAddr(unsigned vaddr)
{
   unsigned retValHi, retValLo;

   asm volatile("add\t$5,$0,%0" :: "r" (vaddr));
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_GET_PHYSADDR));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
   asm volatile ("add\t$2,$4,$0" ::);   
   asm volatile ("add\t$3,$5,$0" ::);   
   return ;
}

unsigned
_GetNodeNum(void)
{
   asm volatile("li\t$4,%0" :: "n" (MIPC_ANL_GET_NODENUM));
   asm volatile("li\t$2,%0" :: "n" (SYS_backdoor));
   asm volatile("syscall" ::);
   asm volatile ("add\t$2,$4,$0" ::);   
   return ;
}
