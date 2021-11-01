/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include <inttypes.h>

namespace debugger {

static const uint64_t EXT_SIGN_5  = 0xFFFFFFFFFFFFFFF0LL;
static const uint64_t EXT_SIGN_6  = 0xFFFFFFFFFFFFFFE0LL;
static const uint64_t EXT_SIGN_8  = 0xFFFFFFFFFFFFFF80LL;
static const uint64_t EXT_SIGN_9  = 0xFFFFFFFFFFFFFF00LL;
static const uint64_t EXT_SIGN_11 = 0xFFFFFFFFFFFFF800LL;
static const uint64_t EXT_SIGN_12 = 0xFFFFFFFFFFFFF000LL;
static const uint64_t EXT_SIGN_16 = 0xFFFFFFFFFFFF0000LL;
static const uint64_t EXT_SIGN_32 = 0xFFFFFFFF00000000LL;

/**
 * @name Use RISC-V CSR registers mapping for all platforms: ARM, Leon3, HC08 etc.
 */
/// @{
/** FPU Accrued Exceptions fields from FCSR */
static const uint16_t CSR_fflags         = 0x001;
/** FPU dynamic Rounding Mode fields from FCSR */
static const uint16_t CSR_frm            = 0x002;
/** FPU Control and Status register (frm + fflags) */
static const uint16_t CSR_fcsr           = 0x003;
/** machine mode status read/write register. */
static const uint16_t CSR_mstatus        = 0x300;
/** ISA and extensions supported. */
static const uint16_t CSR_misa           = 0x301;
/** Machine exception delegation  */
static const uint16_t CSR_medeleg        = 0x302;
/** Machine interrupt delegation  */
static const uint16_t CSR_mideleg        = 0x303;
/** Machine interrupt enable */
static const uint16_t CSR_mie            = 0x304;
/** The base address of the M-mode trap vector. */
static const uint16_t CSR_mtvec          = 0x305;
/** Scratch register for machine trap handlers. */
static const uint16_t CSR_mscratch       = 0x340;
/** Exception program counters. */
static const uint16_t CSR_uepc           = 0x041;
static const uint16_t CSR_sepc           = 0x141;
static const uint16_t CSR_hepc           = 0x241;
static const uint16_t CSR_mepc           = 0x341;
/** Machine trap cause */
static const uint16_t CSR_mcause         = 0x342;
/** Machine bad address or instruction. */
static const uint16_t CSR_mtval          = 0x343;
/** Machine interrupt pending */
static const uint16_t CSR_mip            = 0x344;
/** Stack overflow (non-standard CSR). */
static const uint16_t CSR_mstackovr      = 0x350;
/** Stack underflow (non-standard CSR). */
static const uint16_t CSR_mstackund      = 0x351;
/** MPU region address (non-standard CSR). */
static const uint16_t CSR_mpu_addr       = 0x352;
/** MPU region mask (non-standard CSR). */
static const uint16_t CSR_mpu_mask       = 0x353;
/** MPU region control (non-standard CSR). */
static const uint16_t CSR_mpu_ctrl       = 0x354;
/** Flush specified address in I-cache module without execution of fence.i */
static const uint16_t CSR_flushi         = 0x359;
// Software reset.
static const uint16_t CSR_mreset         = 0x782;
// Trigger select
static const uint16_t CSR_tselect        = 0x7a0;
// Trigger data1
static const uint16_t CSR_tdata1         = 0x7a1;
// Trigger data2
static const uint16_t CSR_tdata2         = 0x7a2;
// Trigger extra (RV64)
static const uint16_t CSR_textra         = 0x7a3;
// Trigger info
static const uint16_t CSR_tinfo          = 0x7a4;
// Trigger control
static const uint16_t CSR_tcontrol       = 0x7a5;
// Machine context
static const uint16_t CSR_mcontext       = 0x7a8;
// Supervisor context
static const uint16_t CSR_scontext       = 0x7aa;
// Debug Control and status
static const uint16_t CSR_dcsr           = 0x7b0;
// Debug PC
static const uint16_t CSR_dpc            = 0x7b1;
// Debug Scratch Register 0
static const uint16_t CSR_dscratch0      = 0x7b2;
// Debug Scratch Register 1
static const uint16_t CSR_dscratch1      = 0x7b3;
/** Machine Cycle counter */
static const uint16_t CSR_mcycle         = 0xB00;
/** Machine Instructions-retired counter */
static const uint16_t CSR_minsret        = 0xB02;
/** User Cycle counter for RDCYCLE pseudo-instruction */
static const uint16_t CSR_cycle          = 0xC00;
/** User Timer for RDTIME pseudo-instruction */
static const uint16_t CSR_time           = 0xC01;
/** User Instructions-retired counter for RDINSTRET pseudo-instruction */
static const uint16_t CSR_insret         = 0xC02;
/** 0xC00 to 0xC1F reserved for counters */
/** Vendor ID. */
static const uint16_t CSR_mvendorid         = 0xf11;
/** Architecture ID. */
static const uint16_t CSR_marchid           = 0xf12;
/** Vendor ID. */
static const uint16_t CSR_mimplementationid = 0xf13;
/** Thread id (the same as core). */
static const uint16_t CSR_mhartid           = 0xf14;
/// @}

// DCSR register halt causes:
static const uint64_t HALT_CAUSE_EBREAK       = 1;  // software breakpoint
static const uint64_t HALT_CAUSE_TRIGGER      = 2;  // hardware breakpoint
static const uint64_t HALT_CAUSE_HALTREQ      = 3;  // halt request via debug interface
static const uint64_t HALT_CAUSE_STEP         = 4;  // step done
static const uint64_t HALT_CAUSE_RESETHALTREQ = 5;  // not implemented

static const uint64_t PROGBUF_ERR_NONE = 0;         // no error
static const uint64_t PROGBUF_ERR_BUSY = 1;         // abstract command in progress
static const uint64_t PROGBUF_ERR_NOT_SUPPORTED = 2;// Request command not supported
static const uint64_t PROGBUF_ERR_EXCEPTION = 3;    // Exception occurs while executing progbuf
static const uint64_t PROGBUF_ERR_HALT_RESUME = 4;  // Command cannot be executed because of wrong CPU state
static const uint64_t PROGBUF_ERR_BUS = 5;          // Bus error occurs
static const uint64_t PROGBUF_ERR_OTHER = 7;        // Other reason


const uint32_t REG_ADDR_ERROR = 0xFFFFFFFFul;

struct ECpuRegMapping {
    const char name[16];
    int size;
    uint32_t offset;
};

// Trigger Data1
union TriggerData1Type {
    uint64_t val;
    uint8_t u8[8];
    struct bits_type {
        uint64_t data : 59;     // [58:0]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } bitsdef;
    struct bits_type2 {
        uint64_t load : 1;      // [0]
        uint64_t store : 1;     // [1]
        uint64_t execute : 1;   // [2]
        uint64_t u : 1;         // [3]
        uint64_t s : 1;         // [4]
        uint64_t rsr5 : 1;      // [5]
        uint64_t m : 1;         // [6]
        uint64_t match : 4;     // [10:7]
        uint64_t chain : 1;     // [11]
        uint64_t action : 4;    // [15:12]
        uint64_t sizelo : 2;    // [17:16]
        uint64_t timing : 1;    // [18]
        uint64_t select : 1;    // [19]
        uint64_t hit : 1;       // [20]
        uint64_t sizehi : 2;    // [22:21]
        uint64_t rsrv_23 : 30;  // [52:23]
        uint64_t maskmax : 6;   // [58:53]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } mcontrol_bits;
    struct bits_type3 {
        uint64_t action : 6;    // [5:0]: 0=raise breakpoint exception; 1=Enter Debug Mode
        uint64_t u : 1;         // [6]
        uint64_t s : 1;         // [7]
        uint64_t rsr5 : 1;      // [8]
        uint64_t m : 1;         // [9]
        uint64_t count : 14;    // [23:10]
        uint64_t hit : 1;       // [24]
        uint64_t rsrv57_10 : 34;// [58:25]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } icount_bits;
    struct bits_type4 {         // the same for type4 and type5
        uint64_t action : 6;    // [5:0]: 0=raise breakpoint exception; 1=Enter Debug Mode
        uint64_t u : 1;         // [6]
        uint64_t s : 1;         // [7]
        uint64_t rsr5 : 1;      // [8]
        uint64_t m : 1;         // [9]
        uint64_t rsrv57_10 : 48;// [57:10]
        uint64_t hit : 1;       // [58]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } itrigger_bits;
};

}  // namespace debugger

