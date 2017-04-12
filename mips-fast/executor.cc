#include "executor.h"

Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

void
Exe::MainLoop (void)
{
   unsigned int ins;
   Bool isSyscall, isIllegalOp;

   while (1) {
      AWAIT_P_PHI0;  // @posedge
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
         copied_id_ex->_bd             = _mc->id_ex._bd;               
         copied_id_ex->_btaken         = _mc->id_ex._btaken;        
         copied_id_ex->_btgt           = _mc->id_ex._btgt;            
         copied_id_ex->_isSyscall      = _mc->id_ex._isSyscall;       
         copied_id_ex->_isIllegalOp    = _mc->id_ex._isIllegalOp; 
         copied_id_ex->_opControl      = _mc->id_ex._opControl; 
         copied_id_ex->_memOp          = _mc->id_ex._memOp;   
         AWAIT_P_PHI1;  // @negedge

         if (!copied_id_ex->_isSyscall && !copied_id_ex->_isIllegalOp) {
            copied_id_ex->_opControl(_mc,copied_id_ex,copied_id_ex->_ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x\n", SIM_TIME, copied_id_ex->_ins);
#endif
         }
         else if (copied_id_ex->_isSyscall) {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Deferring execution of syscall ins %#x\n", SIM_TIME, copied_id_ex->_ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Illegal ins %#x in execution stage at PC %#x\n", SIM_TIME, copied_id_ex->_ins, copied_id_ex->_pc);
#endif
         }
         // _mc->_decodeValid = FALSE;
         // _mc->_execValid = TRUE;

         // if (!copied_id_ex->_isIllegalOp && !copied_id_ex->_isSyscall) {
         //    /* Handle branch delay here, send NOPS for instr i+2 i.e. all 0s in IF/ID in decoder*/
         //    if (copied_id_ex->_lastbd && copied_id_ex->_btaken)
         //    {

         //       _mc->_pc = copied_id_ex->_btgt;
         //    }
         //    else
         //    {
         //       _mc->_pc = copied_id_ex->_pc + 4;
         //    }
         //    /* I think lastbd of the next instr i+1 */
         //    copied_id_ex->_lastbd = copied_id_ex->_bd;
         // }
         
         _mc->ex_mem._ins               =     copied_id_ex->_ins              ;                                   
         _mc->ex_mem._pc                =     copied_id_ex->_pc               ;              
         _mc->ex_mem._decodedSRC1       =     copied_id_ex->_decodedSRC1      ;
         _mc->ex_mem._decodedSRC2       =     copied_id_ex->_decodedSRC2      ;
         _mc->ex_mem._decodedDST        =     copied_id_ex->_decodedDST       ;
         _mc->ex_mem._subregOperand     =     copied_id_ex->_subregOperand    ; 
         _mc->ex_mem._memControl        =     copied_id_ex->_memControl       ;
         _mc->ex_mem._writeREG          =     copied_id_ex->_writeREG         ;
         _mc->ex_mem._writeFREG         =     copied_id_ex->_writeFREG        ;
         _mc->ex_mem._branchOffset      =     copied_id_ex->_branchOffset     ; 
         _mc->ex_mem._hiWPort           =     copied_id_ex->_hiWPort          ; 
         _mc->ex_mem._loWPort           =     copied_id_ex->_loWPort          ;
         _mc->ex_mem._decodedShiftAmt   =     copied_id_ex->_decodedShiftAmt  ; 
         _mc->ex_mem._bd                =     copied_id_ex->_bd               ;
         _mc->ex_mem._btaken            =     copied_id_ex->_btaken           ;  
         _mc->ex_mem._btgt              =     copied_id_ex->_btgt             ;
         _mc->ex_mem._isSyscall         =     copied_id_ex->_isSyscall        ;
         _mc->ex_mem._isIllegalOp       =     copied_id_ex->_isIllegalOp      ;  
         _mc->ex_mem._memOp             =     copied_id_ex->_memOp            ; 
         _mc->ex_mem._opControl         =     copied_id_ex->_opControl        ; 


      // }
      // else {
      //    PAUSE(1);
      // }
   }
}
