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
   unsigned int last_dst1, last_dst2, fpt_last_dst1, fpt_last_dst2, default_reg = 2000;
   last_dst1 = default_reg; //last_dst1 is destination of (i-1)th instr
   last_dst2 = default_reg; //(i-2)th instr
   fpt_last_dst1 = default_reg; //fpt_last_dst1 is destination of (i-1)th instr if it is floating pt instr
   fpt_last_dst2 = default_reg; //(i-2)th instr
   unsigned int check_src1, check_src2,check_dst1,check_dst2;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      // if (_mc->_insValid) {
         ins = _mc->if_id._ins;
         pc = _mc->if_id._pc;
         AWAIT_P_PHI1;	// @negedge
         if (_mc->got_syscall==TRUE){
            _mc->zeroOutID_EX();
         }
         else{
            _mc->Dec(ins,pc);
            _mc->id_ex._pc = pc;
            _mc->id_ex._ins = ins; 
            if (_mc->id_ex._isSyscall==TRUE){ //Syscall --> stall from next cycle
               _mc->got_syscall = TRUE;
            }
            else{
               if (_mc->id_ex.is_fpt == TRUE){       //Floating pt instr --> requires comparison of fpt registers
                  if (_mc->id_ex.fpt_src_reg == fpt_last_dst1){
                     //stall 2 cycles
                  }
                  else if (_mc->id_ex.fpt_src_reg == fpt_last_dst2){
                     //stall 1 cycle
                  }
                  fpt_last_dst2 = fpt_last_dst1;
                  fpt_last_dst1 = _mc->id_ex._decodedDST;
                  last_dst2 = last_dst1;
                  last_dst1 = default_reg;
               }
               else{                //Integer instructions
                  if (_mc->id_ex.src_reg1 == last_dst1 || _mc->id_ex.src_reg2 == last_dst1){
                     //Stall 2 cycles
                  }  
                  else if (_mc->id_ex.src_reg1 == last_dst2 || _mc->id_ex.src_reg2 == last_dst2){
                     //Stall 1 cycle
                  }
                  last_dst2 = last_dst1;
                  last_dst1 = _mc->id_ex._decodedDST;
                  fpt_last_dst2 = fpt_last_dst1;
                  fpt_last_dst1 = default_reg;
               }
            }
         }
#ifdef MIPC_DEBUG
         fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
#endif

      // }
      // else {
      //    PAUSE(1);
      // }
   }
}
