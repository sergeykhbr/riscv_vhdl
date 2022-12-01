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

namespace debugger {

static const uint32_t CFG_VENDOR_ID = 0x000000F1;
static const uint32_t CFG_IMPLEMENTATION_ID = 0x20220813;
static const bool CFG_HW_FPU_ENABLE = true;
static const bool CFG_TRACER_ENABLE = false;

// Architectural size definition
static const int RISCV_ARCH = 64;

static const int CFG_CPU_ADDR_BITS = 48;
static const int CFG_CPU_ID_BITS = 1;
static const int CFG_CPU_USER_BITS = 1;

// 
// 2**Number of CPU slots in the clusters. Some of them could be unavailable
// 
static const int CFG_LOG2_CPU_MAX = 2;                      // 1=Dual-core (maximum); 2=Quad-core (maximum), 3=..
static const int CFG_CPU_MAX = (1 << CFG_LOG2_CPU_MAX);
// +1 ACP coherent port
static const int CFG_SLOT_L1_TOTAL = (CFG_CPU_MAX + 1);

// Power-on start address can be free changed
static const uint64_t CFG_RESET_VECTOR = 0x10000ull;

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
// L1 cache common parameters (suppose I$ and D$ have the same size)
// 
static const int CFG_LOG2_L1CACHE_BYTES_PER_LINE = 5;       // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int L1CACHE_BYTES_PER_LINE = (1 << CFG_LOG2_L1CACHE_BYTES_PER_LINE);
static const int L1CACHE_LINE_BITS = (8 * L1CACHE_BYTES_PER_LINE);
// D-Cache flags:
static const int TAG_FL_VALID = 0;                          // always 0
static const int DTAG_FL_DIRTY = 1;
static const int DTAG_FL_SHARED = 2;
static const int DTAG_FL_RESERVED = 3;
static const int DTAG_FL_TOTAL = 4;
// I-Cache flags:
static const int ITAG_FL_TOTAL = 1;

static const int SNOOP_REQ_TYPE_READDATA = 0;               // 0=check flags; 1=data transfer
static const int SNOOP_REQ_TYPE_READCLEAN = 1;              // 0=do nothing; 1=read and invalidate line
static const int SNOOP_REQ_TYPE_BITS = 2;


// 
// L2 cache config (River 16 KB, 2 ways by default, Wasserfall 64 KB, 4 ways)
// 
static const int CFG_L2_LOG2_BYTES_PER_LINE = 5;            // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
static const int L2CACHE_BYTES_PER_LINE = (1 << CFG_L2_LOG2_BYTES_PER_LINE);
static const int L2CACHE_LINE_BITS = (8 * L2CACHE_BYTES_PER_LINE);

static const int L2TAG_FL_DIRTY = 1;
static const int L2TAG_FL_TOTAL = 2;

static const int L2_REQ_TYPE_WRITE = 0;
static const int L2_REQ_TYPE_CACHED = 1;
static const int L2_REQ_TYPE_UNIQUE = 2;
static const int L2_REQ_TYPE_SNOOP = 3;                     // Use data received through snoop channel (no memory request)
static const int L2_REQ_TYPE_BITS = 4;

// PMP config
static const int CFG_PMP_TBL_WIDTH = 3;                     // [1:0]  log2(MPU_TBL_SIZE)
static const int CFG_PMP_TBL_SIZE = (1 << CFG_PMP_TBL_WIDTH);

static const int CFG_PMP_FL_R = 0;
static const int CFG_PMP_FL_W = 1;
static const int CFG_PMP_FL_X = 2;
static const int CFG_PMP_FL_L = 3;
static const int CFG_PMP_FL_V = 4;
static const int CFG_PMP_FL_TOTAL = 5;

// MMU config. Fetch and Data pathes have its own MMU block
static const int CFG_MMU_TLB_AWIDTH = 6;                    // TLB memory address bus width
static const int CFG_MMU_TLB_SIZE = (1 << CFG_MMU_TLB_AWIDTH);// Number of PTE entries in a table
static const int CFG_MMU_PTE_DWIDTH = ((2 * RISCV_ARCH) - 12);// PTE entry size in bits
static const int CFG_MMU_PTE_DBYTES = (CFG_MMU_PTE_DWIDTH / 8);// PTE entry size in bytes


static const uint8_t MEMOP_8B = 3;
static const uint8_t MEMOP_4B = 2;
static const uint8_t MEMOP_2B = 1;
static const uint8_t MEMOP_1B = 0;

// Integer Registers specified by ISA
static const int REG_ZERO = 0;
static const int REG_RA = 1;                                // [1] Return address
static const int REG_SP = 2;                                // [2] Stack pointer
static const int REG_GP = 3;                                // [3] Global pointer
static const int REG_TP = 4;                                // [4] Thread pointer
static const int REG_T0 = 5;                                // [5] Temporaries 0 s3
static const int REG_T1 = 6;                                // [6] Temporaries 1 s4
static const int REG_T2 = 7;                                // [7] Temporaries 2 s5
static const int REG_S0 = 8;                                // [8] s0/fp Saved register/frame pointer
static const int REG_S1 = 9;                                // [9] Saved register 1
static const int REG_A0 = 10;                               // [10] Function argumentes 0
static const int REG_A1 = 11;                               // [11] Function argumentes 1
static const int REG_A2 = 12;                               // [12] Function argumentes 2
static const int REG_A3 = 13;                               // [13] Function argumentes 3
static const int REG_A4 = 14;                               // [14] Function argumentes 4
static const int REG_A5 = 15;                               // [15] Function argumentes 5
static const int REG_A6 = 16;                               // [16] Function argumentes 6
static const int REG_A7 = 17;                               // [17] Function argumentes 7
static const int REG_S2 = 18;                               // [18] Saved register 2
static const int REG_S3 = 19;                               // [19] Saved register 3
static const int REG_S4 = 20;                               // [20] Saved register 4
static const int REG_S5 = 21;                               // [21] Saved register 5
static const int REG_S6 = 22;                               // [22] Saved register 6
static const int REG_S7 = 23;                               // [23] Saved register 7
static const int REG_S8 = 24;                               // [24] Saved register 8
static const int REG_S9 = 25;                               // [25] Saved register 9
static const int REG_S10 = 26;                              // [26] Saved register 10
static const int REG_S11 = 27;                              // [27] Saved register 11
static const int REG_T3 = 28;                               // [28]
static const int REG_T4 = 29;                               // [29]
static const int REG_T5 = 30;                               // [30]
static const int REG_T6 = 31;                               // [31]

static const int REG_F0 = 0;                                // ft0 temporary register
static const int REG_F1 = 1;                                // ft1
static const int REG_F2 = 2;                                // ft2
static const int REG_F3 = 3;                                // ft3
static const int REG_F4 = 4;                                // ft4
static const int REG_F5 = 5;                                // ft5
static const int REG_F6 = 6;                                // ft6
static const int REG_F7 = 7;                                // ft7
static const int REG_F8 = 8;                                // fs0 saved register
static const int REG_F9 = 9;                                // fs1
static const int REG_F10 = 10;                              // fa0 argument/return value
static const int REG_F11 = 11;                              // fa1 argument/return value
static const int REG_F12 = 12;                              // fa2 argument register
static const int REG_F13 = 13;                              // fa3
static const int REG_F14 = 14;                              // fa4
static const int REG_F15 = 15;                              // fa5
static const int REG_F16 = 16;                              // fa6
static const int REG_F17 = 17;                              // fa7
static const int REG_F18 = 18;                              // fs2 saved register
static const int REG_F19 = 19;                              // fs3
static const int REG_F20 = 20;                              // fs4
static const int REG_F21 = 21;                              // fs5
static const int REG_F22 = 22;                              // fs6
static const int REG_F23 = 23;                              // fs7
static const int REG_F24 = 24;                              // fs8
static const int REG_F25 = 25;                              // fs9
static const int REG_F26 = 26;                              // fs10
static const int REG_F27 = 27;                              // fs11
static const int REG_F28 = 28;                              // ft8 temporary register
static const int REG_F29 = 29;                              // ft9
static const int REG_F30 = 30;                              // ft10
static const int REG_F31 = 31;                              // ft11

static const int INTREGS_TOTAL = 32;
static const int FPUREGS_OFFSET = INTREGS_TOTAL;
static const int FPUREGS_TOTAL = 32;

static const int REGS_BUS_WIDTH = 6;
static const int REGS_TOTAL = (1 << REGS_BUS_WIDTH);        // INTREGS_TOTAL + FPUREGS_TOTAL


// Instruction formats specified by ISA specification
static const int ISA_R_type = 0;
static const int ISA_I_type = 1;
static const int ISA_S_type = 2;
static const int ISA_SB_type = 3;
static const int ISA_U_type = 4;
static const int ISA_UJ_type = 5;
static const int ISA_Total = 6;

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
    Instr_SFENCE_VMA = 62,
    Instr_DIV = 63,
    Instr_DIVU = 64,
    Instr_DIVW = 65,
    Instr_DIVUW = 66,
    Instr_MUL = 67,
    Instr_MULW = 68,
    Instr_MULH = 69,
    Instr_MULHSU = 70,
    Instr_MULHU = 71,
    Instr_REM = 72,
    Instr_REMU = 73,
    Instr_REMW = 74,
    Instr_REMUW = 75,
    Instr_AMOADD_W = 76,
    Instr_AMOXOR_W = 77,
    Instr_AMOOR_W = 78,
    Instr_AMOAND_W = 79,
    Instr_AMOMIN_W = 80,
    Instr_AMOMAX_W = 81,
    Instr_AMOMINU_W = 82,
    Instr_AMOMAXU_W = 83,
    Instr_AMOSWAP_W = 84,
    Instr_LR_W = 85,
    Instr_SC_W = 86,
    Instr_AMOADD_D = 87,
    Instr_AMOXOR_D = 88,
    Instr_AMOOR_D = 89,
    Instr_AMOAND_D = 90,
    Instr_AMOMIN_D = 91,
    Instr_AMOMAX_D = 92,
    Instr_AMOMINU_D = 93,
    Instr_AMOMAXU_D = 94,
    Instr_AMOSWAP_D = 95,
    Instr_LR_D = 96,
    Instr_SC_D = 97,
    Instr_ECALL = 98,
    Instr_EBREAK = 99,
    Instr_FADD_D = 100,
    Instr_FCVT_D_W = 101,
    Instr_FCVT_D_WU = 102,
    Instr_FCVT_D_L = 103,
    Instr_FCVT_D_LU = 104,
    Instr_FCVT_W_D = 105,
    Instr_FCVT_WU_D = 106,
    Instr_FCVT_L_D = 107,
    Instr_FCVT_LU_D = 108,
    Instr_FDIV_D = 109,
    Instr_FEQ_D = 110,
    Instr_FLD = 111,
    Instr_FLE_D = 112,
    Instr_FLT_D = 113,
    Instr_FMAX_D = 114,
    Instr_FMIN_D = 115,
    Instr_FMOV_D_X = 116,
    Instr_FMOV_X_D = 117,
    Instr_FMUL_D = 118,
    Instr_FSD = 119,
    Instr_FSUB_D = 120,
    Instr_Total = 121
};

