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

#ifndef __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
#define __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_CPU_RISCV = "ICpuRiscV";

/** Signal types */
//static const int CPU_SIGNAL_RESET   = 0;
//static const int CPU_SIGNAL_EXT_IRQ = 1;

class ICpuRiscV : public IFace {
public:
    ICpuRiscV() : IFace(IFACE_CPU_RISCV) {}

    // Read/Write Control Status Registers
    virtual uint64_t readCSR(uint32_t regno) = 0;
    virtual void writeCSR(uint32_t regno, uint64_t val) = 0;
    // Read/Write General Purpose Registers + FPU registers
    virtual uint64_t readGPR(uint32_t regno) = 0;
    virtual void writeGPR(uint32_t regno, uint64_t val) = 0;
    // Read/Write Non-standard extension registers
    virtual uint64_t readNonStandardReg(uint32_t regno) = 0;
    virtual void writeNonStandardReg(uint32_t regno, uint64_t val) = 0;

    // atomic instruction LR/SC reservation
    virtual void mmuAddrReserve(uint64_t addr) = 0;
    virtual bool mmuAddrRelease(uint64_t addr) = 0;

    enum ERiscvRegNames {
        Reg_Zero,
        Reg_ra,       // [1] Return address
        Reg_sp,       // [2] Stack pointer
        Reg_gp,       // [3] Global pointer
        Reg_tp,       // [4] Thread pointer
        Reg_t0,       // [5] Temporaries 0 s3
        Reg_t1,       // [6] Temporaries 1 s4
        Reg_t2,       // [7] Temporaries 2 s5
        Reg_s0,       // [8] s0/fp Saved register/frame pointer
        Reg_s1,       // [9] Saved register 1
        Reg_a0,       // [10] Function argumentes 0
        Reg_a1,       // [11] Function argumentes 1
        Reg_a2,       // [12] Function argumentes 2
        Reg_a3,       // [13] Function argumentes 3
        Reg_a4,       // [14] Function argumentes 4
        Reg_a5,       // [15] Function argumentes 5
        Reg_a6,       // [16] Function argumentes 6
        Reg_a7,       // [17] Function argumentes 7
        Reg_s2,       // [18] Saved register 2
        Reg_s3,       // [19] Saved register 3
        Reg_s4,       // [20] Saved register 4
        Reg_s5,       // [21] Saved register 5
        Reg_s6,       // [22] Saved register 6
        Reg_s7,       // [23] Saved register 7
        Reg_s8,       // [24] Saved register 8
        Reg_s9,       // [25] Saved register 9
        Reg_s10,      // [26] Saved register 10
        Reg_s11,      // [27] Saved register 11
        Reg_t3,       // [28]
        Reg_t4,       // [29]
        Reg_t5,       // [30]
        Reg_t6,       // [31]
        Reg_Total
    };

    static const int RegFpu_Offset = Reg_Total;

    enum ERiscvRegFpuNames {
        Reg_f0,     // ft0 temporary register
        Reg_f1,     // ft1
        Reg_f2,     // ft2
        Reg_f3,     // ft3
        Reg_f4,     // ft4
        Reg_f5,     // ft5
        Reg_f6,     // ft6
        Reg_f7,     // ft7
        Reg_f8,     // fs0 saved register
        Reg_f9,     // fs1
        Reg_f10,    // fa0 argument/return value
        Reg_f11,    // fa1 argument/return value
        Reg_f12,    // fa2 argument register
        Reg_f13,    // fa3
        Reg_f14,    // fa4
        Reg_f15,    // fa5
        Reg_f16,    // fa6
        Reg_f17,    // fa7
        Reg_f18,    // fs2 saved register
        Reg_f19,    // fs3
        Reg_f20,    // fs4
        Reg_f21,    // fs5
        Reg_f22,    // fs6
        Reg_f23,    // fs7
        Reg_f24,    // fs8
        Reg_f25,    // fs9
        Reg_f26,    // fs10
        Reg_f27,    // fs11
        Reg_f28,    // ft8 temporary register
        Reg_f29,    // ft9
        Reg_f30,    // ft10
        Reg_f31,    // ft11
        RegFpu_Total
    };

    /**
     * @name PRV bits possible values:
     */
    /// @{
    /// User-mode
    static const uint64_t PRV_U       = 0;
    /// super-visor mode
    static const uint64_t PRV_S       = 1;
    /// hyper-visor mode
    static const uint64_t PRV_H       = 2;
    //// machine mode
    static const uint64_t PRV_M       = 3;
    /// @}

