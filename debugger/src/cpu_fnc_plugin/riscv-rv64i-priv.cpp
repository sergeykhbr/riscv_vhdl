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
 *
 * @brief      Base ISA implementation (extension I, privileged level).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

/** 
 * @brief The CSRRC (Atomic Read and Clear Bit in CSR).
 *
 * Instruction reads the value of the CSR, zeroextends the value to XLEN bits,
 * and writes it to integer register rd. The initial value in integer
 * register rs1 specifies bit positions to be cleared in the CSR. Any bit that
 * is high in rs1 will cause the corresponding bit to be cleared in the CSR,
 * if that CSR bit is writable. Other bits in the CSR are unaffected.
 */
class CSRRC : public RiscvInstruction {
public:
    CSRRC(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "CSRRC", "?????????????????011?????1110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];

        uint64_t clr_mask = ~R[u.bits.rs1];
        uint64_t csr = icpu_->readCSR(u.bits.imm);
        if (u.bits.rd) {
            icpu_->setReg(u.bits.rd, csr);
        }
        icpu_->writeCSR(u.bits.imm, (csr & clr_mask));
        return 4;
    }
};

/** 
 * @brief The CSRRCI (Atomic Read and Clear Bit in CSR immediate).
 *
 * Similar to CSRRC except it updates the CSR using a 5-bit zero-extended 
 * immediate (zimm[4:0]) encoded in the rs1 field instead of a value from 
 * an integer register.
 */
class CSRRCI : public RiscvInstruction {
public:
    CSRRCI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "CSRRCI", "?????????????????111?????1110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];

        uint64_t clr_mask = ~static_cast<uint64_t>((u.bits.rs1));
        uint64_t csr = icpu_->readCSR(u.bits.imm);
        if (u.bits.rd) {
            icpu_->setReg(u.bits.rd, csr);
        }
        icpu_->writeCSR(u.bits.imm, (csr & clr_mask));
        return 4;
    }
};

/**
 * @brief The CSRRS (Atomic Read and Set Bit in CSR).
 *
 *   Instruction reads the value of the CSR, zero-extends the value to XLEN 
 * bits, and writes it to integer register rd. The initial value in integer 
 * register rs1 specifies bit positions to be set in the CSR. Any bit that is
 * high in rs1 will cause the corresponding bit to be set in the CSR, if that
 * CSR bit is writable. Other bits in the CSR are unaffected (though CSRs 
 * might have side effects when written).
 *   The CSRR pseudo instruction (read CSR), when rs1 = 0.
 */
class CSRRS : public RiscvInstruction {
public:
    CSRRS(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "CSRRS", "?????????????????010?????1110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];

        uint64_t set_mask = R[u.bits.rs1];
        uint64_t csr = icpu_->readCSR(u.bits.imm);
        if (u.bits.rd) {
            icpu_->setReg(u.bits.rd, csr);
        }
        icpu_->writeCSR(u.bits.imm, (csr | set_mask));
        return 4;
    }
};

/**
 * @brief The CSRRSI (Atomic Read and Set Bit in CSR immediate).
 *
 * Similar to CSRRS except it updates the CSR using a 5-bit zero-extended 
 * immediate (zimm[4:0]) encoded in the rs1 field instead of a value from 
 * an integer register.
 */
class CSRRSI : public RiscvInstruction {
public:
    CSRRSI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "CSRRSI", "?????????????????110?????1110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];

        uint64_t set_mask = u.bits.rs1;
        uint64_t csr = icpu_->readCSR(u.bits.imm);
        if (u.bits.rd) {
            icpu_->setReg(u.bits.rd, csr);
        }
        icpu_->writeCSR(u.bits.imm, (csr | set_mask));
        return 4;
    }
};

/** 
 * @brief The CSRRW (Atomic Read/Write CSR).
 *
 *   Instruction atomically swaps values in the CSRs and integer registers. 
 * CSRRW reads the old value of the CSR, zero-extends the value to XLEN bits,
 * then writes it to integer register rd. The initial value in rs1 is written
 * to the CSR.
 *   The CSRW pseudo instruction (write CSR), when rs1 = 0.
 */
class CSRRW : public RiscvInstruction {
public:
    CSRRW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "CSRRW", "?????????????????001?????1110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];

        uint64_t wr_value = R[u.bits.rs1];
        if (u.bits.rd) {
            icpu_->setReg(u.bits.rd, icpu_->readCSR(u.bits.imm));
        }
        icpu_->writeCSR(u.bits.imm, wr_value);
        return 4;
    }
};

