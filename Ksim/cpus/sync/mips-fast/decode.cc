#include "decode.h"

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_insValid) {
         ins = _mc->_ins;
         AWAIT_P_PHI1;	// @negedge
         _mc->Dec(ins);
#ifdef MIPC_DEBUG
         fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
#endif
         _mc->_insValid = FALSE;
         _mc->_decodeValid = TRUE;
      }
      else {
         PAUSE(1);
      }
   }
}
