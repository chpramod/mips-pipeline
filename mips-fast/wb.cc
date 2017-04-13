#include "wb.h"

Writeback::Writeback (Mipc *mc)
{
   _mc = mc;
}

Writeback::~Writeback (void) {}

void
Writeback::MainLoop (void)
{
   unsigned int ins;
   MEM_WB_REG* copied_mem_wb = new MEM_WB_REG();

   while (1) {
      // PAUSE(10);
      AWAIT_P_PHI0;	// @posedge
      // PAUSE(50);
      // Sample the important signals
      // if (_mc->_memValid) {
         // writeReg = _mc->_writeREG;
         // writeFReg = _mc->_writeFREG;
         // loWPort = _mc->_loWPort;
         // hiWPort = _mc->_hiWPort;
         // decodedDST = _mc->_decodedDST;
         // opResultLo = _mc->_opResultLo;
         // opResultHi = _mc->_opResultHi;
         // isSyscall = _mc->_isSyscall;
         // isIllegalOp = _mc->_isIllegalOp;
         // ins = _mc->_ins;
         *copied_mem_wb = _mc->mem_wb;
         // printf("WB : %d %d %d %d\n",
         //    copied_mem_wb->_writeREG, 
         //    copied_mem_wb->_writeFREG, 
         //    copied_mem_wb->_loWPort, 
         //    copied_mem_wb->_hiWPort);
         if (copied_mem_wb->_isSyscall) {
            // printf("WB : isSyscall\n");
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> SYSCALL! Trapping to emulation layer at PC %#x\n", SIM_TIME, copied_mem_wb->_pc);
#endif      
            ID_EX_REG* fake_id_ex = new ID_EX_REG();
            fake_id_ex->_pc = copied_mem_wb->_pc;
            // printf("Instr %#x\n",copied_mem_wb->_ins );
            copied_mem_wb->_opControl(_mc,fake_id_ex, copied_mem_wb->_ins);
            _mc->_pc = copied_mem_wb->_pc + 4;
            _mc->got_syscall = FALSE;
         }
         else if (copied_mem_wb->_isIllegalOp) {
            // printf("Illegal ins %#x at PC %#x. Terminating simulation!\n",copied_mem_wb->_ins, copied_mem_wb->_pc);
#ifdef MIPC_DEBUG
            fclose(_mc->_debugLog);
#endif
            // printf("Register state on termination:\n\n");
            // printf("HEREEEEEEEEEEEEEEEEEE\n");
            _mc->dumpregs(copied_mem_wb);
            exit(0);
         }
         else {
            // printf("WB : inElse\n");
            // printf("WB : %d %d %d %d\n",copied_mem_wb->_writeREG, copied_mem_wb->_writeFREG, copied_mem_wb->_loWPort, copied_mem_wb->_hiWPort);
            if (copied_mem_wb->_writeREG) {
               _mc->_gpr[copied_mem_wb->_decodedDST] = copied_mem_wb->_opResultLo;
#ifdef MIPC_DEBUG
               fprintf(_mc->_debugLog, "<%llu> Writing to reg %u, value: %#x\n", SIM_TIME, copied_mem_wb->_decodedDST, copied_mem_wb->_opResultLo);
#endif
            }
            else if (copied_mem_wb->_writeFREG) {
               _mc->_fpr[(copied_mem_wb->_decodedDST)>>1].l[FP_TWIDDLE^((copied_mem_wb->_decodedDST)&1)] = copied_mem_wb->_opResultLo;
#ifdef MIPC_DEBUG
               fprintf(_mc->_debugLog, "<%llu> Writing to freg %u, value: %#x\n", SIM_TIME, copied_mem_wb->_decodedDST>>1, copied_mem_wb->_opResultLo);
#endif
            }
            else if (copied_mem_wb->_loWPort || copied_mem_wb->_hiWPort) {
               if (copied_mem_wb->_loWPort) {
                  _mc->_lo = copied_mem_wb->_opResultLo;
#ifdef MIPC_DEBUG
                  fprintf(_mc->_debugLog, "<%llu> Writing to Lo, value: %#x\n", SIM_TIME, copied_mem_wb->_opResultLo);
#endif
               }
               if (copied_mem_wb->_hiWPort) {
                  _mc->_hi = copied_mem_wb->_opResultHi;
#ifdef MIPC_DEBUG
                  fprintf(_mc->_debugLog, "<%llu> Writing to Hi, value: %#x\n", SIM_TIME, copied_mem_wb->_opResultHi);
#endif
               }
            }
         }
         _mc->_gpr[0] = 0;
         AWAIT_P_PHI1;       // @negedge
         // _mc->_memValid = FALSE;
         // _mc->_insDone = TRUE;
      // }
      // else {
      //    PAUSE(1);
      // }
   }
}
