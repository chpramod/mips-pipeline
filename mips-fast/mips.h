#ifndef __MIPS_H__
#define __MIPS_H__

#include "sim.h"

class Mipc;
class MipcSysCall;
class SysCall;

typedef unsigned Bool;
#define TRUE 1
#define FALSE 0

#if BYTE_ORDER == LITTLE_ENDIAN

#define FP_TWIDDLE 0

#else

#define FP_TWIDDLE 1

#define REG_HI 100
#define REG_LO 101
#define REG_BOTH 102

#endif

#include "mem.h"
#include "../../common/syscall.h"
#include "queue.h"

#define MIPC_DEBUG 1

struct ID_EX_REG;
struct EX_MEM_REG;
struct MEM_WB_REG;
struct IF_ID_REG;
struct EX_IF_BYPASS;

struct IF_ID_REG{
   unsigned int _ins;
   unsigned int _pc;
};

/* EX/MEM register */
struct EX_MEM_REG{
   unsigned int _ins;
   unsigned int _pc;
   signed int  _decodedSRC1, _decodedSRC2;   // Reg fetch output (source values)
   unsigned _decodedDST;         // Decoder output (dest reg no)
   unsigned    _subregOperand;         // Needed for lwl and lwr
   unsigned _MAR;          // Memory address register
   unsigned _opResultHi, _opResultLo;  // Result of operation
   Bool  _memControl;         // Memory instruction?
   Bool     _writeREG, _writeFREG;     // WB control
   signed int  _branchOffset;
   Bool  _hiWPort, _loWPort;     // WB control
   unsigned _decodedShiftAmt;    // Shift amount
   unsigned int _hi, _lo;        // mult, div destination
   int      _btaken;          // taken branch (1 if taken, 0 if fall-through)
   int      _bd;           // 1 if the next ins is delay slot
   unsigned int   _btgt;            // branch target
   Bool     _isSyscall;       // 1 if system call
   Bool     _isIllegalOp;        // 1 if illegal opcode
   void (*_memOp)(Mipc*,EX_MEM_REG*);
   void (*_opControl)(Mipc*, ID_EX_REG*,unsigned);
};

struct ID_EX_REG{
   unsigned int _ins;
   unsigned int _pc;
   unsigned int src_reg1, src_reg2,fpt_src_reg,dst_hi,dst_lo;
   Bool is_fpt;
   Bool is_hi_lo;
   signed int  _decodedSRC1, _decodedSRC2;   // Reg fetch output (source values)
   unsigned _decodedDST;         // Decoder output (dest reg no)
   unsigned    _subregOperand;         // Needed for lwl and lwr
   unsigned _MAR;          // Memory address register
   unsigned _opResultHi, _opResultLo;  // Result of operation
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
};

struct MEM_WB_REG{
   unsigned int _ins;
   unsigned int _pc;
   signed int  _decodedSRC1, _decodedSRC2;   // Reg fetch output (source values)
   unsigned _decodedDST;         // Decoder output (dest reg no)
   unsigned    _subregOperand;         // Needed for lwl and lwr
   unsigned _MAR;          // Memory address register
   unsigned _opResultHi, _opResultLo;  // Result of operation
   Bool  _memControl;         // Memory instruction?
   Bool     _writeREG, _writeFREG;     // WB control
   signed int  _branchOffset;
   Bool  _hiWPort, _loWPort;     // WB control
   unsigned _decodedShiftAmt;    // Shift amount
   unsigned int _hi, _lo;        // mult, div destination
   int      _btaken;          // taken branch (1 if taken, 0 if fall-through)
   int      _bd;           // 1 if the next ins is delay slot
   unsigned int   _btgt;            // branch target
   Bool     _isSyscall;       // 1 if system call
   Bool     _isIllegalOp;        // 1 if illegal opcode
   void (*_memOp)(Mipc*,EX_MEM_REG*);
   void (*_opControl)(Mipc*, ID_EX_REG*,unsigned);
};

struct EX_IF_BYPASS{
   int      _btaken;          // taken branch (1 if taken, 0 if fall-through)
   int      _bd;           // 1 if the next ins is delay slot
   unsigned int   _btgt;            // branch target
};


class Mipc : public SimObject {
public:
   Mipc (Mem *m);
   ~Mipc ();
  
   FAKE_SIM_TEMPLATE;

   MipcSysCall *_sys;		// Emulated system call layer

   void dumpregs (MEM_WB_REG*);	// Dumps current register state

   void Reboot (char *image = NULL);
				// Restart processor.
				// "image" = file name for new memory
				// image if any.

