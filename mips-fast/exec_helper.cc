#include <math.h>
#include "mips.h"
#include "opcodes.h"
#include <assert.h>
#include "app_syscall.h"

#define REG_HI 100
#define REG_LO 101

/*------------------------------------------------------------------------
 *
 *  Instruction exec 
 *
 *------------------------------------------------------------------------
 */

void Mipc::zeroOutIF_ID(){
   if_id._ins = 0;
   if_id._pc = 0;
}

void Mipc::zeroOutID_EX(){
   id_ex._ins            = 0;
   id_ex._pc             = 0;
   id_ex._decodedSRC1    = 0;
   id_ex._decodedSRC2    = 0;
   id_ex._decodedDST     = 0;
   id_ex._subregOperand  = 0;
   id_ex._memControl     = 0;
   id_ex._writeREG       = 0;
   id_ex._writeFREG      = 0;
   id_ex._branchOffset   = 0;
   id_ex._hiWPort        = false;
   id_ex._loWPort        = false;
   id_ex._decodedShiftAmt= 0;
   id_ex._bd             = 0;
   id_ex._btaken         = 0;
   id_ex._btgt           = 0;
   id_ex._isSyscall      = false;
   id_ex._isIllegalOp    = false;
   id_ex._opControl      = NULL;
   id_ex._memOp          = NULL;
}

void Mipc::zeroOutEX_MEM(){
   ex_mem._ins               = 0; 
   ex_mem._pc                = 0;
   ex_mem._decodedSRC1       = 0;
   ex_mem._decodedSRC2       = 0; 
   ex_mem._decodedDST        = 0; 
   ex_mem._subregOperand     = 0; 
   ex_mem._MAR               = 0; 
   ex_mem._opResultHi        = 0;
   ex_mem._opResultLo        = 0;
   ex_mem._memControl        = false;
   ex_mem._writeREG          = false;
   ex_mem._writeFREG         = false;
   ex_mem._branchOffset      = 0;
   ex_mem._hiWPort           = false;
   ex_mem._loWPort           = false;
   ex_mem._decodedShiftAmt   = 0;
   ex_mem._hi                = 0;
   ex_mem._lo                = 0;
   ex_mem._bd                = 0;
   ex_mem._btaken            = 0;
   ex_mem._btgt              = 0;
   ex_mem._isSyscall         = false;
   ex_mem._isIllegalOp       = false;
   ex_mem._memOp             = NULL;
   ex_mem._opControl         = NULL;
}

void Mipc::zeroOutMEM_WB(){
   mem_wb._ins               = 0; 
   mem_wb._pc                = 0;
   mem_wb._decodedSRC1       = 0;
   mem_wb._decodedSRC2       = 0; 
   mem_wb._decodedDST        = 0; 
   mem_wb._subregOperand     = 0; 
   mem_wb._MAR               = 0; 
   mem_wb._opResultHi        = 0;
   mem_wb._opResultLo        = 0;
   mem_wb._memControl        = false;
   mem_wb._writeREG          = false;
   mem_wb._writeFREG         = false;
   mem_wb._branchOffset      = 0;
   mem_wb._hiWPort           = false;
   mem_wb._loWPort           = false;
   mem_wb._decodedShiftAmt   = 0;
   mem_wb._hi                = 0;
   mem_wb._lo                = 0;
   mem_wb._bd                = 0;
   mem_wb._btaken            = 0;
   mem_wb._btgt              = 0;
   mem_wb._isSyscall         = false;
   mem_wb._isIllegalOp       = false;
   mem_wb._memOp             = NULL;
   mem_wb._opControl         = NULL;
}

