#ifndef APP_SYSCALL_H
#define APP_SYSCALL_H

/* Backdoor syscall */
#define SYS_backdoor	1181

/*
 * Backdoor calls:
 */
#define BackDoor_SetArgs	0
#define BackDoor_SimTime     	1
#define BackDoor_CpuId       	2
#define BackDoor_ResetStats  	3
#define BackDoor_PlaceRange  	4
#define BackDoor_ActiveProcs 	5
#define BackDoor_Create		6
#define BackDoor_WaitForEnd	7
#define BackDoor_IOMap		8
#define BackDoor_IOMapGeneral	9
#define BackDoor_GetPhysAddr	10
#define BackDoor_HitWBInvalD    11
#define BackDoor_HitWBInvalS    12
#define BackDoor_IndexWBInvalD  13
#define BackDoor_IndexWBInvalS  14
#define BackDoor_FlushD         15
#define BackDoor_FlushS         16
#define BackDoor_PageFlushD     17
#define BackDoor_PageFlushS     18
#define BackDoor_MemoryMap      19
#define BackDoor_GetNodeNum	20
#define BackDoor_Delay  	21

#endif