/** 
 * @brief The CSRRWI (Atomic Read/Write CSR immediate).
 *
 * Similar to CSRRW except it updates the CSR using a 5-bit zero-extended 
 * immediate (zimm[4:0]) encoded in the rs1 field instead of a value from 
 * an integer register.
 */
class CSRRWI : public RiscvInstruction {
public:
    CSRRWI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "CSRRWI", "?????????????????101?????1110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];

        uint64_t wr_value = u.bits.rs1;
        if (u.bits.rd) {
            icpu_->setReg(u.bits.rd, icpu_->readCSR(u.bits.imm));
        }
        icpu_->writeCSR(u.bits.imm, wr_value);
        return 4;
    }
};

/** 
 * @brief MRET, HRET, SRET, or URET
 *
 * These instructions are used to return from traps in M-mode, Hmode, 
 * S-mode, or U-mode respectively. When executing an xRET instruction, 
 * supposing x PP holds the value y, y IE is set to x PIE; the privilege 
 * mode is changed to y; x PIE is set to 1; and x PP is set to U 
 * (or M if user-mode is not supported).
 *
 * User-level interrupts are an optional extension and have been allocated 
 * the ISA extension letter N. If user-level interrupts are omitted, the UIE 
 * and UPIE bits are hardwired to zero. For all other supported privilege 
 * modes x, the x IE, x PIE, and x PP fields are required to be implemented.
 */
class URET : public RiscvInstruction {
public:
    URET(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "URET", "00000000001000000000000001110011") {}

    virtual int exec(Reg64Type *payload) {
        if (icpu_->getPrvLevel() != PRV_U) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
            return 4;
        }
        csr_mstatus_type mstatus;
        mstatus.value = icpu_->readCSR(CSR_mstatus);

        uint64_t xepc = (PRV_U << 8) + 0x41;
        icpu_->setBranch(icpu_->readCSR(static_cast<uint32_t>(xepc)));

        bool is_N_extension = false;
        if (is_N_extension) {
            mstatus.bits.UIE = mstatus.bits.UPIE;
            mstatus.bits.UPIE = 1;
            // User mode not changed.
        } else {
            mstatus.bits.UIE = 0;
            mstatus.bits.UPIE = 0;
        }
        icpu_->setPrvLevel(PRV_U);
        icpu_->writeCSR(CSR_mstatus, mstatus.value);
        return 4;
    }
};

/**
 * @brief SRET return from super-user mode
 */
class SRET : public RiscvInstruction {
public:
    SRET(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRET", "00010000001000000000000001110011") {}

    virtual int exec(Reg64Type *payload) {
        if (icpu_->getPrvLevel() != PRV_S) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
            return 4;
        }
        csr_mstatus_type mstatus;
        mstatus.value = icpu_->readCSR(CSR_mstatus);

        uint64_t xepc = (PRV_S << 8) + 0x41;
        icpu_->setBranch(icpu_->readCSR(static_cast<uint32_t>(xepc)));

        mstatus.bits.SIE = mstatus.bits.SPIE;
        mstatus.bits.SPIE = 1;
        icpu_->setPrvLevel(mstatus.bits.SPP);
        mstatus.bits.SPP = PRV_U;
            
        icpu_->writeCSR(CSR_mstatus, mstatus.value);
        return 4;
    }
};

/**
 * @brief HRET return from hypervisor mode
 */
class HRET : public RiscvInstruction {
public:
    HRET(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "HRET", "00100000001000000000000001110011") {}

    virtual int exec(Reg64Type *payload) {
        if (icpu_->getPrvLevel() != PRV_H) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
            return 4;
        }
        csr_mstatus_type mstatus;
        mstatus.value = icpu_->readCSR(CSR_mstatus);

        uint64_t xepc = (PRV_H << 8) + 0x41;
        icpu_->setBranch(icpu_->readCSR(static_cast<uint32_t>(xepc)));

        mstatus.bits.HIE = mstatus.bits.HPIE;
        mstatus.bits.HPIE = 1;
        icpu_->setPrvLevel(mstatus.bits.HPP);
        mstatus.bits.HPP = PRV_U;
            
        icpu_->writeCSR(CSR_mstatus, mstatus.value);
        return 4;
    }
};

/**
 * @brief MRET return from machine mode
 */
class MRET : public RiscvInstruction {
public:
    MRET(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "MRET", "00110000001000000000000001110011") {}

    virtual int exec(Reg64Type *payload) {
        if (icpu_->getPrvLevel() != PRV_M) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
            return 4;
        }
        csr_mstatus_type mstatus;
        mstatus.value = icpu_->readCSR(CSR_mstatus);