void
Mipc::Dec (unsigned int ins,unsigned int pc)
{
   //Our variables
   signed int  _decodedSRC1, _decodedSRC2;   // Reg fetch output (source values)
   unsigned _decodedDST;         // Decoder output (dest reg no)
   unsigned    _subregOperand;         // Needed for lwl and lwr
   Bool  _memControl;         // Memory instruction?
   Bool     _writeREG, _writeFREG;     // WB control
   signed int  _branchOffset;
   Bool  _hiWPort, _loWPort;     // WB control
   unsigned _decodedShiftAmt;    // Shift amount
   int      _bd;           // 1 if the next ins is delay slot
   int      _btaken;          // taken branch (1 if taken, 0 if fall-through)
   unsigned int   _btgt;            // branch target
   Bool     _isSyscall;       // 1 if system call
   Bool     _isIllegalOp;        // 1 if illegal opcode
   void (*_opControl)(Mipc*, ID_EX_REG*,unsigned);
   void (*_memOp)(Mipc*,EX_MEM_REG*);

   MipsInsn i;
   signed int a1, a2;
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
   LL addr;
   unsigned int val;
   unsigned int src_reg1, src_reg2,fpt_src_reg,dst_hi,dst_lo;
   Bool is_fpt, is_hi_lo; 

   LL value, mask;
   int sa,j;
   Word dummy;
   _bd = 0;
   _btaken = 0;
   _btgt = 0xdeadbeef;
   src_reg1 = 10000;
   src_reg2 = 10000;
   dst_hi = 10000;
   dst_lo = 10000;
   fpt_src_reg = 10000;
   _isIllegalOp = FALSE;
   _isSyscall = FALSE;
   is_fpt = FALSE;
   is_hi_lo = FALSE;
   i.data = ins;
   // printf("EXEC_HELPER : reg.op:%d reg.func:%d\n",i.reg.op,i.reg.func);
  
#define SIGN_EXTEND_BYTE(x)  do { x <<= 24; x >>= 24; } while (0)
#define SIGN_EXTEND_IMM(x)   do { x <<= 16; x >>= 16; } while (0)
   // printf("PC : %#x i.reg.op %#x i.reg.func %#x i.reg.rt %#x \n",pc,i.reg.op, i.reg.func, i.reg.rt );
   switch (i.reg.op) {
   case 0:
      // SPECIAL (ALU format)
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = _gpr[i.reg.rt];
      _decodedDST = i.reg.rd;
      src_reg1 = i.reg.rs;
      src_reg2 = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;

      switch (i.reg.func) {
      case 0x20:			// add
      case 0x21:			// addu
         _opControl = func_add_addu;
	 break;

      case 0x24:			// and
         _opControl = func_and;
	 break;

      case 0x27:			// nor
         _opControl = func_nor;
	 break;

      case 0x25:			// or
         _opControl = func_or;
	 break;

      case 0:			// sll
         _opControl = func_sll;
         _decodedShiftAmt = i.reg.sa;
	 break;

      case 4:			// sllv
         _opControl = func_sllv;
	 break;

      case 0x2a:			// slt
         _opControl = func_slt;
	 break;

      case 0x2b:			// sltu
         _opControl = func_sltu;
	 break;

      case 0x3:			// sra
         _opControl = func_sra;
         _decodedShiftAmt = i.reg.sa;
	 break;

      case 0x7:			// srav
         _opControl = func_srav;
	 break;

      case 0x2:			// srl
         _opControl = func_srl;
         _decodedShiftAmt = i.reg.sa;
	 break;

      case 0x6:			// srlv
         _opControl = func_srlv;
	 break;

      case 0x22:			// sub
      case 0x23:			// subu
	 // no overflow check
         _opControl = func_sub_subu;
	 break;

      case 0x26:			// xor
         _opControl = func_xor;
	 break;

      case 0x1a:			// div
         _opControl = func_div;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         dst_hi = REG_HI;
         dst_lo = REG_LO;
         is_hi_lo = TRUE;
	 break;

      case 0x1b:			// divu
         _opControl = func_divu;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         dst_hi = REG_HI;
         dst_lo = REG_LO;
         is_hi_lo = TRUE;
	 break;

      case 0x10:			// mfhi
         _opControl = func_mfhi;
         src_reg1 = REG_HI;
	 break;

      case 0x12:			// mflo
         _opControl = func_mflo;
         src_reg1 = REG_LO;
	 break;

      case 0x11:			// mthi
         _opControl = func_mthi;
         _hiWPort = TRUE;
         dst_hi = REG_HI;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         is_hi_lo = TRUE;
	 break;

      case 0x13:			// mtlo
         _opControl = func_mtlo;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         dst_lo = REG_LO;
         is_hi_lo = TRUE;
	 break;

      case 0x18:			// mult
         _opControl = func_mult;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         dst_hi = REG_HI;
         dst_lo = REG_LO;
         is_hi_lo = TRUE;
	 break;

      case 0x19:			// multu
         _opControl = func_multu;
         _hiWPort = TRUE;
         _loWPort = TRUE;
         _writeREG = FALSE;
          _writeFREG = FALSE;
         dst_hi = REG_HI;
         dst_lo = REG_LO;
         is_hi_lo = TRUE;
	 break;

      case 9:			// jalr
         _opControl = func_jalr;
         _btgt = _decodedSRC1;
         _bd = 1;
         break;

      case 8:			// jr
         _opControl = func_jr;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         _btgt = _decodedSRC1;
         _bd = 1;
	 break;

      case 0xd:			// await/break
         _opControl = func_await_break;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;

      case 0xc:			// syscall
         _opControl = func_syscall;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         _isSyscall = TRUE;

	 break;

      default:
	 _isIllegalOp = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
	 break;
      }
      break;	// ALU format

   case 8:			// addi
   case 9:			// addiu
      // ignore overflow: no exceptions
      _opControl = func_addi_addiu;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      src_reg1 = i.imm.rs;
      _writeREG = TRUE;
       _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xc:			// andi
      _opControl = func_andi;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      src_reg1 = i.imm.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xf:			// lui
      _opControl = func_lui;
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xd:			// ori
      _opControl = func_ori;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      src_reg1 = i.imm.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xa:			// slti
      _opControl = func_slti;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      src_reg1 = i.imm.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xb:			// sltiu
      _opControl = func_sltiu;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      src_reg1 = i.imm.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 0xe:			// xori
      _opControl = func_xori;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.imm.rt;
      src_reg1 = i.imm.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;

   case 4:			// beq
      _opControl = func_beq;
      _decodedSRC1 = _gpr[i.imm.rs];
      _decodedSRC2 = _gpr[i.imm.rt];
      _branchOffset = i.imm.imm;
      src_reg1 = i.imm.rs;
      src_reg2 = i.imm.rt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
      break;

   case 1:
      // REGIMM
      _decodedSRC1 = _gpr[i.reg.rs];
      _branchOffset = i.imm.imm;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;

      switch (i.reg.rt) {
      case 1:			// bgez
         _opControl = func_bgez;
         _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
	 break;

      case 0x11:			// bgezal
         _opControl = func_bgezal;
         _decodedDST = 31;
         _writeREG = TRUE;
         _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
	 break;

      case 0x10:			// bltzal
         _opControl = func_bltzal;
         _decodedDST = 31;
         _writeREG = TRUE;
         _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
	 break;

      case 0x0:			// bltz
         _opControl = func_bltz;
         _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
	 break;

      default:
	 _isIllegalOp = TRUE;
	 break;
      }
      break;

   case 7:			// bgtz
      _opControl = func_bgtz;
      _decodedSRC1 = _gpr[i.reg.rs];
      src_reg1 = i.reg.rs;
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
      break;

   case 6:			// blez
      _opControl = func_blez;
      _decodedSRC1 = _gpr[i.reg.rs];
      src_reg1 = i.reg.rs;
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
      break;

   case 5:			// bne
      _opControl = func_bne;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = _gpr[i.reg.rt];
      src_reg1 = i.reg.rs;
      src_reg2 = i.reg.rt;
      _branchOffset = i.imm.imm;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _branchOffset <<= 16; _branchOffset >>= 14; _bd = 1; _btgt = (unsigned)((signed)pc+_branchOffset+4);
      break;

   case 2:			// j
      _opControl = func_j;
      _branchOffset = i.tgt.tgt;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _btgt = ((pc+4) & 0xf0000000) | (_branchOffset<<2); _bd = 1;
      break;

   case 3:			// jal
      _opControl = func_jal;
      _branchOffset = i.tgt.tgt;
      _decodedDST = 31;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      _btgt = ((pc+4) & 0xf0000000) | (_branchOffset<<2); _bd = 1;
      break;

   case 0x20:			// lb  
      _opControl = func_lb;
      _memOp = mem_lb;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x24:			// lbu
      _opControl = func_lbu;
      _memOp = mem_lbu;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x21:			// lh
      _opControl = func_lh;
      _memOp = mem_lh;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x25:			// lhu
      _opControl = func_lhu;
      _memOp = mem_lhu;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x22:			// lwl
      _opControl = func_lwl;
      _memOp = mem_lwl;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _subregOperand = _gpr[i.reg.rt];
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      src_reg2 = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x23:			// lw
      _opControl = func_lw;
      _memOp = mem_lw;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x26:			// lwr
      _opControl = func_lwr;
      _memOp = mem_lwr;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _subregOperand = _gpr[i.reg.rt];
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      src_reg2 = i.reg.rt;
      _writeREG = TRUE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x31:			// lwc1
      _opControl = func_lwc1;
      _memOp = mem_lwc1;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = TRUE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x39:			// swc1
      _opControl = func_swc1;
      _memOp = mem_swc1;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x28:			// sb
      _opControl = func_sb;
      _memOp = mem_sb;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x29:			// sh  store half word
      _opControl = func_sh;
      _memOp = mem_sh;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x2a:			// swl
      _opControl = func_swl;
      _memOp = mem_swl;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x2b:			// sw
      _opControl = func_sw;
      _memOp = mem_sw;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x2e:			// swr
      _opControl = func_swr;
      _memOp = mem_swr;
      _decodedSRC1 = _gpr[i.reg.rs];
      _decodedSRC2 = i.imm.imm;
      _decodedDST = i.reg.rt;
      src_reg1 = i.reg.rs;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = TRUE;
      break;

   case 0x11:			// floating-point
      _fpinst++;
      switch (i.freg.fmt) {
      case 4:			// mtc1
         _opControl = func_mtc1;
         _decodedSRC1 = _gpr[i.freg.ft];
         _decodedDST = i.freg.fs;
          is_fpt = TRUE;
          fpt_src_reg = i.freg.ft;
         _writeREG = FALSE;
         _writeFREG = TRUE;
         _hiWPort = FALSE;
         _loWPort = FALSE;
         _memControl = FALSE;
	 break;

      case 0:			// mfc1
         _opControl = func_mfc1;
         _decodedSRC1 = _fpr[(i.freg.fs)>>1].l[FP_TWIDDLE^((i.freg.fs)&1)];
         _decodedDST = i.freg.ft;
          is_fpt = TRUE;
          fpt_src_reg = i.freg.fs;
         _writeREG = TRUE;
         _writeFREG = FALSE;
         _hiWPort = FALSE;
         _loWPort = FALSE;
         _memControl = FALSE;
	 break;
      default:
         _isIllegalOp = TRUE;
         _writeREG = FALSE;
         _writeFREG = FALSE;
         _hiWPort = FALSE;
         _loWPort = FALSE;
         _memControl = FALSE;
	 break;
      }
      break;
   default:
      _isIllegalOp = TRUE;
      _writeREG = FALSE;
      _writeFREG = FALSE;
      _hiWPort = FALSE;
      _loWPort = FALSE;
      _memControl = FALSE;
      break;
   }

   id_ex._ins = ins;
   id_ex._pc = pc;
   id_ex.dst_hi = dst_hi;
   id_ex.dst_lo = dst_lo;
   id_ex.is_hi_lo = is_hi_lo;
   id_ex.src_reg1 = src_reg1;
   id_ex.src_reg2 = src_reg2;
   id_ex.fpt_src_reg = fpt_src_reg;
   id_ex.is_fpt = is_fpt;
   id_ex._decodedSRC1 = _decodedSRC1;
   id_ex._decodedSRC2 = _decodedSRC2;
   id_ex._decodedDST = _decodedDST;
   id_ex._subregOperand = _subregOperand; 
   id_ex._memControl = _memControl;   
   id_ex._writeREG = _writeREG;
   id_ex._writeFREG = _writeFREG;     
   id_ex._branchOffset = _branchOffset;
   id_ex._hiWPort = _hiWPort;
   id_ex._loWPort = _loWPort;    
   id_ex._decodedShiftAmt = _decodedShiftAmt;    
   id_ex._bd = _bd;           
   id_ex._btaken = _btaken;        
   id_ex._btgt = _btgt;            
   id_ex._isSyscall = _isSyscall;       
   id_ex._isIllegalOp = _isIllegalOp; 
   id_ex._opControl = _opControl;       
   id_ex._memOp = _memOp;

    // printf("EXEC_HELPER : %d %d %d %d\n",
    //         id_ex._writeREG, 
    //         id_ex._writeFREG, 
    //         id_ex._loWPort, 
    //         id_ex._hiWPort);
}


