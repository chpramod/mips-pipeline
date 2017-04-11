#include "decode.h"

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins,pc;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      // if (_mc->_insValid) {
         ins = _mc->if_id._ins;
         pc = _mc->if_id._pc;
         AWAIT_P_PHI1;	// @negedge
         _mc->Dec(ins,pc);
         _mc->id_ex._pc = pc;
         _mc->id_ex._ins = ins; 
#ifdef MIPC_DEBUG
         fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
#endif

      // }
      // else {
      //    PAUSE(1);
      // }
   }
}
