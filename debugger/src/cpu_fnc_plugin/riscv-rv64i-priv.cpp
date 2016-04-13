/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base ISA implementation (extension I, privileged level).
 */

#include "riscv-isa.h"
#include "api_utils.h"

namespace debugger {

uint64_t readCSR(uint32_t idx, CpuDataType *data) {
    uint64_t ret = data->csr[idx];
    switch (idx) {
    case CSR_mtime:
        ret = data->step_cnt;
        break;
    default:;
    }
    return ret;
}

void writeCSR(uint32_t idx, uint64_t val, CpuDataType *data) {
    switch (idx) {
    // Read-Only registers
    case CSR_mcpuid:
    case CSR_mimpid:
    case CSR_mheartid:
        break;
    case CSR_mtime:
        break;
    case CSR_send_ipi:
        generateInterrupt(IRQ_Software, data);
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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
 * @brief ERET (Environment Return)
 *
 * After handling a trap, the ERET instruction is used to return to the 
 * privilege level at which the trap occurred. In addition to manipulating 
 * the privilege stack as described in Section 3.1.5, ERET sets the pc to 
 * the value stored in the Xepc register, where X is the privilege mode 
 * (S, H, or M) in which the ERET instruction was executed. */
class ERET : public IsaProcessor {
public:
    ERET() : IsaProcessor("ERET", "00010000000000000000000001110011") {}

    virtual void exec(uint32_t *payload, CpuDataType *data) {
        data->prv_stack_cnt++;
        data->npc = readCSR(CSR_mepc, data);
        if (data->prv_stack_cnt == 2) {
            data->npc = 0x100;
        }
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
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

    virtual void exec(uint32_t *payload, CpuDataType *data) {
        data->npc = data->pc + 4;
    }
};


void addIsaPrivilegedRV64I(CpuDataType *data, AttributeType *out) {
    addSupportedInstruction(new CSRRC, out);
    addSupportedInstruction(new CSRRCI, out);
    addSupportedInstruction(new CSRRS, out);
    addSupportedInstruction(new CSRRSI, out);
    addSupportedInstruction(new CSRRW, out);
    addSupportedInstruction(new CSRRWI, out);
    addSupportedInstruction(new ERET, out);
    addSupportedInstruction(new FENCE, out);
    addSupportedInstruction(new FENCE_I, out);
    // TODO:
    /*
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

    data->csr[CSR_mimpid]   = 0x0001;       // UC Berkeley Rocket repo
    data->csr[CSR_mheartid] = 0;
    data->csr[CSR_mtvec]   = 0x100;         // Hardwired RO value
}

}  // namespace debugger