/*
 *
 * Debugging: print registers
 *
 */
void 
Mipc::dumpregs (MEM_WB_REG* local_mem_wb)
{
   int i;

   printf ("\n--- PC = %08x ---\n",local_mem_wb->_pc);
   for (i=0; i < 32; i++) {
      if (i < 10)
	 printf (" r%d: %08x (%ld)\n", i, _gpr[i], _gpr[i]);
      else
	 printf ("r%d: %08x (%ld)\n", i, _gpr[i], _gpr[i]);
   }
   printf ("taken: %d, bd: %d\n", local_mem_wb->_btaken, local_mem_wb->_bd);
   printf ("target: %08x\n", local_mem_wb->_btgt);
}

void
Mipc::func_add_addu (Mipc *mc, ID_EX_REG* local_id_ex,unsigned ins)
{
   mc->ex_mem._opResultLo = (unsigned)(local_id_ex->_decodedSRC1 + local_id_ex->_decodedSRC2);
   // printf("Encountered unimplemented instruction: add or addu.\n");
   // printf("You need to fill in func_add_addu in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_and (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1 & local_id_ex->_decodedSRC2;
}

void
Mipc::func_nor (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = ~(local_id_ex->_decodedSRC1 | local_id_ex->_decodedSRC2);
}

void
Mipc::func_or (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1 | local_id_ex->_decodedSRC2;
}

