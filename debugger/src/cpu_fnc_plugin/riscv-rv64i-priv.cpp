/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base ISA implementation (extension I, privileged level).
 */

#include "riscv-isa.h"
#include "api_utils.h"

namespace debugger {

uint64_t readCSR(uint32_t idx, CpuContextType *data) {
    uint64_t ret = data->csr[idx];
    switch (idx) {
    case CSR_mtime:
        ret = data->step_cnt;
        break;
    case CSR_mreset:
        ret = data->reset;
        break;
    default:;
    }
    return ret;
}

void writeCSR(uint32_t idx, uint64_t val, CpuContextType *data) {
    switch (idx) {
    // Read-Only registers
    case CSR_misa:
    case CSR_mvendorid:
    case CSR_marchid:
    case CSR_mimplementationid:
    case CSR_mhartid:
        break;
    case CSR_mtime:
        break;
    case CSR_mreset:
        data->reset = val != 0 ? true: false;
        break;
    default:
        data->csr[idx] = val;
    }
}

/** 
 * @brief The CSRRC (Atomic Read and Clear Bit in CSR).
 *
 * Instruction reads the value of the CSR, zeroextends the value to XLEN bits,
 * and writes it to integer register rd. The initial value in integer
 * register rs1 specifies bit positions to be cleared in the CSR. Any bit that
 * is high in rs1 will cause the corresponding bit to be cleared in the CSR,
 * if that CSR bit is writable. Other bits in the CSR are unaffected.
 */
class CSRRC : public IsaProcessor {
public:
    CSRRC() : IsaProcessor("CSRRC", "?????????????????011?????1110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];

        uint64_t clr_mask = ~data->regs[u.bits.rs1];
        uint64_t csr = readCSR(u.bits.imm, data);
        if (u.bits.rd) {
            data->regs[u.bits.rd] = csr;
        }
        writeCSR(u.bits.imm, (csr & clr_mask), data);
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief The CSRRCI (Atomic Read and Clear Bit in CSR immediate).
 *
 * Similar to CSRRC except it updates the CSR using a 5-bit zero-extended 
 * immediate (zimm[4:0]) encoded in the rs1 field instead of a value from  * an integer register. */
class CSRRCI : public IsaProcessor {
public:
    CSRRCI() : IsaProcessor("CSRRCI", "?????????????????111?????1110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];

        uint64_t clr_mask = ~static_cast<uint64_t>((u.bits.rs1));
        uint64_t csr = readCSR(u.bits.imm, data);
        if (u.bits.rd) {
            data->regs[u.bits.rd] = csr;
        }
        writeCSR(u.bits.imm, (csr & clr_mask), data);
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The CSRRS (Atomic Read and Set Bit in CSR).
 *
 *   Instruction reads the value of the CSR, zero-extends the value to XLEN 
 * bits, and writes it to integer register rd. The initial value in integer 
 * register rs1 specifies bit positions to be set in the CSR. Any bit that is
 * high in rs1 will cause the corresponding bit to be set in the CSR, if that
 * CSR bit is writable. Other bits in the CSR are unaffected (though CSRs  * might have side effects when written). *   The CSRR pseudo instruction (read CSR), when rs1 = 0. */
class CSRRS : public IsaProcessor {
public:
    CSRRS() : IsaProcessor("CSRRS", "?????????????????010?????1110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];

        uint64_t set_mask = data->regs[u.bits.rs1];
        uint64_t csr = readCSR(u.bits.imm, data);
        if (u.bits.rd) {
            data->regs[u.bits.rd] = csr;
        }
        writeCSR(u.bits.imm, (csr | set_mask), data);
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The CSRRSI (Atomic Read and Set Bit in CSR immediate).
 *
 * Similar to CSRRS except it updates the CSR using a 5-bit zero-extended 
 * immediate (zimm[4:0]) encoded in the rs1 field instead of a value from  * an integer register. */
class CSRRSI : public IsaProcessor {
public:
    CSRRSI() : IsaProcessor("CSRRSI", "?????????????????110?????1110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];

        uint64_t set_mask = u.bits.rs1;
        uint64_t csr = readCSR(u.bits.imm, data);
        if (u.bits.rd) {
            data->regs[u.bits.rd] = csr;
        }
        writeCSR(u.bits.imm, (csr | set_mask), data);
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief The CSRRW (Atomic Read/Write CSR).
 *
 *   Instruction atomically swaps values in the CSRs and integer registers. 
 * CSRRW reads the old value of the CSR, zero-extends the value to XLEN bits,
 * then writes it to integer register rd. The initial value in rs1 is written * to the CSR. *   The CSRW pseudo instruction (write CSR), when rs1 = 0. */
class CSRRW : public IsaProcessor {
public:
    CSRRW() : IsaProcessor("CSRRW", "?????????????????001?????1110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];

        uint64_t wr_value = data->regs[u.bits.rs1];
        if (u.bits.rd) {
            data->regs[u.bits.rd] = readCSR(u.bits.imm, data);
        }
        writeCSR(u.bits.imm, wr_value, data);
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief The CSRRWI (Atomic Read/Write CSR immediate).
 *
 * Similar to CSRRW except it updates the CSR using a 5-bit zero-extended 
 * immediate (zimm[4:0]) encoded in the rs1 field instead of a value from  * an integer register. */
class CSRRWI : public IsaProcessor {
public:
    CSRRWI() : IsaProcessor("CSRRWI", "?????????????????101?????1110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];

        uint64_t wr_value = u.bits.rs1;
        if (u.bits.rd) {
            data->regs[u.bits.rd] = readCSR(u.bits.imm, data);
        }
        writeCSR(u.bits.imm, wr_value, data);
        data->npc = data->pc + 4;
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
class URET : public IsaProcessor {
public:
    URET() : IsaProcessor("URET", "00000000001000000000000001110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        csr_mstatus_type mstatus;
        mstatus.value = readCSR(CSR_mstatus, data);

        uint64_t xepc = (PRV_LEVEL_U << 8) + 0x41;
        data->npc = readCSR(static_cast<uint32_t>(xepc), data);

        bool is_N_extension = false;
        if (is_N_extension) {
            mstatus.bits.UIE = mstatus.bits.UPIE;
            mstatus.bits.UPIE = 1;
            // User mode not changed.
        } else {
            mstatus.bits.UIE = 0;
            mstatus.bits.UPIE = 0;
        }
        data->cur_prv_level = PRV_LEVEL_U;
        writeCSR(CSR_mstatus, mstatus.value, data);
    }
};

/**
 * @brief MRET return from super-user mode
 */
class SRET : public IsaProcessor {
public:
    SRET() : IsaProcessor("SRET", "00010000001000000000000001110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        csr_mstatus_type mstatus;
        mstatus.value = readCSR(CSR_mstatus, data);

        uint64_t xepc = (PRV_LEVEL_S << 8) + 0x41;
        data->npc = readCSR(static_cast<uint32_t>(xepc), data);

        mstatus.bits.SIE = mstatus.bits.SPIE;
        mstatus.bits.SPIE = 1;
        data->cur_prv_level = mstatus.bits.SPP;
        mstatus.bits.SPP = PRV_LEVEL_U;
            
        writeCSR(CSR_mstatus, mstatus.value, data);
    }
};

/**
 * @brief MRET return from hypervisor mode
 */
class HRET : public IsaProcessor {
public:
    HRET() : IsaProcessor("HRET", "00100000001000000000000001110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        csr_mstatus_type mstatus;
        mstatus.value = readCSR(CSR_mstatus, data);

        uint64_t xepc = (PRV_LEVEL_H << 8) + 0x41;
        data->npc = readCSR(static_cast<uint32_t>(xepc), data);

        mstatus.bits.HIE = mstatus.bits.HPIE;
        mstatus.bits.HPIE = 1;
        data->cur_prv_level = mstatus.bits.HPP;
        mstatus.bits.HPP = PRV_LEVEL_U;
            
        writeCSR(CSR_mstatus, mstatus.value, data);
    }
};

/**
 * @brief MRET return from machine mode
 */
class MRET : public IsaProcessor {
public:
    MRET() : IsaProcessor("MRET", "00110000001000000000000001110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        csr_mstatus_type mstatus;
        mstatus.value = readCSR(CSR_mstatus, data);

        uint64_t xepc = (PRV_LEVEL_M << 8) + 0x41;
        data->npc = readCSR(static_cast<uint32_t>(xepc), data);

        mstatus.bits.MIE = mstatus.bits.MPIE;
        mstatus.bits.MPIE = 1;
        data->cur_prv_level = mstatus.bits.MPP;
        mstatus.bits.MPP = PRV_LEVEL_U;
            
        writeCSR(CSR_mstatus, mstatus.value, data);
    }
};


/** 
 * @brief FENCE (memory barrier)
 *
 * Not used in functional model so that cache is not modeling.
 */
class FENCE : public IsaProcessor {
public:
    FENCE() : IsaProcessor("FENCE", "?????????????????000?????0001111") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief FENCE_I (memory barrier)
 *
 * Not used in functional model so that cache is not modeling.
 */
class FENCE_I : public IsaProcessor {
public:
    FENCE_I() : IsaProcessor("FENCE_I", "?????????????????001?????0001111") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        data->npc = data->pc + 4;
    }
};


void addIsaPrivilegedRV64I(CpuContextType *data, AttributeType *out) {
    addSupportedInstruction(new CSRRC, out);
    addSupportedInstruction(new CSRRCI, out);
    addSupportedInstruction(new CSRRS, out);
    addSupportedInstruction(new CSRRSI, out);
    addSupportedInstruction(new CSRRW, out);
    addSupportedInstruction(new CSRRWI, out);
    addSupportedInstruction(new URET, out);
    addSupportedInstruction(new SRET, out);
    addSupportedInstruction(new HRET, out);
    addSupportedInstruction(new MRET, out);
    addSupportedInstruction(new FENCE, out);
    addSupportedInstruction(new FENCE_I, out);
    // TODO:
    /*
  def URET               = BitPat("b00000000001000000000000001110011")
  def SRET               = BitPat("b00010000001000000000000001110011")
  def HRET               = BitPat("b00100000001000000000000001110011")
  def MRET               = BitPat("b00110000001000000000000001110011")
  def DRET               = BitPat("b01111011001000000000000001110011")
  def SFENCE_VM          = BitPat("b000100000100?????000000001110011")
  def WFI                = BitPat("b00010000010100000000000001110011")  // wait for interrupt

    addInstr("SCALL",              "00000000000000000000000001110011", NULL, out);
    addInstr("SBREAK",             "00000000000100000000000001110011", NULL, out);
    addInstr("SRET",               "10000000000000000000000001110011", NULL, out);
    def RDCYCLE            = BitPat("b11000000000000000010?????1110011")
    def RDTIME             = BitPat("b11000000000100000010?????1110011")
    def RDINSTRET          = BitPat("b11000000001000000010?????1110011")
    def RDCYCLEH           = BitPat("b11001000000000000010?????1110011")
    def RDTIMEH            = BitPat("b11001000000100000010?????1110011")
    def RDINSTRETH         = BitPat("b11001000001000000010?????1110011")
    def ECALL              = BitPat("b00000000000000000000000001110011")
    def EBREAK             = BitPat("b00000000000100000000000001110011")
    */

    /**
     * The 'U', 'S', and 'H' bits will be set if there is support for 
     * user, supervisor, and hypervisor privilege modes respectively.
     */
    data->csr[CSR_misa] |= (1LL << ('U' - 'A'));
    data->csr[CSR_misa] |= (1LL << ('S' - 'A'));
    data->csr[CSR_misa] |= (1LL << ('H' - 'A'));
}

}  // namespace debugger
