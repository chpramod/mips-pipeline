#ifndef __WB_H__
#define __WB_H__

#include "mips.h"

class Mipc;

class Writeback : public SimObject {
public:
   Writeback (Mipc*);
   ~Writeback ();
  
   FAKE_SIM_TEMPLATE;

   Mipc *_mc;
};
#endif
