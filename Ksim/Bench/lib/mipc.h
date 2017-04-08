#ifndef __MIPC_H__
#define __MIPC_H__

#include "app_syscall.h"

typedef unsigned long long LL;

#define DOUBLE_WORD_SIZE         8ULL
#define CACHE_LINE_SIZE          128ULL
#define MIPC_ANL_CACHE_LINE_SIZE 128

/* ANL related backdoor syscalls */
#define MIPC_ANL_CLOCK		BackDoor_SimTime
#define MIPC_ANL_GETCPU		BackDoor_CpuId
#define MIPC_ANL_RESET_STATS	BackDoor_ResetStats
#define MIPC_ANL_PLACE_RANGE	BackDoor_PlaceRange
#define MIPC_ANL_ACTIVE_PROCS	BackDoor_ActiveProcs
#define MIPC_ANL_CREATE		BackDoor_Create
#define MIPC_ANL_WAIT_FOR_END	BackDoor_WaitForEnd
#define MIPC_ANL_IO_MAP		BackDoor_IOMap
#define MIPC_ANL_IO_MAP_GENERAL	BackDoor_IOMapGeneral
#define MIPC_ANL_MEMORY_MAP	BackDoor_MemoryMap
#define MIPC_ANL_GET_PHYSADDR	BackDoor_GetPhysAddr
#define MIPC_ANL_GET_NODENUM	BackDoor_GetNodeNum
#define MIPC_ANL_HIT_WB_INVAL_D	BackDoor_HitWBInvalD
#define MIPC_ANL_HIT_WB_INVAL_S BackDoor_HitWBInvalS
#define MIPC_ANL_INDEX_WB_INVAL_D	BackDoor_IndexWBInvalD
#define MIPC_ANL_INDEX_WB_INVAL_S       BackDoor_IndexWBInvalS
#define MIPC_ANL_FLUSH_D	BackDoor_FlushD
#define MIPC_ANL_FLUSH_S        BackDoor_FlushS
#define MIPC_ANL_PAGE_FLUSH_D        	BackDoor_PageFlushD
#define MIPC_ANL_PAGE_FLUSH_S           BackDoor_PageFlushS

extern unsigned _Syscall(unsigned number);
extern void _PlaceRange(unsigned start, unsigned stop, unsigned node);
extern void _Create(unsigned func,unsigned ex);
extern void _Wait_for_end(unsigned how_many);
extern void _IOMap(unsigned vaddr, unsigned long long paddr);
extern void _IOMapGeneral(unsigned vaddr, unsigned details, unsigned long long paddr); 
extern void _MemoryMap(unsigned vaddress, unsigned paddress, unsigned node, unsigned space, unsigned flavor);
extern unsigned long long _GetPhysAddr(unsigned vaddr);
extern unsigned _GetNodeNum(void);
extern void _Hit_WB_Inval_D(unsigned vaddr);
extern void _Hit_WB_Inval_S(unsigned vaddr);
extern void _Index_WB_Inval_D(unsigned index, unsigned way);
extern void _Index_WB_Inval_S(unsigned index, unsigned way);
extern void _Page_Flush_D(unsigned vaddr);
extern void _Page_Flush_S(unsigned vaddr);
extern LL _LoadDouble(LL *Address);
extern void _StoreDouble(LL *ADdress, LL Value);

#endif
