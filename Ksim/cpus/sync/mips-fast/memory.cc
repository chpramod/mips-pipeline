#include "memory.h"

Memory::Memory (Mipc *mc)
{
   _mc = mc;
}

Memory::~Memory (void) {}

void
Memory::MainLoop (void)
{
   Bool memControl;

   while (1) {
      AWAIT_P_PHI0;	// @posedge
      if (_mc->_execValid) {
         memControl = _mc->_memControl;
         AWAIT_P_PHI1;       // @negedge
         if (memControl) {
            _mc->_memOp (_mc);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _mc->_MAR, _mc->_ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, _mc->_ins);
#endif
         }
         _mc->_execValid = FALSE;
         _mc->_memValid = TRUE;
      }
      else {
         PAUSE(1);
      }
   }
}
