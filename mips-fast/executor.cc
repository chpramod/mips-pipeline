#include "executor.h"

Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

struct ID_EX_REG{
   unsigned int _ins;
   unsigned int _pc;
   signed int  _decodedSRC1, _decodedSRC2;   // Reg fetch output (source values)
   unsigned _decodedDST;         // Decoder output (dest reg no)
   unsigned    _subregOperand;         // Needed for lwl and lwr
   Bool  _memControl;         // Memory instruction?
   Bool     _writeREG, _writeFREG;     // WB control
   signed int  _branchOffset;
   Bool  _hiWPort, _loWPort;     // WB control
   unsigned _decodedShiftAmt;    // Shift amount
   unsigned int _lastbd;         // branch delay state
   int      _bd;           // 1 if the next ins is delay slot
   int      _btaken;          // taken branch (1 if taken, 0 if fall-through)
   unsigned int   _btgt;            // branch target
   Bool     _isSyscall;       // 1 if system call
   Bool     _isIllegalOp;        // 1 if illegal opcode
   void (*_opControl)(Mipc*, ID_EX_REG*,unsigned);
};
void
Exe::MainLoop (void)
{
   unsigned int ins;
   Bool isSyscall, isIllegalOp;

   while (1) {
      AWAIT_P_PHI0;	// @posedge
         ID_EX_REG *copied_id_ex = new ID_EX_REG();
         copied_id_ex->_ins    = _mc->id_ex._ins;
         copied_id_ex->_pc    = _mc->id_ex._pc;
         copied_id_ex->_decodedSRC1    = _mc->id_ex._decodedSRC1;       
         copied_id_ex->_decodedSRC2    = _mc->id_ex._decodedSRC2;     
         copied_id_ex->_decodedDST     = _mc->id_ex._decodedDST;       
         copied_id_ex->_subregOperand  = _mc->id_ex._subregOperand;      
         copied_id_ex->_memControl     = _mc->id_ex._memControl;      
         copied_id_ex->_writeREG       = _mc->id_ex._writeREG;          
         copied_id_ex->_writeFREG      = _mc->id_ex._writeFREG;        
         copied_id_ex->_branchOffset   = _mc->id_ex._branchOffset;       
         copied_id_ex->_hiWPort        = _mc->id_ex._hiWPort;        
         copied_id_ex->_loWPort        = _mc->id_ex._loWPort;         
         copied_id_ex->_decodedShiftAmt = _mc->id_ex._decodedShiftAmt;    
         copied_id_ex->_lastbd         = _mc->id_ex._lastbd;              
         copied_id_ex->_bd             = _mc->id_ex._bd;               
         copied_id_ex->_btaken         = _mc->id_ex._btaken;        
         copied_id_ex->_btgt           = _mc->id_ex._btgt;            
         copied_id_ex->_isSyscall      = _mc->id_ex._isSyscall;       
         copied_id_ex->_isIllegalOp    = _mc->id_ex._isIllegalOp; 
         copied_id_ex->_opControl      = _mc->id_ex._opControl;   
         AWAIT_P_PHI1;	// @negedge

         if (!isSyscall && !isIllegalOp) {
            copied_id_ex->_opControl(_mc,copied_id_ex->_ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x\n", SIM_TIME, copied_id_ex->_ins);
#endif
         }
         else if (isSyscall) {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Deferring execution of syscall ins %#x\n", SIM_TIME, copied_id_ex->_ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Illegal ins %#x in execution stage at PC %#x\n", SIM_TIME, copied_id_ex->_ins, copied_id_ex->_pc);
#endif
         }
         _mc->_decodeValid = FALSE;
         _mc->_execValid = TRUE;

         if (!isIllegalOp && !isSyscall) {
            if (_mc->_lastbd && _mc->_btaken)
            {
               _mc->_pc = _mc->_btgt;
            }
            else
            {
               _mc->_pc = _mc->_pc + 4;
            }
            _mc->_lastbd = _mc->_bd;
         }
      // }
      // else {
      //    PAUSE(1);
      // }
   }
}