void
Mipc::func_sll (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC2 << local_id_ex->_decodedShiftAmt;
}

void
Mipc::func_sllv (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC2 << (local_id_ex->_decodedSRC1 & 0x1f);
   // printf("Encountered unimplemented instruction: sllv.\n");
   // printf("You need to fill in func_sllv in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_slt (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   if (local_id_ex->_decodedSRC1 < local_id_ex->_decodedSRC2) {
      mc->ex_mem._opResultLo = 1;
   }
   else {
      mc->ex_mem._opResultLo = 0;
   }
}

void
Mipc::func_sltu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   if ((unsigned)local_id_ex->_decodedSRC1 < (unsigned)local_id_ex->_decodedSRC2) {
      mc->ex_mem._opResultLo = 1;
   }
   else {
      mc->ex_mem._opResultLo = 0;
   }
}

void
Mipc::func_sra (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC2 >> local_id_ex->_decodedShiftAmt;
}

void
Mipc::func_srav (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC2 >> (local_id_ex->_decodedSRC1 & 0x1f);
}

void
Mipc::func_srl (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = (unsigned)local_id_ex->_decodedSRC2 >> local_id_ex->_decodedShiftAmt;
}

void
Mipc::func_srlv (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = (unsigned)local_id_ex->_decodedSRC2 >> (local_id_ex->_decodedSRC1 & 0x1f);
}