static const int Instr_FPU_First = Instr_FADD_D;
static const int Instr_FPU_Last = Instr_FSUB_D;
static const int Instr_FPU_Total = ((Instr_FSUB_D - Instr_FADD_D) + 1);


// @name PRV bits possible values:
// @{
// User-mode
static const uint8_t PRV_U = 0;
// super-visor mode
static const uint8_t PRV_S = 1;
// hyper-visor mode
static const uint8_t PRV_H = 2;
// machine mode
static const uint8_t PRV_M = 3;
// @}

// Dport request types:
static const int DPortReq_Write = 0;
static const int DPortReq_RegAccess = 1;
static const int DPortReq_MemAccess = 2;
static const int DPortReq_MemVirtual = 3;
static const int DPortReq_Progexec = 4;
static const int DPortReq_Total = 5;

// DCSR register halt causes:
static const uint8_t HALT_CAUSE_EBREAK = 1;                 // software breakpoint
static const uint8_t HALT_CAUSE_TRIGGER = 2;                // hardware breakpoint
static const uint8_t HALT_CAUSE_HALTREQ = 3;                // halt request via debug interface
static const uint8_t HALT_CAUSE_STEP = 4;                   // step done
static const uint8_t HALT_CAUSE_RESETHALTREQ = 5;           // not implemented