        uint64_t xepc = (PRV_M << 8) + 0x41;
        icpu_->setBranch(icpu_->readCSR(static_cast<uint32_t>(xepc)));

        mstatus.bits.MIE = mstatus.bits.MPIE;
        mstatus.bits.MPIE = 1;
        icpu_->setPrvLevel(mstatus.bits.MPP);
        mstatus.bits.MPP = PRV_U;

        icpu_->writeCSR(CSR_mstatus, mstatus.value);
        return 4;
    }
};


/** 
 * @brief FENCE (memory barrier)
 *
 * Not used in functional model so that cache is not modeling.
 */
class FENCE : public RiscvInstruction {
public:
    FENCE(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "FENCE", "?????????????????000?????0001111") {}

    virtual int exec(Reg64Type *payload) {
        return 4;
    }
};

/** 
 * @brief FENCE_I (memory barrier)
 *
 * Not used in functional model so that cache is not modeling.
 */
class FENCE_I : public RiscvInstruction {
public:
    FENCE_I(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "FENCE_I", "?????????????????001?????0001111") {}

    virtual int exec(Reg64Type *payload) {
        return 4;
    }
};

/**
 * @brief EBREAK (breakpoint instruction)
 *
 * The EBREAK instruction is used by debuggers to cause control to be
 * transferred back to a debug-ging environment.
 */
class EBREAK : public RiscvInstruction {
public:
    EBREAK(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "EBREAK", "00000000000100000000000001110011") {}

    virtual int exec(Reg64Type *payload) {
        icpu_->raiseSignal(EXCEPTION_Breakpoint);
        icpu_->doNotCache(icpu_->getPC());
        return 4;
    }
};

/**
 * @brief ECALL (environment call instruction)
 *
 * The ECALL instruction is used to make a request to the supporting execution
 * environment, which isusually an operating system. The ABI for the system
 * will define how parameters for the environment request are passed, but usually
 * these will be in defined locations in the integer register file.
 */
class ECALL : public RiscvInstruction {
public:
    ECALL(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ECALL", "00000000000000000000000001110011") {}

    virtual int exec(Reg64Type *payload) {
        switch (icpu_->getPrvLevel()) {
        case PRV_M:
            icpu_->raiseSignal(EXCEPTION_CallFromMmode);
            break;
        case PRV_U:
            icpu_->raiseSignal(EXCEPTION_CallFromUmode);
            break;
        default:;
        }
        return 4;
    }
};


void CpuRiver_Functional::addIsaPrivilegedRV64I() {
    addSupportedInstruction(new CSRRC(this));
    addSupportedInstruction(new CSRRCI(this));
    addSupportedInstruction(new CSRRS(this));
    addSupportedInstruction(new CSRRSI(this));
    addSupportedInstruction(new CSRRW(this));
    addSupportedInstruction(new CSRRWI(this));
    addSupportedInstruction(new URET(this));
    addSupportedInstruction(new SRET(this));
    addSupportedInstruction(new HRET(this));
    addSupportedInstruction(new MRET(this));
    addSupportedInstruction(new FENCE(this));
    addSupportedInstruction(new FENCE_I(this));
    addSupportedInstruction(new ECALL(this));
    addSupportedInstruction(new EBREAK(this));

    // TODO:
    /*
  def DRET               = BitPat("b01111011001000000000000001110011")
  def SFENCE_VMA         = BitPat("b0001001??????????000000001110011")
  def WFI                = BitPat("b00010000010100000000000001110011")  // wait for interrupt

    def RDCYCLE            = BitPat("b11000000000000000010?????1110011")
    def RDTIME             = BitPat("b11000000000100000010?????1110011")
    def RDINSTRET          = BitPat("b11000000001000000010?????1110011")
    def RDCYCLEH           = BitPat("b11001000000000000010?????1110011")
    def RDTIMEH            = BitPat("b11001000000100000010?????1110011")
    def RDINSTRETH         = BitPat("b11001000001000000010?????1110011")
    */

    /**
     * The 'U', 'S', and 'H' bits will be set if there is support for 
     * user, supervisor, and hypervisor privilege modes respectively.
     */
    uint64_t isa = portCSR_.read(CSR_misa).val;
    isa |= (1LL << ('U' - 'A'));
    isa |= (1LL << ('S' - 'A'));
    isa |= (1LL << ('H' - 'A'));
    portCSR_.write(CSR_misa, isa);
}

}  // namespace debugger
