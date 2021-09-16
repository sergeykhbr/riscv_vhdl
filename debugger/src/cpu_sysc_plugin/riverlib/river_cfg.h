/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_RIVER_CFG_H__
#define __DEBUGGER_RIVER_CFG_H__

#include <systemc.h>
#include "../ambalib/types_amba.h"   // to re-use CFG_BUS_ADDR_WIDTH

namespace debugger {

static const uint64_t CFG_VENDOR_ID         = 0x000000F1;
static const uint64_t CFG_IMPLEMENTATION_ID = 0x20191123;
static const bool CFG_HW_FPU_ENABLE         = true;

static const int RISCV_ARCH     = 64;

static const int CFG_CPU_ADDR_BITS       = CFG_BUS_ADDR_WIDTH;
static const int CFG_CPU_ID_BITS         = 1;
static const int CFG_CPU_USER_BITS       = 1;

/** 
 * ICacheLru config (16 KB by default)
 */
static const int CFG_ILOG2_BYTES_PER_LINE = 5;    // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int CFG_ILOG2_LINES_PER_WAY  = 7;
static const int CFG_ILOG2_NWAYS          = 2;

// Derivatives I$ constants:
static const int ICACHE_BYTES_PER_LINE    = 1 << CFG_ILOG2_BYTES_PER_LINE;
static const int ICACHE_LINES_PER_WAY     = 1 << CFG_ILOG2_LINES_PER_WAY;
static const int ICACHE_WAYS              = 1 << CFG_ILOG2_NWAYS;

static const int ICACHE_LINE_BITS         = 8*ICACHE_BYTES_PER_LINE;

// Information: To define the CACHE SIZE in Bytes use the following:
static const int ICACHE_SIZE_BYTES =
    ICACHE_WAYS * ICACHE_LINES_PER_WAY * ICACHE_BYTES_PER_LINE;

static const int ITAG_FL_TOTAL      = 1;


/** 
 * DCacheLru config (16 KB by default)
 */
static const int CFG_DLOG2_BYTES_PER_LINE = 5;    // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int CFG_DLOG2_LINES_PER_WAY  = 7;
static const int CFG_DLOG2_NWAYS          = 2;

// Derivatives D$ constants:
static const int DCACHE_BYTES_PER_LINE    = 1 << CFG_DLOG2_BYTES_PER_LINE;
static const int DCACHE_LINES_PER_WAY     = 1 << CFG_DLOG2_LINES_PER_WAY;
static const int DCACHE_WAYS              = 1 << CFG_DLOG2_NWAYS;

static const int DCACHE_LINE_BITS         = 8*DCACHE_BYTES_PER_LINE;

// Information: To define the CACHE SIZE in Bytes use the following:
static const int DCACHE_SIZE_BYTES =
    DCACHE_WAYS * DCACHE_LINES_PER_WAY * DCACHE_BYTES_PER_LINE;

static const int TAG_FL_VALID       = 0;    // always 0
static const int DTAG_FL_DIRTY      = 1;
static const int DTAG_FL_SHARED     = 2;
static const int DTAG_FL_TOTAL      = 3;


/**
 * L1 cache common parameters (suppose I$ and D$ have the same size)
 */
static const int L1CACHE_BYTES_PER_LINE   = DCACHE_BYTES_PER_LINE;
static const int L1CACHE_LINE_BITS        = 8*DCACHE_BYTES_PER_LINE;

static const int REQ_MEM_TYPE_WRITE        = 0;
static const int REQ_MEM_TYPE_CACHED       = 1;
static const int REQ_MEM_TYPE_UNIQUE       = 2;
static const int REQ_MEM_TYPE_BITS         = 3;

static const int SNOOP_REQ_TYPE_READDATA     = 0;   // 0=check flags; 1=data transfer
static const int SNOOP_REQ_TYPE_READCLEAN    = 1;   // 0=do nothing; 1=read and invalidate line
static const int SNOOP_REQ_TYPE_BITS         = 2;

sc_uint<REQ_MEM_TYPE_BITS> ReadNoSnoop();
sc_uint<REQ_MEM_TYPE_BITS> ReadShared();
sc_uint<REQ_MEM_TYPE_BITS> ReadMakeUnique();
sc_uint<REQ_MEM_TYPE_BITS> WriteNoSnoop();
sc_uint<REQ_MEM_TYPE_BITS> WriteLineUnique();
sc_uint<REQ_MEM_TYPE_BITS> WriteBack();


/** 
 * L2 cache config (16 KB by default)
 */
static const int CFG_L2_LOG2_BYTES_PER_LINE = 5;    // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int CFG_L2_LOG2_LINES_PER_WAY  = 7;
static const int CFG_L2_LOG2_NWAYS          = 2;

// Derivatives D$ constants:
static const int L2CACHE_BYTES_PER_LINE    = 1 << CFG_L2_LOG2_BYTES_PER_LINE;
static const int L2CACHE_LINES_PER_WAY     = 1 << CFG_L2_LOG2_LINES_PER_WAY;
static const int L2CACHE_WAYS              = 1 << CFG_L2_LOG2_NWAYS;

static const int L2CACHE_LINE_BITS         = 8*L2CACHE_BYTES_PER_LINE;
static const int L2CACHE_SIZE_BYTES =
    L2CACHE_WAYS * L2CACHE_LINES_PER_WAY * L2CACHE_BYTES_PER_LINE;

static const int L2TAG_FL_DIRTY      = 1;
static const int L2TAG_FL_TOTAL      = 2;

static const int L2_REQ_TYPE_WRITE  = 0;
static const int L2_REQ_TYPE_CACHED = 1;
static const int L2_REQ_TYPE_UNIQUE = 2;
static const int L2_REQ_TYPE_SNOOP  = 3;    // Use data received through snoop channel (no memory request)
static const int L2_REQ_TYPE_BITS   = 4;

/** MPU config */
static const int CFG_MPU_TBL_WIDTH   = 2;    // [1:0]  log2(MPU_TBL_SIZE)
static const int CFG_MPU_TBL_SIZE    = 1 << CFG_MPU_TBL_WIDTH;
static const int CFG_MPU_FL_WR       = 0;
static const int CFG_MPU_FL_RD       = 1;
static const int CFG_MPU_FL_EXEC     = 2;
static const int CFG_MPU_FL_CACHABLE = 3;
static const int CFG_MPU_FL_ENA      = 4;
static const int CFG_MPU_FL_TOTAL    = 5;


static const uint8_t MEMOP_8B = 3;
static const uint8_t MEMOP_4B = 2;
static const uint8_t MEMOP_2B = 1;
static const uint8_t MEMOP_1B = 0;

/** Non-maskable interrupts (exceptions) table.
    It can be freely changed to optimize memory consumption/performance
 */
static const uint64_t CFG_NMI_RESET_VECTOR          = 0x0000;
static const uint64_t CFG_NMI_INSTR_UNALIGNED_ADDR  = 0x0008;
static const uint64_t CFG_NMI_INSTR_FAULT_ADDR      = 0x0010;
static const uint64_t CFG_NMI_INSTR_ILLEGAL_ADDR    = 0x0018;
static const uint64_t CFG_NMI_BREAKPOINT_ADDR       = 0x0020;
static const uint64_t CFG_NMI_LOAD_UNALIGNED_ADDR   = 0x0028;
static const uint64_t CFG_NMI_LOAD_FAULT_ADDR       = 0x0030;
static const uint64_t CFG_NMI_STORE_UNALIGNED_ADDR  = 0x0038;
static const uint64_t CFG_NMI_STORE_FAULT_ADDR      = 0x0040;
static const uint64_t CFG_NMI_CALL_FROM_UMODE_ADDR  = 0x0048;
static const uint64_t CFG_NMI_CALL_FROM_SMODE_ADDR  = 0x0050;
static const uint64_t CFG_NMI_CALL_FROM_HMODE_ADDR  = 0x0058;
static const uint64_t CFG_NMI_CALL_FROM_MMODE_ADDR  = 0x0060;
static const uint64_t CFG_NMI_INSTR_PAGE_FAULT_ADDR = 0x0068;
static const uint64_t CFG_NMI_LOAD_PAGE_FAULT_ADDR  = 0x0070;
static const uint64_t CFG_NMI_STORE_PAGE_FAULT_ADDR = 0x0078;
static const uint64_t CFG_NMI_STACK_OVERFLOW_ADDR   = 0x0080;
static const uint64_t CFG_NMI_STACK_UNDERFLOW_ADDR  = 0x0088;

/** Debug interface configuration: */
static const int CFG_DPORT_ADDR_BITS        = 16;
// Valid size 0..16
static const int CFG_PROGBUF_REG_TOTAL      = 16;
// Must be at least 2 to support RV64I
static const int CFG_DATA_REG_TOTAL         = 2;
/** Number of elements each 2*CFG_ADDR_WIDTH in stack trace buffer: */
static const int CFG_LOG2_STACK_TRACE_ADDR  = 5;
static const int STACK_TRACE_BUF_SIZE       = 1 << CFG_LOG2_STACK_TRACE_ADDR;

enum EIsaType {
    ISA_R_type,
    ISA_I_type,
    ISA_S_type,
    ISA_SB_type,
    ISA_U_type,
    ISA_UJ_type,
    ISA_Total
};

enum EInstuctionsType {
    Instr_ADD,
    Instr_ADDI,
    Instr_ADDIW,
    Instr_ADDW,
    Instr_AND,
    Instr_ANDI,
    Instr_AUIPC,
    Instr_BEQ,
    Instr_BGE,
    Instr_BGEU,
    Instr_BLT,
    Instr_BLTU,
    Instr_BNE,
    Instr_JAL,
    Instr_JALR,
    Instr_LB,
    Instr_LH,
    Instr_LW,
    Instr_LD,
    Instr_LBU,
    Instr_LHU,
    Instr_LWU,
    Instr_LUI,
    Instr_OR,
    Instr_ORI,
    Instr_SLLI,
    Instr_SLT,
    Instr_SLTI,
    Instr_SLTU,
    Instr_SLTIU,
    Instr_SLL,
    Instr_SLLW,
    Instr_SLLIW,
    Instr_SRA,
    Instr_SRAW,
    Instr_SRAI,
    Instr_SRAIW,
    Instr_SRL,
    Instr_SRLI,
    Instr_SRLIW,
    Instr_SRLW,
    Instr_SB,
    Instr_SH,
    Instr_SW,
    Instr_SD,
    Instr_SUB,
    Instr_SUBW,
    Instr_XOR,
    Instr_XORI,
    Instr_CSRRW,
    Instr_CSRRS,
    Instr_CSRRC,
    Instr_CSRRWI,
    Instr_CSRRCI,
    Instr_CSRRSI,
    Instr_URET,
    Instr_SRET,
    Instr_HRET,
    Instr_MRET,
    Instr_FENCE,
    Instr_FENCE_I,
    Instr_DIV,
    Instr_DIVU,
    Instr_DIVW,
    Instr_DIVUW,
    Instr_MUL,
    Instr_MULW,
    Instr_MULH,
    Instr_MULHSU,
    Instr_MULHU,
    Instr_REM,
    Instr_REMU,
    Instr_REMW,
    Instr_REMUW,
    Instr_AMOADD_W,
    Instr_AMOXOR_W,
    Instr_AMOOR_W,
    Instr_AMOAND_W,
    Instr_AMOMIN_W,
    Instr_AMOMAX_W,
    Instr_AMOMINU_W,
    Instr_AMOMAXU_W,
    Instr_AMOSWAP_W,
    Instr_LR_W,
    Instr_SC_W,
    Instr_AMOADD_D,
    Instr_AMOXOR_D,
    Instr_AMOOR_D,
    Instr_AMOAND_D,
    Instr_AMOMIN_D,
    Instr_AMOMAX_D,
    Instr_AMOMINU_D,
    Instr_AMOMAXU_D,
    Instr_AMOSWAP_D,
    Instr_LR_D,
    Instr_SC_D,
    Instr_ECALL,
    Instr_EBREAK,
    Instr_FADD_D,
    Instr_FCVT_D_W,
    Instr_FCVT_D_WU,
    Instr_FCVT_D_L,
    Instr_FCVT_D_LU,
    Instr_FCVT_W_D,
    Instr_FCVT_WU_D,
    Instr_FCVT_L_D,
    Instr_FCVT_LU_D,
    Instr_FDIV_D,
    Instr_FEQ_D,
    Instr_FLD,
    Instr_FLE_D,
    Instr_FLT_D,
    Instr_FMAX_D,
    Instr_FMIN_D,
    Instr_FMOV_D_X,
    Instr_FMOV_X_D,
    Instr_FMUL_D,
    Instr_FSD,
    Instr_FSUB_D,
    Instr_Total
};

static const int Instr_FPU_Total = Instr_FSUB_D - Instr_FADD_D + 1;

// Depth of the fifo between Executor and MemoryAccess modules.
static const int CFG_MEMACCESS_QUEUE_DEPTH = 2;
// Register's tag used to detect reg hazard and it should be higher than available
// slots in the fifo Executor => Memaccess.
static const int CFG_REG_TAG_WITH          = 3;

// Request type: [0]-read csr; [1]-write csr; [2]-change mode
static const int CsrReq_ReadBit        = 0;
static const int CsrReq_WriteBit       = 1;
static const int CsrReq_ChangeModeBit  = 2;
static const int CsrReq_PcBit          = 3;     // get instruction pointer
static const int CsrReq_TotalBits      = 4;

static const int CsrReq_ReadCmd        = 1 << CsrReq_ReadBit;
static const int CsrReq_WriteCmd       = 1 << CsrReq_WriteBit;
static const int CsrReq_ChaneModeCmd   = 1 << CsrReq_ChangeModeBit;
static const int CsrReq_PcCmd          = 1 << CsrReq_PcBit;

static const int CsrReq_PcCmd_UnsupInstruction = 0x001;
static const int CsrReq_PcCmd_EnvCall          = 0x002;

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_CFG_H__