   void MipcDumpstats();			// Prints simulation statistics
   void Dec (unsigned int ins,unsigned int pc);			// Decoder function
   void fake_syscall (unsigned int ins, unsigned int pc);	// System call interface

   /* processor state */
   unsigned int _ins;   // instruction register
   // Bool         _insValid;      // Needed for unpipelined design
   // Bool         _decodeValid;   // Needed for unpipelined design
   // Bool		_execValid;	// Needed for unpipelined design
   // Bool		_memValid;	// Needed for unpipelined design
   // Bool         _insDone;       // Needed for unpipelined design

   /* IF/ID register */
   IF_ID_REG if_id;

   /* ID/EX register */
   ID_EX_REG empty_id_ex;
   ID_EX_REG id_ex;

   /* EX/MEM register */
   EX_MEM_REG ex_mem;

   EX_IF_BYPASS ex_if_bypass;
   /* MEM/WB register */
   MEM_WB_REG mem_wb;

   /* syscall handlers */
   Bool got_syscall;
   Bool got_branch;
   Bool refetch;
   
   signed int	_decodedSRC1, _decodedSRC2;	// Reg fetch output (source values)
   unsigned	_decodedDST;			// Decoder output (dest reg no)
   unsigned 	_subregOperand;			// Needed for lwl and lwr
   unsigned	_MAR;				// Memory address register
   unsigned	_opResultHi, _opResultLo;	// Result of operation
   Bool 	_memControl;			// Memory instruction?
   Bool		_writeREG, _writeFREG;		// WB control
   signed int	_branchOffset;
   Bool 	_hiWPort, _loWPort;		// WB control
   unsigned	_decodedShiftAmt;		// Shift amount

   unsigned int 	_gpr[32];		// general-purpose integer registers

   union {
      unsigned int l[2];
      float f[2];
      double d;
   } _fpr[16];					// floating-point registers (paired)

   unsigned int _hi, _lo; 			// mult, div destination
   unsigned int	_pc;				// Program counter
   unsigned int _boot;				// boot code loaded?

   int 		_btaken;// 			// taken branch (1 if taken, 0 if fall-through)
   int 		_bd;//				// 1 if the next ins is delay slot
   unsigned int	_btgt;//				// branch target

   Bool		_isSyscall;			// 1 if system call
   Bool		_isIllegalOp;			// 1 if illegal opcode

   // Simulation statistics counters

   LL	_nfetched;
   LL	_num_cond_br;
   LL	_num_jal;
   LL	_num_jr;
   LL   _num_load;
   LL   _num_store;
   LL   _fpinst;

   Mem	*_mem;	// attached memory (not a cache)

   Log	_l;
   int  _sim_exit;		// 1 on normal termination

   void (*_opControl)(Mipc*,ID_EX_REG*,unsigned);
   void (*_memOp)(Mipc*,EX_MEM_REG*);

   FILE *_debugLog;

   // EXE stage definitions

