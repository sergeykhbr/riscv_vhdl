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
 * @brief      RISC-V extension-M.
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

/**
 * @brief The DIV signed division
 */
class DIV : public RiscvInstruction {
 public:
    DIV(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "DIV", "0000001??????????100?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        if (R[u.bits.rs2]) {
            R[u.bits.rd] = static_cast<int64_t>(R[u.bits.rs1])
                 / static_cast<int64_t>(R[u.bits.rs2]);
        } else {
            R[u.bits.rd] = 0;
        }
        return 4;
    }
};

/**
 * @brief DIVU unsigned division
 */
class DIVU : public RiscvInstruction {
 public:
    DIVU(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "DIVU", "0000001??????????101?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        if (R[u.bits.rs2]) {
            R[u.bits.rd] = R[u.bits.rs1] / R[u.bits.rs2];
        } else {
            R[u.bits.rd] = 0;
        }
        return 4;
    }
};

/**
 * @brief DIVUW 32-bits unsigned division (RV64I)
 */
class DIVUW : public RiscvInstruction {
 public:
    DIVUW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "DIVUW", "0000001??????????101?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        if (static_cast<uint32_t>(R[u.bits.rs2])) {
            R[u.bits.rd] = 
                static_cast<uint32_t>(R[u.bits.rs1]) / 
                static_cast<uint32_t>(R[u.bits.rs2]);
        } else {
            R[u.bits.rd] = 0;
        }
        return 4;
    }
};

/**
 * @brief DIVW 32-bits signed division (RV64I)
 */
class DIVW : public RiscvInstruction {
 public:
    DIVW(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "DIVW", "0000001??????????100?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        int32_t divident = static_cast<int32_t>(R[u.bits.rs1]);
        int32_t divisor = static_cast<int32_t>(R[u.bits.rs2]);
        if (divisor) {
            R[u.bits.rd] = static_cast<int64_t>(divident / divisor);
        } else {
            R[u.bits.rd] = 0;
        }
        return 4;
    }
};

/**
 * @brief The MUL signed multiplication
 *
 * MUL performs an XLEN-bit XLEN-bit multiplication and places the lower XLEN 
 * bits in the destination register.
 */
class MUL : public RiscvInstruction {
 public:
    MUL(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "MUL", "0000001??????????000?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        R[u.bits.rd] = static_cast<int64_t>(R[u.bits.rs1])
                * static_cast<int64_t>(R[u.bits.rs2]);
        return 4;
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
class MULW : public RiscvInstruction {
 public:
    MULW(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "MULW", "0000001??????????000?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        int32_t m1 = static_cast<int32_t>(R[u.bits.rs1]);
        int32_t m2 = static_cast<int32_t>(R[u.bits.rs2]);

        R[u.bits.rd] = static_cast<int64_t>(m1 * m2);
        if (R[u.bits.rd] & (1LL << 31)) {
            R[u.bits.rd] |= EXT_SIGN_32;
        }
        return 4;
    }
};

/**
 * @brief The REM (remainder of the corresponding signed division operation)
 */
class REM : public RiscvInstruction {
public:
    REM(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "REM", "0000001??????????110?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        R[u.bits.rd] = static_cast<int64_t>(R[u.bits.rs1])
             % static_cast<int64_t>(R[u.bits.rs2]);
        return 4;
    }
};

/**
 * @brief The REMU (remainder of the corresponding unsgined division operation)
 */
class REMU : public RiscvInstruction {
public:
    REMU(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "REMU", "0000001??????????111?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        R[u.bits.rd] = R[u.bits.rs1] % R[u.bits.rs2];
        return 4;
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
class REMW : public RiscvInstruction {
public:
    REMW(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "REMW", "0000001??????????110?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        int32_t tmp;
        u.value = payload->buf32[0];
        tmp = static_cast<int32_t>(R[u.bits.rs1])
            % static_cast<int32_t>(R[u.bits.rs2]);
        R[u.bits.rd] = static_cast<uint64_t>(static_cast<int64_t>(tmp));
        return 4;
    }
};

class REMUW : public RiscvInstruction {
public:
    REMUW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "REMUW", "0000001??????????111?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        uint32_t tmp;
        u.value = payload->buf32[0];
        tmp = static_cast<uint32_t>(R[u.bits.rs1])
            % static_cast<uint32_t>(R[u.bits.rs2]);
        R[u.bits.rd] = static_cast<uint64_t>(static_cast<int64_t>(tmp));
        return 4;
    }
};

void CpuRiver_Functional::addIsaExtensionM() {
    addSupportedInstruction(new DIV(this));
    addSupportedInstruction(new DIVU(this));
    addSupportedInstruction(new DIVUW(this));
    addSupportedInstruction(new DIVW(this));
    addSupportedInstruction(new MUL(this));
    addSupportedInstruction(new MULW(this));
    addSupportedInstruction(new REM(this));
    addSupportedInstruction(new REMU(this));
    addSupportedInstruction(new REMW(this));
    addSupportedInstruction(new REMUW(this));

    // TODO
    /*
    addInstr("MULH", "0000001??????????001?????0110011", NULL, out);
    addInstr("MULHSU", "0000001??????????010?????0110011", NULL, out);
    addInstr("MULHU", "0000001??????????011?????0110011", NULL, out);
    */

    uint64_t isa = portCSR_.read(CSR_misa).val;
    portCSR_.write(CSR_misa, isa | (1LL << ('M' - 'A')));
}

}  // namespace debugger