void
Mipc::func_sub_subu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = (unsigned)local_id_ex->_decodedSRC1 - (unsigned)local_id_ex->_decodedSRC2;
}

void
Mipc::func_xor (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1 ^ local_id_ex->_decodedSRC2;
}

void
Mipc::func_div (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   if (local_id_ex->_decodedSRC2 != 0) {
      mc->ex_mem._opResultHi = (unsigned)(local_id_ex->_decodedSRC1 % local_id_ex->_decodedSRC2);
      mc->ex_mem._opResultLo = (unsigned)(local_id_ex->_decodedSRC1 / local_id_ex->_decodedSRC2);
   }
   else {
      mc->ex_mem._opResultHi = 0x7fffffff;
      mc->ex_mem._opResultLo = 0x7fffffff;
   }
}

void
Mipc::func_divu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   if ((unsigned)local_id_ex->_decodedSRC2 != 0) {
      mc->ex_mem._opResultHi = (unsigned)(local_id_ex->_decodedSRC1) % (unsigned)(local_id_ex->_decodedSRC2);
      mc->ex_mem._opResultLo = (unsigned)(local_id_ex->_decodedSRC1) / (unsigned)(local_id_ex->_decodedSRC2);
   }
   else {
      mc->ex_mem._opResultHi = 0x7fffffff;
      mc->ex_mem._opResultLo = 0x7fffffff;
   }
}

