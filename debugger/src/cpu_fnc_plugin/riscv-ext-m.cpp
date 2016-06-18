/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RISC-V extension-M.
 */

#include "riscv-isa.h"
#include "api_utils.h"

namespace debugger {

/**
 * @brief The DIV signed division
 */
class DIV : public IsaProcessor {
public:
    DIV() : IsaProcessor("DIV", "0000001??????????100?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        if (data->regs[u.bits.rs2]) {
            data->regs[u.bits.rd] = static_cast<int64_t>(data->regs[u.bits.rs1])
                 / static_cast<int64_t>(data->regs[u.bits.rs2]);
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief DIVU unsigned division
 */
class DIVU : public IsaProcessor {
public:
    DIVU() : IsaProcessor("DIVU", "0000001??????????101?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        if (data->regs[u.bits.rs2]) {
            data->regs[u.bits.rd] = 
                data->regs[u.bits.rs1] / data->regs[u.bits.rs2];
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief DIVUW 32-bits unsigned division (RV64I)
 */
class DIVUW : public IsaProcessor {
public:
    DIVUW() : IsaProcessor("DIVUW", "0000001??????????101?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        if (static_cast<uint32_t>(data->regs[u.bits.rs2])) {
            data->regs[u.bits.rd] = 
                static_cast<uint32_t>(data->regs[u.bits.rs1]) / 
                static_cast<uint32_t>(data->regs[u.bits.rs2]);
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief DIVW 32-bits signed division (RV64I)
 */
class DIVW : public IsaProcessor {
public:
    DIVW() : IsaProcessor("DIVW", "0000001??????????100?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        int32_t divident = static_cast<int32_t>(data->regs[u.bits.rs1]);
        int32_t divisor = static_cast<int32_t>(data->regs[u.bits.rs2]);
        if (divisor) {
            data->regs[u.bits.rd] = static_cast<int64_t>(divident / divisor);
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The MUL signed multiplication
 *
 * MUL performs an XLEN-bit XLEN-bit multiplication and places the lower XLEN 
 * bits in the destination register.
 */
class MUL : public IsaProcessor {
public:
    MUL() : IsaProcessor("MUL", "0000001??????????000?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = static_cast<int64_t>(data->regs[u.bits.rs1])
                * static_cast<int64_t>(data->regs[u.bits.rs2]);
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The MULW 32-bits signed multiplication (RV64I)
 *
 * MULW is only valid for RV64, and multiplies the lower 32 bits of the source
 * registers, placing the sign-extension of the lower 32 bits of the result
 * into the destination register. MUL can be used to obtain the upper 32 bits
 * of the 64-bit product, but signed arguments must be proper 32-bit signed
 * values, whereas unsigned arguments must have their upper 32 bits clear.
 */
class MULW : public IsaProcessor {
public:
    MULW() : IsaProcessor("MULW", "0000001??????????000?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        int32_t m1 = static_cast<int32_t>(data->regs[u.bits.rs1]);
        int32_t m2 = static_cast<int32_t>(data->regs[u.bits.rs2]);

        data->regs[u.bits.rd] = static_cast<int64_t>(m1 * m2);
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The REM (remainder of the corresponding signed division operation)
 */
class REM : public IsaProcessor {
public:
    REM() : IsaProcessor("REM", "0000001??????????110?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = static_cast<int64_t>(data->regs[u.bits.rs1])
             % static_cast<int64_t>(data->regs[u.bits.rs2]);
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The REMU (remainder of the corresponding unsgined division operation)
 */
class REMU : public IsaProcessor {
public:
    REMU() : IsaProcessor("REMU", "0000001??????????111?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
            data->regs[u.bits.rs1] % data->regs[u.bits.rs2];
        data->npc = data->pc + 4;
    }
};

/**
 * @brief REMW signed reminder operation
 * 
 * REMW and REMUW instructions are only valid
 * for RV64, and provide the corresponding signed and unsigned remainder 
 * operations respectively.
 * Both REMW and REMUW sign-extend the 32-bit result to 64 bits.
 */
class REMW : public IsaProcessor {
public:
    REMW() : IsaProcessor("REMW", "0000001??????????110?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        int32_t tmp;
        u.value = payload[0];
        tmp = static_cast<int32_t>(data->regs[u.bits.rs1])
            % static_cast<int32_t>(data->regs[u.bits.rs2]);
        data->regs[u.bits.rd] = 
            static_cast<uint64_t>(static_cast<int64_t>(tmp));
        data->npc = data->pc + 4;
    }
};

class REMUW : public IsaProcessor {
public:
    REMUW() : IsaProcessor("REMUW", "0000001??????????111?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        uint32_t tmp;
        u.value = payload[0];
        tmp = static_cast<uint32_t>(data->regs[u.bits.rs1])
            % static_cast<uint32_t>(data->regs[u.bits.rs2]);
        data->regs[u.bits.rd] = 
            static_cast<uint64_t>(static_cast<int64_t>(tmp));
        data->npc = data->pc + 4;
    }
};

void addIsaExtensionM(CpuContextType *data, AttributeType *out) {
    addSupportedInstruction(new DIV, out);
    addSupportedInstruction(new DIVU, out);
    addSupportedInstruction(new DIVUW, out);
    addSupportedInstruction(new DIVW, out);
    addSupportedInstruction(new MUL, out);
    addSupportedInstruction(new MULW, out);
    addSupportedInstruction(new REM, out);
    addSupportedInstruction(new REMU, out);
    addSupportedInstruction(new REMW, out);
    addSupportedInstruction(new REMUW, out);
    // TODO
    /*
    addInstr("MULH",               "0000001??????????001?????0110011", NULL, out);
    addInstr("MULHSU",             "0000001??????????010?????0110011", NULL, out);
    addInstr("MULHU",              "0000001??????????011?????0110011", NULL, out);
    */
    data->csr[CSR_mcpuid] |= (1LL << ('M' - 'A'));
}

}  // namespace debugger