    enum EExceptions {
        // Instruction address misaligned
        EXCEPTION_InstrMisalign,
        // Instruction access fault
        EXCEPTION_InstrFault,
        // Illegal instruction
        EXCEPTION_InstrIllegal,
        // Breakpoint
        EXCEPTION_Breakpoint,
        // Load address misaligned
        EXCEPTION_LoadMisalign,
        // Load access fault
        EXCEPTION_LoadFault,
        // Store/AMO address misaligned
        EXCEPTION_StoreMisalign,
        // Store/AMO access fault
        EXCEPTION_StoreFault,
        // Environment call from U-mode
        EXCEPTION_CallFromUmode,
        // Environment call from S-mode
        EXCEPTION_CallFromSmode,
        // Environment call from H-mode
        EXCEPTION_CallFromHmode,
        // Environment call from M-mode
        EXCEPTION_CallFromMmode,
        // Instruction page fault
        EXCEPTION_InstrPageFault,
        // Load page fault
        EXCEPTION_LoadPageFault,
        // reserved
        EXCEPTION_rsrv14,
        // Store/AMO page fault
        EXCEPTION_StorePageFault,
        // Stack overflow
        EXCEPTION_StackOverflow,
        // Stack underflow
        EXCEPTION_StackUnderflow,
        EXCEPTIONS_Total
    };

    enum EInterrupts {
        INTERRUPT_XSoftware,
        INTERRUPT_XTimer,
        INTERRUPT_XExternal,
        INTERRUPT_Total
    };
    /** Exceptions */
    enum ESignals {
        SIGNAL_Exception = 0,
        SIGNAL_XSoftware = EXCEPTIONS_Total + 4*INTERRUPT_XSoftware,
        SIGNAL_XTimer = EXCEPTIONS_Total + 4*INTERRUPT_XTimer,
        SIGNAL_XExternal = EXCEPTIONS_Total + 4*INTERRUPT_XExternal,
        SIGNAL_HardReset,
        SIGNAL_Total
    };

    static const int EXCEPTION_CallFromXMode    = EXCEPTION_CallFromUmode;

    /**
     * @name Use RISC-V CSR registers mapping for all platforms: ARM, Leon3, HC08 etc.
     */
    // CSR[11:10] indicate whether the register is read/write (00, 01, or 10) or read-only (11)
    // CSR[9:8] encode the lowest privilege level that can access the CSR
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
    /** Supervisor Address Translation and Protection */
    static const uint16_t CSR_satp           = 0x180;
    static const uint16_t CSR_hepc           = 0x241;
    static const uint16_t CSR_mepc           = 0x341;
    /** Machine trap cause */
    static const uint16_t CSR_mcause         = 0x342;
    /** Machine bad address or instruction. */
    static const uint16_t CSR_mtval          = 0x343;
    /** Machine interrupt pending */
    static const uint16_t CSR_mip            = 0x344;
    /** Physical memory protection */
    static const uint16_t CSR_pmpcfg0        = 0x3A0;
    static const uint16_t CSR_pmpcfg15       = 0x3AF;
    static const uint16_t CSR_pmpaddr0       = 0x3B0;
    static const uint16_t CSR_pmpaddr63      = 0x3EF;
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
    /** Non-stadnard usermode CSR:
       Flush specified address in I-cache module without execution of fence.i */
    static const uint16_t CSR_flushi         = 0x800;

    // Standard read/write Machine CSRs:
    /** Machine Cycle counter */
    static const uint16_t CSR_mcycle         = 0xB00;
    /** Machine Instructions-retired counter */
    static const uint16_t CSR_minsret        = 0xB02;

    // Non-standard machine mode CSR
    /** Stack overflow. */
    static const uint16_t CSR_mstackovr      = 0xBC0;
    /** Stack underflow (non-standard CSR). */
    static const uint16_t CSR_mstackund      = 0xBC1;
    /** MPU region address (non-standard CSR). */
    static const uint16_t CSR_mpu_addr       = 0xBC2;
    /** MPU region mask (non-standard CSR). */
    static const uint16_t CSR_mpu_mask       = 0xBC3;
    /** MPU region control (non-standard CSR). */
    static const uint16_t CSR_mpu_ctrl       = 0xBC4;

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

};

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
