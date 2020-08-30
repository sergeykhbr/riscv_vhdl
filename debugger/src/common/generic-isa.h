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
/** Machine bad address. */
static const uint16_t CSR_mbadaddr       = 0x343;
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
/** Halt/resume requests handling */
static const uint16_t CSR_runcontrol     = 0x355;
/** Instruction per single step */
static const uint16_t CSR_insperstep     = 0x356;
/** Write value into progbuf */
static const uint16_t CSR_progbuf        = 0x357;
/** Abstract commmand control status (ABSTRACTCS) */
static const uint16_t CSR_abstractcs     = 0x358;
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
/** ISA and extensions supported. */
static const uint16_t CSR_misa              = 0xf10;
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


}  // namespace debugger

