// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "../ambalib/types_amba.h"

namespace debugger {

// 
// 2**Number of CPU slots in the clusters. Some of them could be unavailable
// 
static const int CFG_LOG2_CPU_MAX = 2;
static const int CFG_CPU_MAX = (1 << CFG_LOG2_CPU_MAX);

static const sc_uint<32> CFG_VENDOR_ID = 0x000000F1;
static const sc_uint<32> CFG_IMPLEMENTATION_ID = 0x20191123;
static const bool CFG_HW_FPU_ENABLE = true;

static const int RISCV_ARCH = 64;

static const int CFG_CPU_ADDR_BITS = CFG_BUS_ADDR_WIDTH;
static const int CFG_CPU_ID_BITS = 1;
static const int CFG_CPU_USER_BITS = 1;

// 
// Branch Predictor Branch Target Buffer (BTB) size
// 
static const int CFG_BTB_SIZE = 8;
// Branch predictor depth. It is better when it is equal to the pipeline depth excluding fetcher.
// Let it be equal to the decoder's history depth
static const int CFG_BP_DEPTH = 5;

// 
// Decoded instructions history buffer size in Decoder
// 
static const int CFG_DEC_DEPTH = (CFG_BP_DEPTH - 3);        // requested, fetching, fetched

// Power-on start address can be free changed
static const sc_uint<CFG_BUS_ADDR_WIDTH> CFG_RESET_VECTOR = 0x10000;

// Valid size 0..16
static const int CFG_PROGBUF_REG_TOTAL = 16;
// Must be at least 2 to support RV64I
static const int CFG_DATA_REG_TOTAL = 4;
// Total number of dscratch registers
static const int CFG_DSCRATCH_REG_TOTAL = 2;
// Number of elements each 2*CFG_ADDR_WIDTH in stack trace buffer:
static const int CFG_LOG2_STACK_TRACE_ADDR = 5;
static const int STACK_TRACE_BUF_SIZE = (1 << CFG_LOG2_STACK_TRACE_ADDR);

// 
// ICacheLru config (16 KB by default)
// 
static const int CFG_ILOG2_BYTES_PER_LINE = 5;              // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int CFG_ILOG2_LINES_PER_WAY = 7;
static const int CFG_ILOG2_NWAYS = 2;

// Derivatives I$ constants:
static const int ICACHE_BYTES_PER_LINE = (1 << CFG_ILOG2_BYTES_PER_LINE);
static const int ICACHE_LINES_PER_WAY = (1 << CFG_ILOG2_LINES_PER_WAY);
static const int ICACHE_WAYS = (1 << CFG_ILOG2_NWAYS);
static const int ICACHE_LINE_BITS = (8 * ICACHE_BYTES_PER_LINE);

// Information: To define the CACHE SIZE in Bytes use the following:
static const int ICACHE_SIZE_BYTES = (ICACHE_WAYS * (ICACHE_LINES_PER_WAY * ICACHE_BYTES_PER_LINE));

static const int ITAG_FL_TOTAL = 1;


// 
// DCacheLru config (16 KB by default)
// 
static const int CFG_DLOG2_BYTES_PER_LINE = 5;              // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int CFG_DLOG2_LINES_PER_WAY = 7;               // 7=16KB; 8=32KB; ..
static const int CFG_DLOG2_NWAYS = 2;

// Derivatives D$ constants:
static const int DCACHE_BYTES_PER_LINE = (1 << CFG_DLOG2_BYTES_PER_LINE);
static const int DCACHE_LINES_PER_WAY = (1 << CFG_DLOG2_LINES_PER_WAY);
static const int DCACHE_WAYS = (1 << CFG_DLOG2_NWAYS);

static const int DCACHE_LINE_BITS = (8 * DCACHE_BYTES_PER_LINE);

// Information: To define the CACHE SIZE in Bytes use the following:
static const int DCACHE_SIZE_BYTES = (DCACHE_WAYS * (DCACHE_LINES_PER_WAY * DCACHE_BYTES_PER_LINE));

static const int TAG_FL_VALID = 0;                          // always 0
static const int DTAG_FL_DIRTY = 1;
static const int DTAG_FL_SHARED = 2;
static const int DTAG_FL_RESERVED = 3;
static const int DTAG_FL_TOTAL = 4;


// 
// L1 cache common parameters (suppose I$ and D$ have the same size)
// 
static const int L1CACHE_BYTES_PER_LINE = DCACHE_BYTES_PER_LINE;
static const int L1CACHE_LINE_BITS = (8 * DCACHE_BYTES_PER_LINE);

static const int SNOOP_REQ_TYPE_READDATA = 0;               // 0=check flags; 1=data transfer
static const int SNOOP_REQ_TYPE_READCLEAN = 1;              // 0=do nothing; 1=read and invalidate line
static const int SNOOP_REQ_TYPE_BITS = 2;


// 
// L2 cache config (River 16 KB by default, Wasserfall 64 KB)
// 
static const int CFG_L2_LOG2_BYTES_PER_LINE = 5;            // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int CFG_L2_LOG2_LINES_PER_WAY = 7;             // 7=16KB; 8=32KB; 9=64KB, ..
static const int CFG_L2_LOG2_NWAYS = 2;

// Derivatives D$ constants:
static const int L2CACHE_BYTES_PER_LINE = (1 << CFG_L2_LOG2_BYTES_PER_LINE);
static const int L2CACHE_LINES_PER_WAY = (1 << CFG_L2_LOG2_LINES_PER_WAY);
static const int L2CACHE_WAYS = (1 << CFG_L2_LOG2_NWAYS);

static const int L2CACHE_LINE_BITS = (8 * L2CACHE_BYTES_PER_LINE);
static const int L2CACHE_SIZE_BYTES = (L2CACHE_WAYS * (L2CACHE_LINES_PER_WAY * L2CACHE_BYTES_PER_LINE));

static const int L2TAG_FL_DIRTY = 1;
static const int L2TAG_FL_TOTAL = 2;

static const int L2_REQ_TYPE_WRITE = 0;
static const int L2_REQ_TYPE_CACHED = 1;
static const int L2_REQ_TYPE_UNIQUE = 2;
static const int L2_REQ_TYPE_SNOOP = 3;                     // Use data received through snoop channel (no memory request)
static const int L2_REQ_TYPE_BITS = 4;

// MPU config
static const int CFG_MPU_TBL_WIDTH = 3;                     // [1:0]  log2(MPU_TBL_SIZE)
static const int CFG_MPU_TBL_SIZE = (1 << CFG_MPU_TBL_WIDTH);

static const int CFG_MPU_FL_WR = 0;
static const int CFG_MPU_FL_RD = 1;
static const int CFG_MPU_FL_EXEC = 2;
static const int CFG_MPU_FL_CACHABLE = 3;
static const int CFG_MPU_FL_ENA = 4;
static const int CFG_MPU_FL_TOTAL = 5;


enum EnumMemopSize {
    MEMOP_1B = 0,
    MEMOP_2B = 1,
    MEMOP_4B = 2,
    MEMOP_8B = 3
};

// Dport request types:
static const int DPortReq_Write = 0;
static const int DPortReq_RegAccess = 1;
static const int DPortReq_MemAccess = 2;
static const int DPortReq_MemVirtual = 3;
static const int DPortReq_Progexec = 4;
static const int DPortReq_Total = 5;

// Instruction formats specified by ISA specification
enum EIsaType {
    ISA_R_type = 0,
    ISA_I_type = 1,
    ISA_S_type = 2,
    ISA_SB_type = 3,
    ISA_U_type = 4,
    ISA_UJ_type = 5,
    ISA_Total = 6
};

// Implemented instruction list and its indexes
enum EInstructionType {
    Instr_ADD = 0,
    Instr_ADDI = 1,
    Instr_ADDIW = 2,
    Instr_ADDW = 3,
    Instr_AND = 4,
    Instr_ANDI = 5,
    Instr_AUIPC = 6,
    Instr_BEQ = 7,
    Instr_BGE = 8,
    Instr_BGEU = 9,
    Instr_BLT = 10,
    Instr_BLTU = 11,
    Instr_BNE = 12,
    Instr_JAL = 13,
    Instr_JALR = 14,
    Instr_LB = 15,
    Instr_LH = 16,
    Instr_LW = 17,
    Instr_LD = 18,
    Instr_LBU = 19,
    Instr_LHU = 20,
    Instr_LWU = 21,
    Instr_LUI = 22,
    Instr_OR = 23,
    Instr_ORI = 24,
    Instr_SLLI = 25,
    Instr_SLT = 26,
    Instr_SLTI = 27,
    Instr_SLTU = 28,
    Instr_SLTIU = 29,
    Instr_SLL = 30,
    Instr_SLLW = 31,
    Instr_SLLIW = 32,
    Instr_SRA = 33,
    Instr_SRAW = 34,
    Instr_SRAI = 35,
    Instr_SRAIW = 36,
    Instr_SRL = 37,
    Instr_SRLI = 38,
    Instr_SRLIW = 39,
    Instr_SRLW = 40,
    Instr_SB = 41,
    Instr_SH = 42,
    Instr_SW = 43,
    Instr_SD = 44,
    Instr_SUB = 45,
    Instr_SUBW = 46,
    Instr_XOR = 47,
    Instr_XORI = 48,
    Instr_CSRRW = 49,
    Instr_CSRRS = 50,
    Instr_CSRRC = 51,
    Instr_CSRRWI = 52,
    Instr_CSRRCI = 53,
    Instr_CSRRSI = 54,
    Instr_URET = 55,
    Instr_SRET = 56,
    Instr_HRET = 57,
    Instr_MRET = 58,
    Instr_FENCE = 59,
    Instr_FENCE_I = 60,
    Instr_WFI = 61,
    Instr_DIV = 62,
    Instr_DIVU = 63,
    Instr_DIVW = 64,
    Instr_DIVUW = 65,
    Instr_MUL = 66,
    Instr_MULW = 67,
    Instr_MULH = 68,
    Instr_MULHSU = 69,
    Instr_MULHU = 70,
    Instr_REM = 71,
    Instr_REMU = 72,
    Instr_REMW = 73,
    Instr_REMUW = 74,
    Instr_AMOADD_W = 75,
    Instr_AMOXOR_W = 76,
    Instr_AMOOR_W = 77,
    Instr_AMOAND_W = 78,
    Instr_AMOMIN_W = 79,
    Instr_AMOMAX_W = 80,
    Instr_AMOMINU_W = 81,
    Instr_AMOMAXU_W = 82,
    Instr_AMOSWAP_W = 83,
    Instr_LR_W = 84,
    Instr_SC_W = 85,
    Instr_AMOADD_D = 86,
    Instr_AMOXOR_D = 87,
    Instr_AMOOR_D = 88,
    Instr_AMOAND_D = 89,
    Instr_AMOMIN_D = 90,
    Instr_AMOMAX_D = 91,
    Instr_AMOMINU_D = 92,
    Instr_AMOMAXU_D = 93,
    Instr_AMOSWAP_D = 94,
    Instr_LR_D = 95,
    Instr_SC_D = 96,
    Instr_ECALL = 97,
    Instr_EBREAK = 98,
    Instr_FADD_D = 99,
    Instr_FCVT_D_W = 100,
    Instr_FCVT_D_WU = 101,
    Instr_FCVT_D_L = 102,
    Instr_FCVT_D_LU = 103,
    Instr_FCVT_W_D = 104,
    Instr_FCVT_WU_D = 105,
    Instr_FCVT_L_D = 106,
    Instr_FCVT_LU_D = 107,
    Instr_FDIV_D = 108,
    Instr_FEQ_D = 109,
    Instr_FLD = 110,
    Instr_FLE_D = 111,
    Instr_FLT_D = 112,
    Instr_FMAX_D = 113,
    Instr_FMIN_D = 114,
    Instr_FMOV_D_X = 115,
    Instr_FMOV_X_D = 116,
    Instr_FMUL_D = 117,
    Instr_FSD = 118,
    Instr_FSUB_D = 119,
    Instr_Total = 120
};

static const int Instr_FPU_Total = ((Instr_FSUB_D - Instr_FADD_D) + 1);

// Depth of the fifo between Executor and MemoryAccess modules.
static const int CFG_MEMACCESS_QUEUE_DEPTH = 2;
// Register's tag used to detect reg hazard and it should be higher than available
// slots in the fifo Executor => Memaccess.
static const int CFG_REG_TAG_WIDTH = 3;

// Request type: [0]-read csr; [1]-write csr; [2]-change mode
static const int CsrReq_ReadBit = 0;
static const int CsrReq_WriteBit = 1;
static const int CsrReq_TrapReturnBit = 2;
static const int CsrReq_ExceptionBit = 3;                   // return instruction pointer
static const int CsrReq_InterruptBit = 4;                   // return instruction pointer
static const int CsrReq_BreakpointBit = 5;
static const int CsrReq_HaltBit = 6;
static const int CsrReq_ResumeBit = 7;
static const int CsrReq_WfiBit = 8;                         // wait for interrupt
static const int CsrReq_TotalBits = 9;

static const sc_uint<CsrReq_TotalBits> CsrReq_ReadCmd = (1 << CsrReq_ReadBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_WriteCmd = (1 << CsrReq_WriteBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_TrapReturnCmd = (1 << CsrReq_TrapReturnBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_ExceptionCmd = (1 << CsrReq_ExceptionBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_InterruptCmd = (1 << CsrReq_InterruptBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_BreakpointCmd = (1 << CsrReq_BreakpointBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_HaltCmd = (1 << CsrReq_HaltBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_ResumeCmd = (1 << CsrReq_ResumeBit);
static const sc_uint<CsrReq_TotalBits> CsrReq_WfiCmd = (1 << CsrReq_WfiBit);

static const int MemopType_Store = 0;                       // 0=load; 1=store
static const int MemopType_Locked = 1;                      // AMO instructions
static const int MemopType_Reserve = 2;                     // LS load with reserve
static const int MemopType_Release = 3;                     // SC store with release
static const int MemopType_Total = 4;

}  // namespace debugger