static const uint8_t PROGBUF_ERR_NONE = 0;                  // no error
static const uint8_t PROGBUF_ERR_BUSY = 1;                  // abstract command in progress
static const uint8_t PROGBUF_ERR_NOT_SUPPORTED = 2;         // Request command not supported
static const uint8_t PROGBUF_ERR_EXCEPTION = 3;             // Exception occurs while executing progbuf
static const uint8_t PROGBUF_ERR_HALT_RESUME = 4;           // Command cannot be executed because of wrong CPU state
static const uint8_t PROGBUF_ERR_BUS = 5;                   // Bus error occurs
static const uint8_t PROGBUF_ERR_OTHER = 7;                 // Other reason

static const int EXCEPTION_InstrMisalign = 0;               // Instruction address misaligned
static const int EXCEPTION_InstrFault = 1;                  // Instruction access fault
static const int EXCEPTION_InstrIllegal = 2;                // Illegal instruction
static const int EXCEPTION_Breakpoint = 3;                  // Breakpoint
static const int EXCEPTION_LoadMisalign = 4;                // Load address misaligned
static const int EXCEPTION_LoadFault = 5;                   // Load access fault
static const int EXCEPTION_StoreMisalign = 6;               // Store/AMO address misaligned
static const int EXCEPTION_StoreFault = 7;                  // Store/AMO access fault
static const int EXCEPTION_CallFromUmode = 8;               // Environment call from U-mode
static const int EXCEPTION_CallFromSmode = 9;               // Environment call from S-mode
static const int EXCEPTION_CallFromHmode = 10;              // Environment call from H-mode
static const int EXCEPTION_CallFromMmode = 11;              // Environment call from M-mode
static const int EXCEPTION_InstrPageFault = 12;             // Instruction page fault
static const int EXCEPTION_LoadPageFault = 13;              // Load page fault
static const int EXCEPTION_rsrv14 = 14;                     // reserved
static const int EXCEPTION_StorePageFault = 15;             // Store/AMO page fault
static const int EXCEPTION_StackOverflow = 16;              // Stack overflow
static const int EXCEPTION_StackUnderflow = 17;             // Stack underflow
static const int EXCEPTIONS_Total = 64;