void
Mipc::func_mfhi (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = mc->_hi;
}

void
Mipc::func_mflo (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = mc->_lo;
}

void
Mipc::func_mthi (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultHi = local_id_ex->_decodedSRC1;
}

void
Mipc::func_mtlo (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC2;
}

void
Mipc::func_mult (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
                                                                                
   ar1 = local_id_ex->_decodedSRC1;
   ar2 = local_id_ex->_decodedSRC2;
   s1 = ar1 >> 31; if (s1) ar1 = 0x7fffffff & (~ar1 + 1);
   s2 = ar2 >> 31; if (s2) ar2 = 0x7fffffff & (~ar2 + 1);
                                                                                
   t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
   r1 = t1 & 0xffff;              // bottom 16 bits
                                                                                
   // compute next set of 16 bits
   t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
   t2 = (ar2 & 0xffff) * (ar1 >> 16);
                                                                                
   r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
   r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
            (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
                                                                                
   if (s1 ^ s2) {
      r1 = ~r1;
      r2 = ~r2;
      r1++;
      if (r1 == 0)
         r2++;
   }
   mc->ex_mem._opResultHi = r2;
   mc->ex_mem._opResultLo = r1;
}

void
Mipc::func_multu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
                                                                                
   ar1 = local_id_ex->_decodedSRC1;
   ar2 = local_id_ex->_decodedSRC2;
                                                                                
   t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
   r1 = t1 & 0xffff;              // bottom 16 bits
                                                                                
   // compute next set of 16 bits
   t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
   t2 = (ar2 & 0xffff) * (ar1 >> 16);
                                                                                
   r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
   r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
            (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
                            
   mc->ex_mem._opResultHi = r2;
   mc->ex_mem._opResultLo = r1;                                                    
}

void
Mipc::func_jalr (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   local_id_ex->_btaken = 1;
   mc->_num_jal++;
   mc->ex_mem._opResultLo = local_id_ex->_pc + 8;
}

void
Mipc::func_jr (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   local_id_ex->_btaken = 1;
   mc->_num_jr++;
}

void
Mipc::func_await_break (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
}

void
Mipc::func_syscall (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   // printf("FAKE!! PC : %#x\n",local_id_ex->_pc);
   mc->fake_syscall (ins, local_id_ex->_pc);
   // exit(0);
}

void
Mipc::func_addi_addiu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._opResultLo = (unsigned)(local_id_ex->_decodedSRC1 + local_id_ex->_decodedSRC2);
   // printf("Encountered unimplemented instruction: addi or addiu.\n");
   // printf("You need to fill in func_addi_addiu in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_andi (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1 & local_id_ex->_decodedSRC2;
}

