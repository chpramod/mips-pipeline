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
      // PAUSE(10);
      AWAIT_P_PHI0;	// @posedge
         // PAUSE(40);
         EX_MEM_REG *copied_ex_mem = new EX_MEM_REG();
         copied_ex_mem->_ins            = _mc->ex_mem._ins;
         copied_ex_mem->_pc             = _mc->ex_mem._pc;
         copied_ex_mem->_decodedSRC1    = _mc->ex_mem._decodedSRC1;       
         copied_ex_mem->_decodedSRC2    = _mc->ex_mem._decodedSRC2;     
         copied_ex_mem->_decodedDST     = _mc->ex_mem._decodedDST;       
         copied_ex_mem->_subregOperand  = _mc->ex_mem._subregOperand;      
         copied_ex_mem->_memControl     = _mc->ex_mem._memControl;      
         copied_ex_mem->_writeREG       = _mc->ex_mem._writeREG;          
         copied_ex_mem->_writeFREG      = _mc->ex_mem._writeFREG;        
         copied_ex_mem->_branchOffset   = _mc->ex_mem._branchOffset;       
         copied_ex_mem->_hiWPort        = _mc->ex_mem._hiWPort;        
         copied_ex_mem->_loWPort        = _mc->ex_mem._loWPort;         
         copied_ex_mem->_decodedShiftAmt= _mc->ex_mem._decodedShiftAmt;          
         copied_ex_mem->_bd             = _mc->ex_mem._bd;               
         copied_ex_mem->_btaken         = _mc->ex_mem._btaken;        
         copied_ex_mem->_btgt           = _mc->ex_mem._btgt;            
         copied_ex_mem->_isSyscall      = _mc->ex_mem._isSyscall;       
         copied_ex_mem->_isIllegalOp    = _mc->ex_mem._isIllegalOp; 
         copied_ex_mem->_opControl      = _mc->ex_mem._opControl; 
         copied_ex_mem->_memOp          = _mc->ex_mem._memOp;   
         copied_ex_mem->_MAR            = _mc->ex_mem._MAR;
         copied_ex_mem->_opResultHi     = _mc->ex_mem._opResultHi;
         copied_ex_mem->_opResultLo     = _mc->ex_mem._opResultLo;
         copied_ex_mem->_hi             = _mc->ex_mem._hi;
         copied_ex_mem->_lo             = _mc->ex_mem._lo;
            
      //if (_mc->_execValid) {
      //   memControl = _mc->_memControl;
         AWAIT_P_PHI1;       // @negedge
         if (copied_ex_mem->_memControl) {
            copied_ex_mem->_memOp (_mc, copied_ex_mem);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, copied_ex_mem->_MAR, copied_ex_mem->_ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, copied_ex_mem->_ins);
#endif
         }
      
         _mc->mem_wb._ins               =     copied_ex_mem->_ins              ;                                   
         _mc->mem_wb._pc                =     copied_ex_mem->_pc               ;              
         _mc->mem_wb._decodedSRC1       =     copied_ex_mem->_decodedSRC1      ;
         _mc->mem_wb._decodedSRC2       =     copied_ex_mem->_decodedSRC2      ;
         _mc->mem_wb._decodedDST        =     copied_ex_mem->_decodedDST       ;
         _mc->mem_wb._subregOperand     =     copied_ex_mem->_subregOperand    ; 
         _mc->mem_wb._memControl        =     copied_ex_mem->_memControl       ;
         _mc->mem_wb._writeREG          =     copied_ex_mem->_writeREG         ;
         _mc->mem_wb._writeFREG         =     copied_ex_mem->_writeFREG        ;
         _mc->mem_wb._branchOffset      =     copied_ex_mem->_branchOffset     ; 
         _mc->mem_wb._hiWPort           =     copied_ex_mem->_hiWPort          ; 
         _mc->mem_wb._loWPort           =     copied_ex_mem->_loWPort          ;
         _mc->mem_wb._decodedShiftAmt   =     copied_ex_mem->_decodedShiftAmt  ; 
         _mc->mem_wb._bd                =     copied_ex_mem->_bd               ;
         _mc->mem_wb._btaken            =     copied_ex_mem->_btaken           ;  
         _mc->mem_wb._btgt              =     copied_ex_mem->_btgt             ;
         _mc->mem_wb._isSyscall         =     copied_ex_mem->_isSyscall        ;
         _mc->mem_wb._isIllegalOp       =     copied_ex_mem->_isIllegalOp      ;  
         _mc->mem_wb._memOp             =     copied_ex_mem->_memOp            ;  
         _mc->mem_wb._MAR               =     copied_ex_mem->_MAR              ;
         _mc->mem_wb._opResultHi        =     copied_ex_mem->_opResultHi       ;
         _mc->mem_wb._opResultLo        =     copied_ex_mem->_opResultLo       ;  
         _mc->mem_wb._hi                =     copied_ex_mem->_hi               ; 
         _mc->mem_wb._lo                =     copied_ex_mem->_lo               ; 
         _mc->mem_wb._opControl         =     copied_ex_mem->_opControl        ; 

         // printf("MEM_WB : %d %d %d %d\n",
         //    _mc->mem_wb._writeREG, 
         //    _mc->mem_wb._writeFREG, 
         //    _mc->mem_wb._loWPort, 
         //    _mc->mem_wb._hiWPort);

//         _mc->_execValid = FALSE;
//         _mc->_memValid = TRUE;
      //}
      //else {
      //   PAUSE(1);
      //}
   }
}
