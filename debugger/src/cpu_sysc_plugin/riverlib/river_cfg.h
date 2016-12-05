/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      "River" CPU configuration parameters
 */

#ifndef __DEBUGGER_RIVER_CFG_H__
#define __DEBUGGER_RIVER_CFG_H__

#include <systemc.h>

namespace debugger {

#define GENERATE_VCD 0
/*
 * Generate memory access and registers modifications trace files to compare
 * them with functional model
 *
 * @note When this define is enabled Core uses step counter instead 
 *       of clock counter to generate callbacks.
 */
#define GENERATE_CORE_TRACE 0

static const int RISCV_ARCH     = 64;

static const int BUS_ADDR_WIDTH = 32;
static const int BUS_DATA_WIDTH = 64;
static const int BUS_DATA_BYTES = BUS_DATA_WIDTH / 8;

static const uint8_t MEMOP_8B = 3;
static const uint8_t MEMOP_4B = 2;
static const uint8_t MEMOP_2B = 1;
static const uint8_t MEMOP_1B = 0;

static const uint64_t RESET_VECTOR      = 0x1000;

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

    Instr_Total
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_CFG_H__
