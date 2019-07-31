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

namespace debugger {

static const uint64_t CFG_VENDOR_ID         = 0x000000F1;
static const uint64_t CFG_IMPLEMENTATION_ID = 0x20190521;
static const bool CFG_HW_FPU_ENABLE         = true;

static const int RISCV_ARCH     = 64;

static const int BUS_ADDR_WIDTH = 32;
static const int BUS_DATA_WIDTH = 64;
static const int BUS_DATA_BYTES = BUS_DATA_WIDTH / 8;

/** ICacheLru config */
static const int CFG_IOFFSET_WIDTH   = 5;    // [4:0]  log2(ICACHE_LINE_BYTES)
static const int CFG_IODDEVEN_WIDTH  = 1;    // [5]    0=even; 1=odd
// [13:6]  8: index: 8 KB per odd/even ways (64 KB icache) 75565 drhy
// [12:6]  7: index: 4 KB per odd/even ways (32 KB icache) 75565
// [11:6]  6: index: 2 KB per odd/even ways (16 KB icache) 75565,87462
static const int CFG_IINDEX_WIDTH    = 6;    // log2(LINES_PER_WAY) odd or even
static const int CFG_ILINES_PER_WAY  = 1 << CFG_IINDEX_WIDTH;
// [31:14] tag when 64 KB
// [31:13] tag when 32 KB
// [31:12] tag when 16 KB
static const int CFG_ITAG_WIDTH      = BUS_ADDR_WIDTH
    - (CFG_IOFFSET_WIDTH + CFG_IODDEVEN_WIDTH + CFG_IINDEX_WIDTH);

static const int CFG_ICACHE_WAYS        = 4;  // 4 odds, 4 even
/** Store tag data as:
       [3:0]            qword is valid flag
       [4]              load_fault
       [ITAG_WIDTH+5:5] tag value
 */
static const int CFG_ITAG_WIDTH_TOTAL = CFG_ITAG_WIDTH + 5;

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

static const int DBG_FETCH_TRACE_SIZE   = 4;

/** Number of elements each 2*CFG_ADDR_WIDTH in stack trace buffer: */
static const int CFG_STACK_TRACE_BUF_SIZE = 32;

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
    Instr_REM,
    Instr_REMU,
    Instr_REMW,
    Instr_REMUW,
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

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_CFG_H__
