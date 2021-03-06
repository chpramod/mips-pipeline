#include "decode.h"

#define REG_HI 100
#define REG_LO 101
#define REG_BOTH 102

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins,pc;
   unsigned int last_dst1, last_dst2, fpt_last_dst1, fpt_last_dst2, default_reg = 2000, mdst1,mdst2;
   last_dst1 = default_reg; //last_dst1 is destination of (i-1)th instr
   last_dst2 = default_reg; //(i-2)th instr
   fpt_last_dst1 = default_reg; //fpt_last_dst1 is destination of (i-1)th instr if it is floating pt instr
   fpt_last_dst2 = default_reg; //(i-2)th instr
   bool repeat_input = FALSE;
   bool last_inst_branch = FALSE;
   int stall = 0;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      last_inst_branch = _mc->id_ex._bd ;
      // PAUSE(20);
      // if (_mc->_insValid) {
      // printf("Stall value %d\n",stall);
      if(stall == 0){
         // printf("DECODE : Stall 0 case\n");
         ins = _mc->if_id._ins;
         pc = _mc->if_id._pc;
         AWAIT_P_PHI1;  // @negedge
      }else if(stall == 1){
         stall --;
         AWAIT_P_PHI1;  // @negedge
         _mc->refetch = FALSE;
      }else{
         stall --;
         AWAIT_P_PHI1;  // @negedge
         _mc->zeroOutID_EX();
         continue;
      }

      if (_mc->got_syscall==true){
         // printf("DECODE : syscall\n");
         _mc->zeroOutID_EX();
      }
      else{
         // printf("DECODE : No syscall\n");
         _mc->Dec(ins,pc);

#ifdef MIPC_DEBUG
         fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
#endif

         if (_mc->id_ex._isSyscall==TRUE){ //Syscall --> stall from next cycle
            _mc->got_syscall = TRUE;
            // printf("DECODE : Received System call PC: %#x\n",pc);
         }
         else{
            if (_mc->id_ex.is_fpt == TRUE){       //Floating pt instr --> requires comparison of fpt registers
               if (_mc->id_ex.fpt_src_reg == fpt_last_dst1 && fpt_last_dst1!=0){
                  //stall 2 cycles
                  _mc->zeroOutID_EX();
                  stall = 2;
                  _mc->refetch = TRUE;
                  fpt_last_dst2 = default_reg;
                  fpt_last_dst1 = default_reg;
                  last_dst2 = default_reg;
                  last_dst1 = default_reg;
               }
               else if (_mc->id_ex.fpt_src_reg == fpt_last_dst2 && fpt_last_dst2!=0){
                  //stall 1 cycle
                  _mc->zeroOutID_EX();
                  stall = 1;
                  _mc->refetch = TRUE;
                  fpt_last_dst2 = fpt_last_dst1;
                  fpt_last_dst1 = default_reg;
                  last_dst2 = last_dst1;
                  last_dst1 = default_reg;
                  // printf("DECODE : Stall 1 case, fpt PC : %#x, \n",pc);
               }else{
                  fpt_last_dst2 = fpt_last_dst1;
                  fpt_last_dst1 = _mc->id_ex._decodedDST  == 10000 ? default_reg : _mc->id_ex._decodedDST;
                  last_dst2 = last_dst1;
                  last_dst1 = default_reg;
               }
            }
            else{                //Integer instructions
               // printf("stored regs %u %u\n",last_dst1, last_dst2);
               // printf("check regs %u %u\n",_mc->id_ex.src_reg1, _mc->id_ex.src_reg2);
               if ( (_mc->id_ex.src_reg1 == last_dst1 && last_dst1!=0 ) || 
                   (_mc->id_ex.src_reg2 == last_dst1 && last_dst1 != 0) || 
                   (last_dst1 == REG_BOTH && (_mc->id_ex.src_reg1==REG_HI || _mc->id_ex.src_reg1==REG_LO)) ) {
                  //Stall 2 cycles
                  _mc->zeroOutID_EX();
                  stall = 2;
                  _mc->refetch = TRUE;
                  last_dst2 = default_reg;
                  last_dst1 = default_reg; 
                  fpt_last_dst2 = default_reg;
                  fpt_last_dst1 = default_reg;
                  // printf("DECODE : Stall 2 case, integer PC : %#x, \n",pc);
               }  
               else if ( (_mc->id_ex.src_reg1 == last_dst2 && last_dst2!=0) || 
                  (_mc->id_ex.src_reg2 == last_dst2 && last_dst2!=0) || 
                  (last_dst2 == REG_BOTH && (_mc->id_ex.src_reg1==REG_HI || _mc->id_ex.src_reg1==REG_LO))) {
                  //Stall 1 cycle
                  _mc->zeroOutID_EX();
                  stall = 1;
                  _mc->refetch = TRUE;
                  last_dst2 = last_dst1;
                  last_dst1 = default_reg; 
                  fpt_last_dst2 = fpt_last_dst1;
                  fpt_last_dst1 = default_reg;
                  // printf("DECODE : Stall 1 case, integer PC : %#x, \n",pc);
               }else{
                  last_dst2 = last_dst1;
                  fpt_last_dst2 = fpt_last_dst1;
                  fpt_last_dst1 = default_reg;
                  if(_mc->id_ex.is_hi_lo){
                     if(_mc->id_ex.dst_hi == REG_HI && _mc->id_ex.dst_lo != REG_LO){
                        last_dst1 = REG_HI;
                     }else if(_mc->id_ex.dst_hi != REG_HI && _mc->id_ex.dst_lo == REG_LO){
                        last_dst1 = REG_LO;
                     }else{
                        last_dst1 = REG_BOTH;
                     }
                  }else{
                     last_dst1 = _mc->id_ex._decodedDST  == 10000 ? default_reg : _mc->id_ex._decodedDST; 
                  }
               }
            }
         }
         // printf("Decode : %d %d %d %d\n",
         //    _mc->mem_wb._writeREG, 
         //    _mc->mem_wb._writeFREG, 
         //    _mc->mem_wb._loWPort, 
         //    _mc->mem_wb._hiWPort);
      }
      // }
      // else {
      //    PAUSE(1);
      // }
   }
}