   static void func_add_addu (Mipc*, ID_EX_REG*, unsigned);
   static void func_and (Mipc*, ID_EX_REG*, unsigned);
   static void func_nor (Mipc*, ID_EX_REG*, unsigned);
   static void func_or (Mipc*, ID_EX_REG*, unsigned);
   static void func_sll (Mipc*, ID_EX_REG*, unsigned);
   static void func_sllv (Mipc*, ID_EX_REG*, unsigned);
   static void func_slt (Mipc*, ID_EX_REG*, unsigned);
   static void func_sltu (Mipc*, ID_EX_REG*, unsigned);
   static void func_sra (Mipc*, ID_EX_REG*, unsigned);
   static void func_srav (Mipc*, ID_EX_REG*, unsigned);
   static void func_srl (Mipc*, ID_EX_REG*, unsigned);
   static void func_srlv (Mipc*, ID_EX_REG*, unsigned);
   static void func_sub_subu (Mipc*, ID_EX_REG*, unsigned);
   static void func_xor (Mipc*, ID_EX_REG*, unsigned);
   static void func_div (Mipc*, ID_EX_REG*, unsigned);
   static void func_divu (Mipc*, ID_EX_REG*, unsigned);
   static void func_mfhi (Mipc*, ID_EX_REG*, unsigned);
   static void func_mflo (Mipc*, ID_EX_REG*, unsigned);
   static void func_mthi (Mipc*, ID_EX_REG*, unsigned);
   static void func_mtlo (Mipc*, ID_EX_REG*, unsigned);
   static void func_mult (Mipc*, ID_EX_REG*, unsigned);
   static void func_multu (Mipc*, ID_EX_REG*, unsigned);
   static void func_jalr (Mipc*, ID_EX_REG*, unsigned);
   static void func_jr (Mipc*, ID_EX_REG*, unsigned);
   static void func_await_break (Mipc*, ID_EX_REG*, unsigned);
   static void func_syscall (Mipc*, ID_EX_REG*, unsigned);
   static void func_addi_addiu (Mipc*, ID_EX_REG*, unsigned);
   static void func_andi (Mipc*, ID_EX_REG*, unsigned);
   static void func_lui (Mipc*, ID_EX_REG*, unsigned);
   static void func_ori (Mipc*, ID_EX_REG*, unsigned);
   static void func_slti (Mipc*, ID_EX_REG*, unsigned);
   static void func_sltiu (Mipc*, ID_EX_REG*, unsigned);
   static void func_xori (Mipc*, ID_EX_REG*, unsigned);
   static void func_beq (Mipc*, ID_EX_REG*, unsigned);
   static void func_bgez (Mipc*, ID_EX_REG*, unsigned);
   static void func_bgezal (Mipc*, ID_EX_REG*, unsigned);
   static void func_bltzal (Mipc*, ID_EX_REG*, unsigned);
   static void func_bltz (Mipc*, ID_EX_REG*, unsigned);
   static void func_bgtz (Mipc*, ID_EX_REG*, unsigned);
   static void func_blez (Mipc*, ID_EX_REG*, unsigned);
   static void func_bne (Mipc*, ID_EX_REG*, unsigned);
   static void func_j (Mipc*, ID_EX_REG*, unsigned);
   static void func_jal (Mipc*, ID_EX_REG*, unsigned);
   static void func_lb (Mipc*, ID_EX_REG*, unsigned);
   static void func_lbu (Mipc*, ID_EX_REG*, unsigned);
   static void func_lh (Mipc*, ID_EX_REG*, unsigned);
   static void func_lhu (Mipc*, ID_EX_REG*, unsigned);
   static void func_lwl (Mipc*, ID_EX_REG*, unsigned);
   static void func_lw (Mipc*, ID_EX_REG*, unsigned);
   static void func_lwr (Mipc*, ID_EX_REG*, unsigned);
   static void func_lwc1 (Mipc*, ID_EX_REG*, unsigned);
   static void func_swc1 (Mipc*, ID_EX_REG*, unsigned);
   static void func_sb (Mipc*, ID_EX_REG*, unsigned);
   static void func_sh (Mipc*, ID_EX_REG*, unsigned);
   static void func_swl (Mipc*, ID_EX_REG*, unsigned);
   static void func_sw (Mipc*, ID_EX_REG*, unsigned);
   static void func_swr (Mipc*, ID_EX_REG*, unsigned);
   static void func_mtc1 (Mipc*, ID_EX_REG*, unsigned);
   static void func_mfc1 (Mipc*, ID_EX_REG*, unsigned);

   // MEM stage definitions

   static void mem_lb (Mipc*, EX_MEM_REG*);
   static void mem_lbu (Mipc*, EX_MEM_REG*);
   static void mem_lh (Mipc*, EX_MEM_REG*);
   static void mem_lhu (Mipc*, EX_MEM_REG*);
   static void mem_lwl (Mipc*, EX_MEM_REG*);
   static void mem_lw (Mipc*, EX_MEM_REG*);
   static void mem_lwr (Mipc*, EX_MEM_REG*);
   static void mem_lwc1 (Mipc*, EX_MEM_REG*);
   static void mem_swc1 (Mipc*, EX_MEM_REG*);
   static void mem_sb (Mipc*, EX_MEM_REG*);
   static void mem_sh (Mipc*, EX_MEM_REG*);
   static void mem_swl (Mipc*, EX_MEM_REG*);
   static void mem_sw (Mipc*, EX_MEM_REG*);
   static void mem_swr (Mipc*, EX_MEM_REG*);

   // Zero Out function
   void zeroOutIF_ID();
   void zeroOutID_EX();
   void zeroOutEX_MEM();
   void zeroOutMEM_WB();
   void zeroOutEX_IF_BYPASS();
};


// Emulated system call interface

class MipcSysCall : public SysCall {
public:

   MipcSysCall (Mipc *ms) {

      char buf[1024];
      m = ms->_mem;
      _ms = ms;
      _num_load = 0;
      _num_store = 0;
   };

   ~MipcSysCall () { };

   LL GetDWord (LL addr);
   void SetDWord (LL addr, LL data);

   Word GetWord (LL addr);
   void SetWord (LL addr, Word data);
  
   void SetReg (int reg, LL val);
   LL GetReg (int reg);
   LL GetTime (void);

private:

   Mipc *_ms;
};
#endif /* __MIPS_H__ */