void
Mipc::func_lui (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC2 << 16;
   // printf("Encountered unimplemented instruction: lui.\n");
   // printf("You need to fill in func_lui in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_ori (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1 | local_id_ex->_decodedSRC2;
   // printf("Encountered unimplemented instruction: ori.\n");
   // printf("You need to fill in func_ori in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_slti (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   if (local_id_ex->_decodedSRC1 < local_id_ex->_decodedSRC2) {
      mc->ex_mem._opResultLo = 1;
   }
   else {
      mc->ex_mem._opResultLo = 0;
   }
}

void
Mipc::func_sltiu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   if ((unsigned)local_id_ex->_decodedSRC1 < (unsigned)local_id_ex->_decodedSRC2) {
      mc->ex_mem._opResultLo = 1;
   }
   else {
      mc->ex_mem._opResultLo = 0;
   }
}

void
Mipc::func_xori (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1 ^ local_id_ex->_decodedSRC2;
}

void
Mipc::func_beq (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = (local_id_ex->_decodedSRC1==local_id_ex->_decodedSRC2)? 1 : 0;
   // printf("Encountered unimplemented instruction: beq.\n");
   // printf("You need to fill in func_beq in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_bgez (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = !(local_id_ex->_decodedSRC1 >> 31);
}

void
Mipc::func_bgezal (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = !(local_id_ex->_decodedSRC1 >> 31);
   mc->ex_mem._opResultLo = local_id_ex->_pc + 8;
}

void
Mipc::func_bltzal (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = (local_id_ex->_decodedSRC1 >> 31);
   mc->ex_mem._opResultLo = local_id_ex->_pc + 8;
}

void
Mipc::func_bltz (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = (local_id_ex->_decodedSRC1 >> 31);
}

void
Mipc::func_bgtz (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = (local_id_ex->_decodedSRC1 > 0);
}

void
Mipc::func_blez (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = (local_id_ex->_decodedSRC1 <= 0);
}

void
Mipc::func_bne (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_cond_br++;
   local_id_ex->_btaken = (local_id_ex->_decodedSRC1 != local_id_ex->_decodedSRC2);
}

void
Mipc::func_j (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   local_id_ex->_btaken = 1;
}

