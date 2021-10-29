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

// CSR Debug Status/Control register
union CrGenericDebugControlType {
    uint64_t val;
    uint8_t u8[8];
    struct bits_type {
        uint64_t prv : 2;       // [1:0] 
        uint64_t step : 1;      // [2]
        uint64_t rsv5_3 : 3;    // [5:3]
        uint64_t cause : 3;     // [8:6]
        uint64_t stoptime : 1;  // [9]
        uint64_t stopcount : 1; // [10]
        uint64_t rsv11 : 1;     // [11]
        uint64_t ebreaku : 1;   // [12]
        uint64_t ebreaks : 1;   // [13]
        uint64_t ebreakh : 1;   // [14]
        uint64_t ebreakm : 1;   // [15]
        uint64_t rsv27_16 : 12; // [27:16]
        uint64_t xdebugver : 4; // [31:28] 0=no external debug support; 4=exists as in spec 0.13
        uint64_t rsv : 32;      // [63:32]
    } bits;
};

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

}  // namespace debugger