static const int EXCEPTION_CallFromXMode = EXCEPTION_CallFromUmode;

// Per Hart Interrupt bus
static const int IRQ_SSIP = 1;                              // Supervisor software pening interrupt
static const int IRQ_MSIP = 3;                              // Machine software pening interrupt
static const int IRQ_STIP = 5;                              // Supervisor timer pening interrupt
static const int IRQ_MTIP = 7;                              // Machine timer pening interrupt
static const int IRQ_SEIP = 9;                              // Supervisor external pening interrupt
static const int IRQ_MEIP = 11;                             // Machine external pening interrupt
static const int IRQ_TOTAL = 16;                            // Total number of direct core interrupt requests
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
static const int CsrReq_FenceBit = 9;                       // one of Fence instructions
static const int CsrReq_TotalBits = 10;

static const uint16_t CsrReq_ReadCmd = (1 << CsrReq_ReadBit);
static const uint16_t CsrReq_WriteCmd = (1 << CsrReq_WriteBit);
static const uint16_t CsrReq_TrapReturnCmd = (1 << CsrReq_TrapReturnBit);
static const uint16_t CsrReq_ExceptionCmd = (1 << CsrReq_ExceptionBit);
static const uint16_t CsrReq_InterruptCmd = (1 << CsrReq_InterruptBit);
static const uint16_t CsrReq_BreakpointCmd = (1 << CsrReq_BreakpointBit);
static const uint16_t CsrReq_HaltCmd = (1 << CsrReq_HaltBit);
static const uint16_t CsrReq_ResumeCmd = (1 << CsrReq_ResumeBit);
static const uint16_t CsrReq_WfiCmd = (1 << CsrReq_WfiBit);
static const uint16_t CsrReq_FenceCmd = (1 << CsrReq_FenceBit);

static const int MemopType_Store = 0;                       // 0=load; 1=store
static const int MemopType_Locked = 1;                      // AMO instructions
static const int MemopType_Reserve = 2;                     // LS load with reserve
static const int MemopType_Release = 3;                     // SC store with release
static const int MemopType_Total = 4;

static const int REQ_MEM_TYPE_WRITE = 0;
static const int REQ_MEM_TYPE_CACHED = 1;
static const int REQ_MEM_TYPE_UNIQUE = 2;
static const int REQ_MEM_TYPE_BITS = 3;

static sc_uint<REQ_MEM_TYPE_BITS> ReadNoSnoop() {
    sc_uint<REQ_MEM_TYPE_BITS> ret;

    ret = 0;
    return ret;
}

static sc_uint<REQ_MEM_TYPE_BITS> ReadShared() {
    sc_uint<REQ_MEM_TYPE_BITS> ret;

    ret = 0;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    return ret;
}

static sc_uint<REQ_MEM_TYPE_BITS> ReadMakeUnique() {
    sc_uint<REQ_MEM_TYPE_BITS> ret;

    ret = 0;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    ret[REQ_MEM_TYPE_UNIQUE] = 1;
    return ret;
}

static sc_uint<REQ_MEM_TYPE_BITS> WriteNoSnoop() {
    sc_uint<REQ_MEM_TYPE_BITS> ret;

    ret = 0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    return ret;
}

static sc_uint<REQ_MEM_TYPE_BITS> WriteLineUnique() {
    sc_uint<REQ_MEM_TYPE_BITS> ret;

    ret = 0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    ret[REQ_MEM_TYPE_UNIQUE] = 1;
    return ret;
}

static sc_uint<REQ_MEM_TYPE_BITS> WriteBack() {
    sc_uint<REQ_MEM_TYPE_BITS> ret;

    ret = 0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    return ret;
}

}  // namespace debugger

