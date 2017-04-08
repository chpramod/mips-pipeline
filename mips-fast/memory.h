#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "mips.h"

class Mipc;

class Memory : public SimObject {
public:
   Memory (Mipc*);
   ~Memory ();
  
   FAKE_SIM_TEMPLATE;

   Mipc *_mc;
};
#endif