void
Mipc::func_jal (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_jal++;
   mc->ex_mem._opResultLo = local_id_ex->_pc + 8;
   local_id_ex->_btaken = 1;
   // printf("Encountered unimplemented instruction: jal.\n");
   // printf("You need to fill in func_jal in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_lb (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   signed int a1;

   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_lbu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_lh (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   signed int a1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_lhu (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_lwl (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   signed int a1;
   unsigned s1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_lw (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
   // printf("Encountered unimplemented instruction: lw.\n");
   // printf("You need to fill in func_lw in exec_helper.cc to proceed forward.\n");
   // exit(0);
}

void
Mipc::func_lwr (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_lwc1 (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_swc1 (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_sb (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_sh (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_swl (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_store++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_sw (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_swr (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_store++;
   SIGN_EXTEND_IMM(local_id_ex->_decodedSRC2);
   mc->ex_mem._MAR = (unsigned)(local_id_ex->_decodedSRC1+local_id_ex->_decodedSRC2);
}

void
Mipc::func_mtc1 (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1;
}

void
Mipc::func_mfc1 (Mipc *mc, ID_EX_REG* local_id_ex, unsigned ins)
{
   mc->ex_mem._opResultLo = local_id_ex->_decodedSRC1;
}



void
Mipc::mem_lb (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   signed int a1;

   a1 = mc->_mem->BEGetByte(local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
   SIGN_EXTEND_BYTE(a1);
   local_ex_mem->_opResultLo = a1;
}

void
Mipc::mem_lbu (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   local_ex_mem->_opResultLo = mc->_mem->BEGetByte(local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
}

void
Mipc::mem_lh (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   signed int a1;

   a1 = mc->_mem->BEGetHalfWord(local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
   SIGN_EXTEND_IMM(a1);
   local_ex_mem->_opResultLo = a1;
}

void
Mipc::mem_lhu (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   local_ex_mem->_opResultLo = mc->_mem->BEGetHalfWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
}

void
Mipc::mem_lwl (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   signed int a1;
   unsigned s1;

   a1 = mc->_mem->BEGetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
   s1 = (local_ex_mem->_MAR & 3) << 3;
   local_ex_mem->_opResultLo = (a1 << s1) | (local_ex_mem->_subregOperand & ~(~0UL << s1));
}

void
Mipc::mem_lw (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   local_ex_mem->_opResultLo = mc->_mem->BEGetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
}

void
Mipc::mem_lwr (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
   s1 = (~local_ex_mem->_MAR & 3) << 3;
   local_ex_mem->_opResultLo = (ar1 >> s1) | (local_ex_mem->_subregOperand & ~(~(unsigned)0 >> s1));
}

void
Mipc::mem_lwc1 (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   local_ex_mem->_opResultLo = mc->_mem->BEGetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
}

void
Mipc::mem_swc1 (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   mc->_mem->Write(local_ex_mem->_MAR & ~(LL)0x7, mc->_mem->BESetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7), mc->_fpr[local_ex_mem->_decodedDST>>1].l[FP_TWIDDLE^(local_ex_mem->_decodedDST&1)]));
}

void
Mipc::mem_sb (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   mc->_mem->Write(local_ex_mem->_MAR & ~(LL)0x7, mc->_mem->BESetByte (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7), mc->_gpr[local_ex_mem->_decodedDST] & 0xff));
}

void
Mipc::mem_sh (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   mc->_mem->Write(local_ex_mem->_MAR & ~(LL)0x7, mc->_mem->BESetHalfWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7), mc->_gpr[local_ex_mem->_decodedDST] & 0xffff));
}

void
Mipc::mem_swl (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
   s1 = (local_ex_mem->_MAR & 3) << 3;
   ar1 = (mc->_gpr[local_ex_mem->_decodedDST] >> s1) | (ar1 & ~(~(unsigned)0 >> s1));
   mc->_mem->Write(local_ex_mem->_MAR & ~(LL)0x7, mc->_mem->BESetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7), ar1));
}

void
Mipc::mem_sw (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   mc->_mem->Write(local_ex_mem->_MAR & ~(LL)0x7, mc->_mem->BESetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7), mc->_gpr[local_ex_mem->_decodedDST]));
}

void
Mipc::mem_swr (Mipc *mc, EX_MEM_REG* local_ex_mem)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7));
   s1 = (~local_ex_mem->_MAR & 3) << 3;
   ar1 = (mc->_gpr[local_ex_mem->_decodedDST] << s1) | (ar1 & ~(~0UL << s1));
   mc->_mem->Write(local_ex_mem->_MAR & ~(LL)0x7, mc->_mem->BESetWord (local_ex_mem->_MAR, mc->_mem->Read(local_ex_mem->_MAR & ~(LL)0x7), ar1));
}

