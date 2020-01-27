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

#include "api_core.h"
#include "arm-isa.h"
#include "cpu_arm7_func.h"

namespace debugger {

/** 4.6.1 ADC (immediate) */
class ADC_I_T1 : public T1Instruction {
 public:
    ADC_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADC_I_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t i = (ti >> 10) & 1;
        uint32_t setflags = (ti >> 4) & 1;
        uint32_t n = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC((i << 11) | (imm3 << 8) | imm8, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, imm32, icpu_->getC(), &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.3 ADD (immediate) */
class ADD_I_T1 : public T1Instruction {
 public:
    ADD_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = (ti >> 6) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        result = AddWithCarry(Rn, imm32, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class ADD_I_T2 : public T1Instruction {
 public:
    ADD_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        bool setflags = !icpu_->InITBlock();
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t Rn = static_cast<uint32_t>(R[dn]);

        result = AddWithCarry(Rn, imm32, 0, &overflow, &carry);
        icpu_->setReg(dn, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class ADD_I_T3 : public T1Instruction {
 public:
    ADD_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, imm32, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.4 ADD (register) */
class ADD_R_T1 : public T1Instruction {
 public:
    ADD_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;

        result = AddWithCarry(Rn, Rm, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class ADD_R_T2 : public T1Instruction {
 public:
    ADD_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t DN = (ti >> 7) & 0x1;
        uint32_t dn = (DN << 3) | (ti & 0x7);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;

        result = AddWithCarry(Rn, Rm, 0, &overflow, &carry);
        icpu_->setReg(dn, result);
        if (dn == Reg_pc) {
            ALUWritePC(result);
        }
        return 2;
    }
};

class ADD_R_T3 : public T1Instruction {
 public:
    ADD_R_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        uint32_t shift_n;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t shifted;

        SRType shift_t = DecodeImmShift(type, (imm3 << 2) | imm2, &shift_n);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        shifted = Shift_C(Rm, shift_t, shift_n, icpu_->getC(), &carry);
        result = AddWithCarry(Rn, shifted, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (d == Reg_pc) {
            ALUWritePC(result);     // setflags always FALSE
        } else {
            if (setflags) {
                // Mask 0x7 no need to check on SP or PC
                icpu_->setN((result >> 31) & 1);
                icpu_->setZ(result == 0 ? 1: 0);
                icpu_->setC(carry);
                icpu_->setV(overflow);
            }
        }
        return 4;
    }
};

/** 4.6.5 ADD (SP plus immediate) */
class ADDSP_I_T1 : public T1Instruction {
 public:
    ADDSP_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = (ti & 0xFF) << 2;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t SP = static_cast<uint32_t>(R[Reg_sp]);

        result = AddWithCarry(SP, imm32, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        return 2;
    }
};

class ADDSP_I_T2 : public T1Instruction {
 public:
    ADDSP_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t imm32 = (ti & 0x7F) << 2;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t SP = static_cast<uint32_t>(R[Reg_sp]);

        result = AddWithCarry(SP, imm32, 0, &overflow, &carry);
        icpu_->setReg(Reg_sp, result);
        return 2;
    }
};

/** 4.6.7 ADR */
class ADR_T1 : public T1Instruction {
 public:
    ADR_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = (ti & 0xFF) << 2;
        uint32_t pc = static_cast<uint32_t>(icpu_->getPC()) + 4;

        pc &= ~0x3;             // Word-aligned PC
        icpu_->setReg(d, pc + imm32);
        return 2;
    }
};

/** 4.6.8 AND (immediate) */
class AND_I_T1 : public T1Instruction {
 public:
    AND_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "AND.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn & imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.9 AND (register) */
class AND_R_T1 : public T1Instruction {
 public:
    AND_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ANDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t result = Rn & Rm;

        icpu_->setReg(dn, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            // C the same because no shoft
            // V unchanged
        }
        return 2;
    }
};

class AND_R_T2 : public T1Instruction {
 public:
    AND_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "AND.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        uint32_t shift_n;
        SRType shift_t = DecodeImmShift(type, (imm3 << 2) | imm2, &shift_n);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t shifted;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        shifted = Shift_C(Rm, shift_t, shift_n, icpu_->getC(), &carry);
        result = Rn & shifted;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.10 ASR (immediate) */
class ASR_I_T1 : public T1Instruction {
 public:
    ASR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ASR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t result;
        uint32_t d = ti  & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm5 = (ti >> 6) & 0x1F;
        uint32_t shift_n;
        uint32_t carry;
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        DecodeImmShift(2, imm5, &shift_n);
        result = Shift_C(Rm, SRType_ASR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.12 B */
class B_T1 : public T1Instruction {
 public:
    B_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t cond = (ti >> 8) & 0xF;
        uint32_t imm8 = ti & 0xFF;
        uint32_t imm32 = imm8 << 1;
        if (ti & 0x80) {
            imm32 |= (~0ul) << 9;
        }

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (check_cond(icpu_, cond)) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 2;
    }
};

class B_T2 : public T1Instruction {
 public:
    B_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t imm11 = ti & 0x7FF;
        uint32_t imm32 = imm11 << 1;
        if (ti & 0x400) {
            imm32 |= (~0ul) << 12;
        }

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
        BranchWritePC(npc);
        return 2;
    }
};

class B_T3 : public T1Instruction {
 public:
    B_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t cond = (ti >> 6) & 0xF;
        uint32_t imm6 = ti & 0x3F;
        uint32_t imm11 = ti1 & 0x7FF;
        uint32_t S = (ti >> 10) & 1;
        uint32_t J1 = (ti1 >> 13) & 1;
        uint32_t J2 = (ti1 >> 11) & 1;
        uint32_t imm32 = (J2 << 19) | (J1 << 18) | (imm6 << 12) | (imm11 << 1);
        if (S) {
            imm32 |= (~0ul) << 20;
        }

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (check_cond(icpu_, cond)) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 4;
    }
};

class B_T4 : public T1Instruction {
 public:
    B_T4(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t S = (ti >> 10) & 1;
        uint32_t imm10 = ti & 0x3FF;
        uint32_t J1 = (ti1 >> 13) & 1;
        uint32_t J2 = (ti1 >> 11) & 1;
        uint32_t imm11 = ti1 & 0x7FF;
        uint32_t I1 = (!(J1 ^ S)) & 1;
        uint32_t I2 = (!(J2 ^ S)) & 1;
        uint32_t imm32 = (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
        if (S) {
            imm32 |= (~0ul) << 24;
        }

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
        BranchWritePC(npc);
        return 4;
    }
};

/** 4.6.15 BIC (immediate) */
class BIC_I_T1 : public T1Instruction {
 public:
    BIC_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BIC.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti0 = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti0  & 0xF;
        bool setflags = (ti0 >> 4) & 1;
        uint32_t i = (ti0 >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn & ~imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.17 BKPT */
class BKPT_T1 : public T1Instruction {
 public:
    BKPT_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BKPT") {}

    virtual int exec(Reg64Type *payload) {
        icpu_->raiseSignal(Interrupt_SoftwareIdx);
        icpu_->doNotCache(0);
        return 2;
    }
};

/** 4.6.18 BL, BLX (immediate) */
class BL_I_T1 : public T1Instruction {
 public:
    BL_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BL_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t instr0 = payload->buf16[0];
        uint32_t instr1 = payload->buf16[1];
        uint32_t S = (instr0 >> 10) & 1;
        uint32_t I1 = (((instr1 >> 13) & 0x1) ^ S) ^ 1;
        uint32_t I2 = (((instr1 >> 11) & 0x1) ^ S) ^ 1;
        uint32_t imm11 = instr1 & 0x7FF;
        uint32_t imm10 = instr0 & 0x3FF;
        uint32_t imm32;
        imm32 = (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
        if (S) {
            imm32 |= (~0ul) << 24;
        }

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t pc = static_cast<uint32_t>(icpu_->getPC()) + 4;
        uint32_t npc = pc + imm32;
        icpu_->setInstrMode(THUMB_mode);
        icpu_->setReg(Reg_lr, pc | 0x1);
        BranchWritePC(npc);
        return 4;
    }
};

/** 4.6.19 BLX (register) */
class BLX_R_T1 : public T1Instruction {
 public:
    BLX_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BLX") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint64_t next_instr_addr = icpu_->getPC() + 4 - 2;

        if (m == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        icpu_->setReg(Reg_lr, next_instr_addr | 0x1);
        BXWritePC(Rm);
        return 2;
    }
};

/** 4.6.20 BX */
class BX_T1 : public T1Instruction {
 public:
    BX_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BX") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr0 = payload->buf16[0];
        uint32_t m = (instr0 >> 3) & 0xF;

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t Rm = static_cast<uint32_t>(R[m]);
        BXWritePC(Rm);
        return 2;
    }
};

/** 4.6.22 CBNZ. Compare and Branch on Non-Zero */
class CBNZ_T1 : public T1Instruction {
 public:
    CBNZ_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CBNZ") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = ti & 0x7;
        uint32_t imm5 = (ti >> 3) & 0x1F;
        uint32_t i = (ti >> 9) & 0x1;
        uint32_t imm32 = (i << 6) | (imm5 << 1);

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (R[n] != 0) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 2;
    }
};

/** 4.6.23 CBZ. Compare and Branch on Zero */
class CBZ_T1 : public T1Instruction {
 public:
    CBZ_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CBZ") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = ti & 0x7;
        uint32_t imm5 = (ti >> 3) & 0x1F;
        uint32_t i = (ti >> 9) & 0x1;
        uint32_t imm32 = (i << 6) | (imm5 << 1);

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (R[n] == 0) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 2;
    }
};

/** 4.6.29 CMP (immediate) */
class CMP_I_T1 : public T1Instruction {
 public:
    CMP_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t n = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;

        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 2;
    }
};

class CMP_I_T2 : public T1Instruction {
 public:
    CMP_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t n = ti & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 4;
    }
};

/** 4.6.30 CMP (register) */
class CMP_R_T1 : public T1Instruction {
 public:
    CMP_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t n = ti  & 0x7;
        uint32_t m = (ti >> 3) & 0x7;

        uint32_t shifted = static_cast<uint32_t>(R[m]); // no shifting
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~shifted, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 2;
    }
};

class CMP_R_T2 : public T1Instruction {
 public:
    CMP_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t N = (ti >> 7) & 0x1;
        uint32_t n = (N << 3) | (ti  & 0x7);
        uint32_t m = (ti >> 3) & 0xF;

        if (n < 8 && m < 8) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t shifted = static_cast<uint32_t>(R[m]); // no shifting
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~shifted, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 2;
    }
};

/** 4.6.31 CPS (Change Processor State) */
class CPS_T1 : public T1Instruction {
 public:
    CPS_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CPS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t disable = (ti >> 4) & 1;
        uint32_t A = (ti >> 2) & 1;
        uint32_t I = (ti >> 1) & 1;
        uint32_t F = (ti >> 0) & 1;

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (!disable) {
            if (A) {
                icpu_->setA(0);
            }
            if (I) {
                icpu_->setI(0);
            }
            if (F) {
                icpu_->setF(0);
            }
        } else {
            if (A) {
                icpu_->setA(1);
            }
            if (I) {
                icpu_->setI(1);
            }
            if (F) {
                icpu_->setF(1);
            }
        }
        return 2;
    }
};

/** 4.6.36 EOR (immediate) */
class EOR_I_T1 : public T1Instruction {
 public:
    EOR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "EOR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn ^ imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.37 EOR (register) */
class EOR_R_T1 : public T1Instruction {
 public:
    EOR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "EORS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t result = Rn ^ Rm;

        icpu_->setReg(dn, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            // C the same because no shoft
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.39 IT (IT Block) */
class IT_T1 : public T1Instruction {
 public:
    IT_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "IT") {}

    virtual int exec(Reg64Type *payload) {
        uint32_t ti = payload->buf16[0];
        uint32_t firstcond = (ti >> 4) & 0xF;
        uint32_t mask = ti & 0xF;
        uint32_t BitCount = 0;
        for (int i = 0; i < 4; i++) {
            if (mask & (1 << i)) {
                BitCount++;
            }
        }

        if (firstcond == 0xF) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (firstcond == 0xE && BitCount != 1) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        icpu_->StartITBlock(firstcond, mask);
        return 2;
    }
};

/** 4.6.42 LDMIA Load Multiple Increment After Load */
class LDMIA_T2 : public T1Instruction {
 public:
    LDMIA_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDMIA.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t P = (ti1 >> 15) & 1;
        uint32_t M = (ti1 >> 14) & 1;
        uint32_t register_list = ti1 & 0xDFFF;
        uint32_t address = static_cast<uint32_t>(R[n]);
        uint32_t wback = (ti >> 5) & 1;

        if (n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (P == 1 && M == 1) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (P == 1 && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;

        for (int i = 0; i < Reg_pc; i++) {
            if (register_list & (1ul << i)) {
                trans_.addr = address;
                icpu_->dma_memop(&trans_);
                if (!(static_cast<unsigned>(i) == n && wback)) {
                    icpu_->setReg(i, trans_.rpayload.b32[0]);
                } else {
                    // R[i] set earlier to be bits[32] UNKNOWN
                }
                address += 4;
            }
        }
        if (ti & (1ul << Reg_pc)) {
            if (wback) {
                icpu_->setReg(n, address + 4);  // to support Exception Exit
            }

            LoadWritePC(address);
            address += 4;
        } else {
            if (wback) {
                icpu_->setReg(n, address);
            }
        }
        return 4;
    }
};

/** 4.6.43 LDR (immediate) */
class LDR_I_T1 : public T1Instruction {
 public:
    LDR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR_I_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t t = instr & 0x7;
        uint32_t n = (instr >> 3) & 0x7;
        uint32_t imm32 = ((instr >> 6) & 0x1F) << 2;
        uint32_t address = static_cast<uint32_t>(R[n]) + imm32;

        if (t == Reg_pc) {
            if (trans_.addr & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC(address);
            }
        } else {
            trans_.addr = address;
            trans_.action = MemAction_Read;
            trans_.xsize = 4;
            trans_.wstrb = 0;
            icpu_->dma_memop(&trans_);
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }

        return 2;
    }
};

class LDR_I_T2 : public T1Instruction {
 public:
    LDR_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR_I_T2") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = (ti >> 8) & 0x7;
        uint32_t Rn = static_cast<uint32_t>(R[Reg_sp]);
        uint32_t imm32 = (ti & 0xFF) << 2;

        trans_.addr = Rn + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        icpu_->setReg(t, trans_.rpayload.b32[0]);
        return 2;
    }
};

class LDR_I_T3 : public T1Instruction {
 public:
    LDR_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xf;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        uint32_t address = static_cast<uint32_t>(R[n]) + imm32;

        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (t == Reg_pc) {
            if (trans_.addr & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC(address);
            }
        } else {
            trans_.addr = address;
            trans_.action = MemAction_Read;
            trans_.xsize = 4;
            trans_.wstrb = 0;
            icpu_->dma_memop(&trans_);
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }
        return 4;
    }
};

class LDR_I_T4 : public T1Instruction {
 public:
    LDR_I_T4(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xf;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFF;
        bool index = (ti1 >> 10) & 1;
        bool add = (ti1 >> 9) & 1;
        bool wback = (ti1 >> 8) & 1;
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (wback && n == t) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t offset_addr = add ? Rn + imm32 : Rn - imm32;
        uint32_t address = index ? offset_addr : Rn;
        if (wback) {
            icpu_->setReg(n, offset_addr);
        }

        if (t == Reg_pc) {
            if (trans_.addr & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC(address);
            }
        } else {
            trans_.action = MemAction_Read;
            trans_.addr = address;
            trans_.xsize = 4;
            trans_.wstrb = 0;
            icpu_->dma_memop(&trans_);
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }
        return 4;
    }
};

/** 4.6.44 LDR (literal) */
class LDR_L_T1 : public T1Instruction {
 public:
    LDR_L_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = (ti >> 8) & 0x7;
        uint32_t imm8 = (ti & 0xFF) << 2;
        uint32_t base = static_cast<uint32_t>(icpu_->getPC()) + 4;
        base &= ~0x3;  // Align(base, 4)
        uint32_t address = base + imm8;

        if (t == Reg_pc) {
            if (address & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC(address);
            }
        } else {
            trans_.addr = address;
            trans_.action = MemAction_Read;
            trans_.xsize = 4;
            trans_.wstrb = 0;
            icpu_->dma_memop(&trans_);
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }

        return 2;
    }
};

class LDR_L_T2 : public T1Instruction {
 public:
    LDR_L_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        uint32_t base = static_cast<uint32_t>(icpu_->getPC()) + 4;
        base &= ~0x3;  // Align(base, 4)
        bool add = (ti >> 7) & 1;
        uint32_t address;

        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        address = add ? base + imm32 : base - imm32;

        if (t == Reg_pc) {
            if (address & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC(address);
            }
        } else {
            trans_.addr = address;
            trans_.action = MemAction_Read;
            trans_.xsize = 4;
            trans_.wstrb = 0;
            icpu_->dma_memop(&trans_);
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }

        return 4;
    }
};

/** 4.6.45 LDR (register) */
class LDR_R_T1 : public T1Instruction {
 public:
    LDR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t t = instr & 0x7;
        uint32_t n = (instr >> 3) & 0x7;
        uint32_t m = (instr >> 6) & 0x7;
        uint32_t address = static_cast<uint32_t>(R[n])
                         + static_cast<uint32_t>(R[m]);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b32[0]);
        return 2;
    }
};

class LDR_R_T2 : public T1Instruction {
 public:
    LDR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int shift_n = (ti1 >> 4) & 0x3;
        uint32_t c_out;

        if (BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        if (t == Reg_pc) {
            if (address & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC(address);
            }
        } else {
            trans_.addr = address;
            trans_.action = MemAction_Read;
            trans_.xsize = 4;
            trans_.wstrb = 0;
            icpu_->dma_memop(&trans_);
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }
        return 4;
    }
};

/** 4.6.46 LDRB (immediate) */
class LDRB_I_T1 : public T1Instruction {
 public:
    LDRB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = (ti >> 6) & 0x1F;

        trans_.addr = static_cast<uint32_t>(R[n]) + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 2;
    }
};

class LDRB_I_T2 : public T1Instruction {
 public:
    LDRB_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        uint32_t address = static_cast<uint32_t>(R[n]) + imm32;

        if (t == Reg_sp) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 4;
    }
};

class LDRB_I_T3 : public T1Instruction {
 public:
    LDRB_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t index = (ti1 >> 10) & 1;
        uint32_t add = (ti1 >> 9) & 1;
        uint32_t wback = (ti1 >> 8) & 1;
        uint32_t imm32 = ti1 & 0xFF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t offset_addr = add != 0 ? (Rn + imm32) : (Rn - imm32);
        uint32_t address = index != 0 ? offset_addr : Rn;

        if (!index && !wback) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t) || (wback && n == t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (wback) {
            icpu_->setReg(n, offset_addr);
        }
        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 4;
    }
};

/** 4.6.48 LDRB (register) */
class LDRB_R_T1 : public T1Instruction {
 public:
    LDRB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        int shift_n = 0;
        uint32_t c_out;

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 2;
    }
};

class LDRB_R_T2 : public T1Instruction {
 public:
    LDRB_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB_R_T2") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int shift_n = static_cast<int>((ti1 >> 4) & 0x3);
        uint32_t c_out;

        if (t == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 4;
    }
};

/** 4.6.55 LDRH (immediate) */
class LDRH_I_T1 : public T1Instruction {
 public:
    LDRH_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRH") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = ((ti >> 6) & 0x1F) << 1;

        trans_.addr = static_cast<uint32_t>(R[n]) + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 2;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b16[0]);
        return 2;
    }
};

/** 4.6.57 LDRH (register) */
class LDRH_R_T2 : public T1Instruction {
 public:
    LDRH_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRH_R_T2") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t t = (ti1 >> 12) & 0xF;
        int shift_n = static_cast<int>((ti1 >> 4) & 0x3);
        uint32_t m = ti1 & 0xF;
        uint32_t c_out;

        if (t == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (t == Reg_pc) {
            RISCV_error("!!!!%s", "See memory hints on page 4-14");
        }

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 2;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b16[0]);
        return 4;
    }
};

/** 4.6.59 LDRSB (immediate) */
class LDRSB_I_T1 : public T1Instruction {
 public:
    LDRSB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRSB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;

        if (t == Reg_sp) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t address = static_cast<uint32_t>(R[n]) + imm32;

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        int32_t result = static_cast<int8_t>(trans_.rpayload.b8[0]);
        icpu_->setReg(t, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.61 LDRSB (register) */
class LDRSB_R_T1 : public T1Instruction {
 public:
    LDRSB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRSB_R_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t m = (ti >> 6) & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t t = ti & 0x7;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t shifted = static_cast<uint32_t>(R[m]);  // no shift
        uint32_t address = Rn + shifted;

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        int32_t result = static_cast<int8_t>(trans_.rpayload.b8[0]);
        icpu_->setReg(t, static_cast<uint32_t>(result));
        return 2;
    }
};

class LDRSB_R_T2 : public T1Instruction {
 public:
    LDRSB_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRSB_R_T2") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        uint32_t m = ti1 & 0xF;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t carry;

        if (t == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t offset = LSL_C(Rm, shift_n, &carry);
        uint32_t address = static_cast<uint32_t>(R[n]) + offset;

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        int32_t result = static_cast<int8_t>(trans_.rpayload.b8[0]);
        icpu_->setReg(t, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.68 LSL (immediate) */
class LSL_I_T1 : public T1Instruction {
 public:
    LSL_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSLS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm5 = (ti >> 6) & 0x1F;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t carry;
        uint32_t shift_n;
        uint32_t result;

        DecodeImmShift(0, imm5, &shift_n);
        result = Shift_C(Rm, SRType_LSL, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

/** LSL (register) */
class LSL_R_T1 : public T1Instruction {
 public:
    LSL_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSLS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t carry;
        uint32_t result;

        result = Shift_C(Rn, SRType_LSL, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(dn, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

class LSL_R_T2 : public T1Instruction {
 public:
    LSL_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSL.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = Shift_C(Rn, SRType_LSL, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.70 LSR (immediate) */
class LSR_I_T1 : public T1Instruction {
 public:
    LSR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSRS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm5 = (ti >> 6) & 0x1F;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t carry;
        uint32_t shift_n;
        uint32_t result;

        DecodeImmShift(1, imm5, &shift_n);
        result = Shift_C(Rm, SRType_LSR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.71 LSR (register) */
class LSR_R_T1 : public T1Instruction {
 public:
    LSR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSRS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t carry;
        uint32_t result;

        result = Shift_C(Rn, SRType_LSR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(dn, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

class LSR_R_T2 : public T1Instruction {
 public:
    LSR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = Shift_C(Rn, SRType_LSR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.74 MLA Multiply and Accumulate*/
class MLA_T1 : public T1Instruction {
 public:
    MLA_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MLA_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t a = (ti1 >> 12) & 0xF;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m) || a == Reg_sp) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        // signed or unsigned independent only if calculation in 64-bits
        result = static_cast<uint32_t>(R[a] + R[n]*R[m]);
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.75 MLS Multiply and Subtract */
class MLS_T1 : public T1Instruction {
 public:
    MLS_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MLS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t a = (ti1 >> 12) & 0xF;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m) || BadReg(a)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        // signed or unsigned independent only if calculation in 64-bits
        result = static_cast<uint32_t>(R[a] - R[n]*R[m]);
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.76 MOV (immediate) */
class MOV_I_T1 : public T1Instruction {
 public:
    MOV_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOVS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t d = (instr >> 8) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm32 = instr  & 0xFF;
        uint32_t carry = icpu_->getC();

        icpu_->setReg(d, imm32);

        if (setflags) {
            icpu_->setN((imm32 >> 31) & 1);
            icpu_->setZ(imm32 == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

class MOV_I_T2 : public T1Instruction {
 public:
    MOV_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

class MOV_I_T3 : public T1Instruction {
 public:
    MOV_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm4 = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm32 = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8;

        if (BadReg(d)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = imm32;
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.77 MOV (register) */
class MOV_R_T1 : public T1Instruction {
 public:
    MOV_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t D = (ti >> 7) & 1;
        uint32_t d = (D << 3) | (ti & 0x7);
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t result = static_cast<uint32_t>(R[m]);

        if (d == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        icpu_->setReg(d, result);
        if (d == Reg_pc) {
            ALUWritePC(result);   // ALUWritePC
        }
        return 2;
    }
};

class MOV_R_T2 : public T1Instruction {
 public:
    MOV_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t result = static_cast<uint32_t>(R[m]);

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        icpu_->setReg(d, result);           // d < 8 always
        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(0);     // No data in specification
        return 2;
    }
};

/** 4.6.84 MUL */
class MUL_T1 : public T1Instruction {
 public:
    MUL_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MUL") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t dm = ti & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[dm]);
        uint32_t result;

        result = Rn * Rm;
        icpu_->setReg(dm, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
        }
        return 2;
    }
};

class MUL_T2 : public T1Instruction {
 public:
    MUL_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MUL.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn * Rm;
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.86 MVN (register) */
class MVN_R_T1 : public T1Instruction {
 public:
    MVN_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MVNS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t result = ~static_cast<uint32_t>(R[m]);

        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            // No shift no C
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.88 NOP */
class NOP_T1 : public T1Instruction {
 public:
    NOP_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "NOP") {}

    virtual int exec(Reg64Type *payload) {
        return 2;
    }
};

/** 4.6.91 ORR (immediate) */
class ORR_I_T1 : public T1Instruction {
 public:
    ORR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ORR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || n == Reg_sp) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn | imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.92 ORR (register) */
class ORR_R_T1 : public T1Instruction {
 public:
    ORR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ORRS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t result = Rn | Rm;

        icpu_->setReg(dn, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            // C the same because no shoft
            // V unchanged
        }
        return 2;
    }
};

class ORR_R_T2 : public T1Instruction {
 public:
    ORR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ORR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        uint32_t shift_n;
        SRType shift_t = DecodeImmShift(type, (imm3 << 2) | imm2, &shift_n);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t shifted;
        uint32_t result;

        if (BadReg(d) || n == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        shifted = Shift_C(Rm, shift_t, shift_n, icpu_->getC(), &carry);
        result = Rn | shifted;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.98 POP */
class POP_T1 : public T1Instruction {
 public:
    POP_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "POP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t address = static_cast<uint32_t>(R[Reg_sp]);

        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;

        for (int i = 0; i < 8; i++) {
            if (instr & (1ul << i)) {
                trans_.addr = address;
                icpu_->dma_memop(&trans_);
                icpu_->setReg(i, trans_.rpayload.b32[0]);
                address += 4;
            }
        }
        if (instr & 0x100) {
            icpu_->setReg(Reg_sp, address + 4); // To support Exception Exit

            LoadWritePC(address);
            address += 4;
        } else {
            icpu_->setReg(Reg_sp, address);
        }
        return 2;
    }
};

class POP_T2 : public T1Instruction {
 public:
    POP_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "POP.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t P = (ti1 >> 15) & 1;
        uint32_t M = (ti1 >> 14) & 1;
        uint32_t register_list = ti1 & 0xDFFF;
        uint32_t address = static_cast<uint32_t>(R[Reg_sp]);

        if (P == 1 && M == 1) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;

        for (int i = 0; i < Reg_pc; i++) {
            if (register_list & (1ul << i)) {
                trans_.addr = address;
                icpu_->dma_memop(&trans_);
                icpu_->setReg(i, trans_.rpayload.b32[0]);
                address += 4;
            }
        }
        if (ti & (1ul << Reg_pc)) {
            icpu_->setReg(Reg_sp, address + 4);

            LoadWritePC(address);
            address += 4;
        } else {
            icpu_->setReg(Reg_sp, address);
        }
        return 4;
    }
};

/** 4.6.99 PUSH */
class PUSH_T1 : public T1Instruction {
 public:
    PUSH_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "PUSH_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint64_t address = R[Reg_sp];

        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        if (instr & 0x100) {
            address -= 4;
            trans_.addr = address;
            trans_.wpayload.b64[0] = R[Reg_lr];
            icpu_->dma_memop(&trans_);
        }

        for (int i = 7; i >= 0; i--) {
            if (instr & (1ul << i)) {
                address -= 4;
                trans_.addr = address;
                trans_.wpayload.b64[0] = R[i];
                icpu_->dma_memop(&trans_);
            }
        }

        icpu_->setReg(Reg_sp, address);
        return 2;
    }
};

/** 4.6.118 RSB (immediate) Reverse Subtract */
class RSB_I_T1 : public T1Instruction {
 public:
    RSB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "NEG") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = 0;     // Implicit zero immediate
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;

        result = AddWithCarry(~Rn, imm32, 1, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class RSB_I_T2 : public T1Instruction {
 public:
    RSB_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "RSB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(~Rn, imm32, 1, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.119 RSB (register) Reverse Subtruct */
class RSB_R_T1 : public T1Instruction {
 public:
    RSB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "RSB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        uint32_t shift_n;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        SRType shift_t = DecodeImmShift(type, (imm3 << 2) | imm2, &shift_n);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t shifted = Shift(Rm, shift_t, shift_n, icpu_->getC());

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(~Rn, shifted, 1, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.125 SBFX */
class SBFX_T1 : public T1Instruction {
 public:
    SBFX_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SBFX_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t result;
        uint32_t n = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t widthm1 = ti1 & 0x1F;
        uint32_t lsbit = (imm3 << 2) | imm2;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t msbit = lsbit + widthm1;
        uint64_t mask = (1ull << (widthm1 + 1)) - 1;

        if (BadReg(d) || BadReg(n) || msbit > 31) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = (Rn >> lsbit) & static_cast<uint32_t>(mask);
        if (result & (1ul << widthm1)) {
            result |= static_cast<uint32_t>(~mask);
        }
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.126 SDIV */
class SDIV_T1 : public T1Instruction {
 public:
    SDIV_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SDIV") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int32_t Rn = static_cast<int32_t>(R[n]);
        int32_t Rm = static_cast<int32_t>(R[m]);
        int32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (Rm == 0) {
            if (icpu_->getDZ()) {
                //icpu_->raiseSignal(ZeroDivide);
            }
            result = 0;
        } else {
            result = Rn / Rm;
        }
        icpu_->setReg(d, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.150 SMULL */
class SMULL_T1 : public T1Instruction {
 public:
    SMULL_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SMULL_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t dLo = (ti1 >> 12) & 0xF;
        uint32_t dHi = (ti1 >> 8) & 0xF;
        uint32_t m = ti1 & 0xF;
        int32_t Rn = static_cast<int32_t>(R[n]);
        int32_t Rm = static_cast<int32_t>(R[m]);
        int64_t result;

        if (BadReg(dLo) || BadReg(dHi) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (dLo == dHi) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = static_cast<int64_t>(Rn) * static_cast<int64_t>(Rm);
        icpu_->setReg(dHi, static_cast<uint32_t>(result >> 32));
        icpu_->setReg(dLo, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.160 STMDB/STMFD */
class STMDB_T1 : public T1Instruction {
 public:
    STMDB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STMDB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t wback = (ti >> 5) & 1;
        uint64_t address = R[n];
        uint32_t BitCount = 0;

        if (n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        for (int i = 14; i >= 0; i--) {
            if (ti1 & (1ul << i)) {
                address -= 4;
                trans_.addr = address;
                trans_.wpayload.b64[0] = R[i];
                icpu_->dma_memop(&trans_);
                BitCount++;
            }
        }
        if (BitCount < 2) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (wback) {
            if (n == Reg_sp) {
                // T2_PUSH equivalent
                icpu_->setReg(n, static_cast<uint32_t>(R[n] - 4*BitCount));
            } else {
                icpu_->setReg(n, static_cast<uint32_t>(R[n] + 4*BitCount));
            }
        }
        return 4;
    }
};

/** 4.6.161 STMIA Store Multiple Increment After */
class STMIA_T1 : public T1Instruction {
 public:
    STMIA_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STMIA_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = (ti >> 8) & 0x7;
        uint32_t register_list = ti & 0xFF;
        uint64_t address = static_cast<uint32_t>(R[n]);
        uint32_t BitCount = 0;

        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        for (int i = 0; i < 8; i++) {
            if (((register_list >> i) & 1) == 0) {
                continue;
            }
            if (static_cast<unsigned>(i) == n) {  // wback always TRUE
                if (BitCount == 0) {              // LowestSetBit
                    trans_.addr = address;
                    trans_.wpayload.b64[0] = R[i];
                    icpu_->dma_memop(&trans_);
                } else {
                    RISCV_error("%s", "UNKNOWN");
                }
            } else {
                trans_.addr = address;
                trans_.wpayload.b64[0] = R[i];
                icpu_->dma_memop(&trans_);
            }
            address += 4;
            BitCount++;
        }
        if (BitCount < 1) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        icpu_->setReg(n, static_cast<uint32_t>(R[n] + 4*BitCount));
        return 2;
    }
};

/** 4.6.162 STR (immediate) */
class STR_I_T1 : public T1Instruction {
 public:
    STR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRI") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = ((ti >> 6) & 0x1F) << 2;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STR_I_T2 : public T1Instruction {
 public:
    STR_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = (ti >> 8) & 0x7;
        uint32_t n = Reg_sp;
        uint64_t imm32 = (ti & 0xFF) << 2;

        trans_.addr = static_cast<uint32_t>(R[n]) + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

/** 4.6.163 STR (register) */
class STR_R_T1 : public T1Instruction {
 public:
    STR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        uint32_t address = static_cast<uint32_t>(R[n])
                         + static_cast<uint32_t>(R[m]);

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STR_R_T2 : public T1Instruction {
 public:
    STR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t c_out;
        uint32_t address = Rn + LSL_C(Rm, shift_n, &c_out);

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (t == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.164 STRB (immediate) */
class STRB_I_T1 : public T1Instruction {
 public:
    STRB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = (ti >> 6) & 0x1F;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STRB_I_T2 : public T1Instruction {
 public:
    STRB_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint64_t imm32 = ti1 & 0xFFF;

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

class STRB_I_T3 : public T1Instruction {
 public:
    STRB_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFF;
        uint32_t index = (ti1 >> 10) & 1;
        uint32_t add = (ti1 >> 9) & 1;
        uint32_t wback = (ti1 >> 8) & 1;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t offset_addr;
        uint32_t address;

        if (n == Reg_pc || (index == 0 && wback == 0)) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t) || (wback && n == t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (add) {
            offset_addr = Rn + imm32;
        } else {
            offset_addr = Rn - imm32;
        }
        if (index) {
            address = offset_addr;
        } else {
            address = Rn;
        }
        if (wback) {
            icpu_->setReg(n, offset_addr);
        }
        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.165 */
class STRB_R_T1 : public T1Instruction {
 public:
    STRB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        // no shift
        uint32_t address = static_cast<uint32_t>(R[n]);

        trans_.addr = address + Rm;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STRB_R_T2 : public T1Instruction {
 public:
    STRB_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t c_out;
        uint32_t address = Rn + LSL_C(Rm, shift_n, &c_out);

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (t == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.167 STRD (immediate) */
class STRD_I_T1 : public T1Instruction {
 public:
    STRD_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRD_I_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t index = (ti >> 8) & 1;
        uint32_t add = (ti >> 7) & 1;
        uint32_t wback = (ti >> 5) & 1;
        uint32_t n = ti & 0xF;
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t t2 = (ti1 >> 8) & 0xF;
        uint32_t imm32 = (ti1 & 0xFF) << 2;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t offset_addr = add != 0 ? Rn + imm32 : Rn - imm32;
        uint32_t address = index != 0 ? offset_addr : Rn;

        if (wback && (t == n || t2 == n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (n == Reg_pc || BadReg(t) || BadReg(t2)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (wback) {
            icpu_->setReg(n, offset_addr);
        }
        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);

        trans_.addr = address + 4;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t2]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.172 STRH (immediate) */
class STRH_I_T1 : public T1Instruction {
 public:
    STRH_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRH") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = ((ti >> 6) & 0x1F) << 1;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 2;
        trans_.wstrb = 0x3;
        trans_.wpayload.b32[0] = static_cast<uint16_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STRH_I_T2 : public T1Instruction {
 public:
    STRH_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRH_I_T2") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        uint32_t address = static_cast<uint32_t>(R[n]) + imm32;

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 2;
        trans_.wstrb = 0x3;
        trans_.wpayload.b32[0] = static_cast<uint16_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

class STRH_I_T3 : public T1Instruction {
 public:
    STRH_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRH_I_T3") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t index = (ti1 >> 10) & 1;
        uint32_t add = (ti1 >> 9) & 1;
        uint32_t wback = (ti1 >> 8) & 1;
        uint32_t imm32 = ti1 & 0xFF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t offset_addr = add != 0 ? Rn + imm32 : Rn - imm32;
        uint32_t address = index != 0 ? offset_addr : Rn;

        if (n == Reg_pc || (!index && !wback)) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t) || (wback && n == t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (wback) {
            icpu_->setReg(n, offset_addr);
        }

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 2;
        trans_.wstrb = 0x3;
        trans_.wpayload.b32[0] = static_cast<uint16_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.176 SUB (immediate) */
class SUB_I_T1 : public T1Instruction {
 public:
    SUB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm32 = (ti >> 6) & 0x7;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class SUB_I_T2 : public T1Instruction {
 public:
    SUB_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t dn = (ti >> 8) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm32 = ti & 0xFF;
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setReg(dn, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class SUB_I_T3 : public T1Instruction {
 public:
    SUB_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.177 SUB (register) */
class SUB_R_T1 : public T1Instruction {
 public:
    SUB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t d = instr & 0x7;
        uint32_t n = (instr >> 3) & 0x7;
        uint32_t m = (instr >> 6) & 0x7;
        bool setflags = !icpu_->InITBlock();

        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        
        uint32_t shifted = Rm;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result = AddWithCarry(Rn, ~shifted, 1, &overflow, &carry);

        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class SUB_R_T2 : public T1Instruction {
 public:
    SUB_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        bool setflags = (ti >> 4) & 1;
        uint32_t n = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t shifted;

        SRType shift_t = DecodeImmShift(type, (imm3 << 2) | imm2, &shift_n);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        shifted = Shift_C(Rm, shift_t, shift_n, icpu_->getC(), &carry);
        result = AddWithCarry(Rn, ~shifted, 1, &overflow, &carry);
        // d Cannot be Reg_pc, see CMP (register) T3_CMP_R
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.178 SUB (SP minus immediate) */
class SUBSP_I_T1 : public T1Instruction {
 public:
    SUBSP_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBSP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t sp = static_cast<uint32_t>(R[Reg_sp]);
        uint32_t instr = payload->buf16[0];
        uint32_t imm32 = (instr & 0x7F) << 2;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;

        result = AddWithCarry(sp, ~imm32, 1, &overflow, &carry);
        icpu_->setReg(Reg_sp, result);
        // Not modify flags
        return 2;
    }
};

/** 4.6.182 SXTAB Signed Extend and Add Byte extracts */
class SXTAB_T1 : public T1Instruction {
 public:
    SXTAB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SXTAB_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t rotation = ((ti1 >> 4) & 0x3) << 3;
        uint32_t m = ti1 & 0xF;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;

        if (BadReg(d) || n == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        int8_t rotated = static_cast<int8_t>(ROR_C(Rm, rotation, &carry));
        uint32_t result = Rn
                + static_cast<uint32_t>(static_cast<int32_t>(rotated));
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.185 SXTB Signed Extend Byte extract */
class SXTB_T1 : public T1Instruction {
 public:
    SXTB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SXTB_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        int8_t rotated = static_cast<int8_t>(R[m]);  // no rotation
        uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(rotated));
        icpu_->setReg(d, result);
        return 2;
    }
};

/** 4.6.188 TBB: Table Branch Cycle */
class TBB_T1 : public T1Instruction {
 public:
    TBB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "TBB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf32[0];
        uint32_t n = ti & 0xF;
        uint32_t m = (ti >> 16) & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        if (n == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (n == Reg_pc) {
            Rn += 4;
        }

        trans_.addr = Rn + Rm;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        
        icpu_->dma_memop(&trans_);
        uint32_t halfwords = trans_.rpayload.b8[0];
        uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4
                     + (halfwords << 1);
        BranchWritePC(npc);
        return 4;
    }
};

/** 4.6.192 TST (immediate) */
class TST_I_T1 : public T1Instruction {
 public:
    TST_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "TST.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti0 = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t n = ti0  & 0xF;
        uint32_t i = (ti0 >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn & imm32;

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        // V unchanged
        return 4;
    }
};

/** 4.6.193 TST (register) */
class TST_R_T1 : public T1Instruction {
 public:
    TST_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "TSTS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = ti  & 0x7;
        uint32_t m = (ti >> 3)  & 0x7;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t shifted = static_cast<uint32_t>(R[m]);
        uint32_t result = Rn & shifted;

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        // C no shift, no change
        // V unchanged
        return 2;
    }
};

/** 4.6.197 UBFX */
class UBFX_T1 : public T1Instruction {
 public:
    UBFX_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UBFX") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti0 = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti0 & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t lsbit = (imm3 << 2) | imm2;
        uint32_t widthm1 = ti1 & 0x1F;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t msbit = lsbit + widthm1;
        uint64_t mask = (1ull << (widthm1 + 1)) - 1;

        if (BadReg(d) || BadReg(n) || msbit > 31) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = (Rn >> lsbit) & static_cast<uint32_t>(mask);
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.198 UDIV */
class UDIV_T1 : public T1Instruction {
 public:
    UDIV_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UDIV") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (Rm == 0) {
            if (icpu_->getDZ()) {
                //icpu_->raiseSignal(ZeroDivide);
            }
            result = 0;
        } else {
            result = Rn / Rm;
        }
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.207 UMULL */
class UMULL_T1 : public T1Instruction {
 public:
    UMULL_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UMULL") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t dLo = (ti1 >> 12) & 0xF;
        uint32_t dHi = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint64_t result;

        if (BadReg(dLo) || BadReg(dHi) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (dHi == dLo) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = static_cast<uint64_t>(Rn) * static_cast<uint64_t>(Rm);
        icpu_->setReg(dHi, static_cast<uint32_t>(result >> 32));
        icpu_->setReg(dLo, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.221 UXTAB */
class UXTAB_T1 : public T1Instruction {
 public:
    UXTAB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UXTAB_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t rotation = ((ti1 >> 4) & 0x3) << 3;
        uint32_t m = ti1 & 0xF;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t rotated;
        uint32_t carry;
        uint32_t result;

        if (BadReg(d) || n == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        rotated = ROR_C(Rm, rotation, &carry);
        result = Rn + (rotated & 0xFF);
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.223 UXTAH */
class UXTAH_T1 : public T1Instruction {
 public:
    UXTAH_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UXTAH") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t rotation = ((ti1 >> 4) & 0x3) << 3;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t rotated;
        uint32_t carry;
        uint32_t result;

        if (BadReg(d) || n == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        rotated = ROR_C(Rm, rotation, &carry);
        result = Rn + (rotated & 0xFFFF);
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.224 UXTB */
class UXTB_T1 : public T1Instruction {
 public:
    UXTB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UXTB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        // rotation = 0 no need to call ROR_C
        icpu_->setReg(d, Rm & 0xFF);
        return 2;
    }
};

/** 4.6.226 UXTH */
class UXTH_T1 : public T1Instruction {
 public:
    UXTH_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UXTH") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        // rotation = 0 no need to call ROR_C
        icpu_->setReg(d, Rm & 0xFFFF);
        return 2;
    }
};

void CpuCortex_Functional::addThumb2Isa() {
    isaTableArmV7_[T1_ADC_I] = new ADC_I_T1(this);
    isaTableArmV7_[T1_ADD_I] = new ADD_I_T1(this);
    isaTableArmV7_[T2_ADD_I] = new ADD_I_T2(this);
    isaTableArmV7_[T3_ADD_I] = new ADD_I_T3(this);
    isaTableArmV7_[T1_ADD_R] = new ADD_R_T1(this);
    isaTableArmV7_[T2_ADD_R] = new ADD_R_T2(this);
    isaTableArmV7_[T3_ADD_R] = new ADD_R_T3(this);
    isaTableArmV7_[T1_ADDSP_I] = new ADDSP_I_T1(this);
    isaTableArmV7_[T2_ADDSP_I] = new ADDSP_I_T2(this);
    isaTableArmV7_[T1_ADR] = new ADR_T1(this);
    isaTableArmV7_[T1_AND_I] = new AND_I_T1(this);
    isaTableArmV7_[T1_AND_R] = new AND_R_T1(this);
    isaTableArmV7_[T2_AND_R] = new AND_R_T2(this);
    isaTableArmV7_[T1_ASR_I] = new ASR_I_T1(this);
    isaTableArmV7_[T1_B] = new B_T1(this);
    isaTableArmV7_[T2_B] = new B_T2(this);
    isaTableArmV7_[T3_B] = new B_T3(this);
    isaTableArmV7_[T4_B] = new B_T4(this);
    isaTableArmV7_[T1_BIC_I] = new BIC_I_T1(this);
    isaTableArmV7_[T1_BKPT] = new BKPT_T1(this);
    isaTableArmV7_[T1_BL_I] = new BL_I_T1(this);
    isaTableArmV7_[T1_BLX_R] = new BLX_R_T1(this);
    isaTableArmV7_[T1_BX] = new BX_T1(this);
    isaTableArmV7_[T1_CBNZ] = new CBNZ_T1(this);
    isaTableArmV7_[T1_CBZ] = new CBZ_T1(this);
    isaTableArmV7_[T1_CMP_I] = new CMP_I_T1(this);
    isaTableArmV7_[T2_CMP_I] = new CMP_I_T2(this);
    isaTableArmV7_[T1_CMP_R] = new CMP_R_T1(this);
    isaTableArmV7_[T2_CMP_R] = new CMP_R_T2(this);
    isaTableArmV7_[T1_CPS] = new CPS_T1(this);
    isaTableArmV7_[T1_EOR_I] = new EOR_I_T1(this);
    isaTableArmV7_[T1_EOR_R] = new EOR_R_T1(this);
    isaTableArmV7_[T1_IT] = new IT_T1(this);
    isaTableArmV7_[T2_LDMIA] = new LDMIA_T2(this);
    isaTableArmV7_[T1_LDR_I] = new LDR_I_T1(this);
    isaTableArmV7_[T2_LDR_I] = new LDR_I_T2(this);
    isaTableArmV7_[T3_LDR_I] = new LDR_I_T3(this);
    isaTableArmV7_[T4_LDR_I] = new LDR_I_T4(this);
    isaTableArmV7_[T1_LDR_L] = new LDR_L_T1(this);
    isaTableArmV7_[T2_LDR_L] = new LDR_L_T2(this);
    isaTableArmV7_[T1_LDR_R] = new LDR_R_T1(this);
    isaTableArmV7_[T2_LDR_R] = new LDR_R_T2(this);
    isaTableArmV7_[T1_LDRB_I] = new LDRB_I_T1(this);
    isaTableArmV7_[T2_LDRB_I] = new LDRB_I_T2(this);
    isaTableArmV7_[T3_LDRB_I] = new LDRB_I_T3(this);
    isaTableArmV7_[T1_LDRB_R] = new LDRB_R_T1(this);
    isaTableArmV7_[T2_LDRB_R] = new LDRB_R_T2(this);
    isaTableArmV7_[T1_LDRH_I] = new LDRH_I_T1(this);
    isaTableArmV7_[T2_LDRH_R] = new LDRH_R_T2(this);
    isaTableArmV7_[T1_LDRSB_I] = new LDRSB_I_T1(this);
    isaTableArmV7_[T1_LDRSB_R] = new LDRSB_R_T1(this);
    isaTableArmV7_[T2_LDRSB_R] = new LDRSB_R_T2(this);
    isaTableArmV7_[T1_LSL_I] = new LSL_I_T1(this);
    isaTableArmV7_[T1_LSL_R] = new LSL_R_T1(this);
    isaTableArmV7_[T2_LSL_R] = new LSL_R_T2(this);
    isaTableArmV7_[T1_LSR_I] = new LSR_I_T1(this);
    isaTableArmV7_[T1_LSR_R] = new LSR_R_T1(this);
    isaTableArmV7_[T2_LSR_R] = new LSR_R_T2(this);
    isaTableArmV7_[T1_MLA] = new MLA_T1(this);
    isaTableArmV7_[T1_MLS] = new MLS_T1(this);
    isaTableArmV7_[T1_MOV_I] = new MOV_I_T1(this);
    isaTableArmV7_[T2_MOV_I] = new MOV_I_T2(this);
    isaTableArmV7_[T3_MOV_I] = new MOV_I_T3(this);
    isaTableArmV7_[T1_MOV_R] = new MOV_R_T1(this);
    isaTableArmV7_[T2_MOV_R] = new MOV_R_T2(this);
    isaTableArmV7_[T1_MUL] = new MUL_T1(this);
    isaTableArmV7_[T2_MUL] = new MUL_T2(this);
    isaTableArmV7_[T1_MVN_R] = new MVN_R_T1(this);
    isaTableArmV7_[T1_NOP] = new NOP_T1(this);
    isaTableArmV7_[T1_POP] = new POP_T1(this);
    isaTableArmV7_[T2_POP] = new POP_T2(this);
    isaTableArmV7_[T1_PUSH] = new PUSH_T1(this);
    isaTableArmV7_[T1_ORR_I] = new ORR_I_T1(this);
    isaTableArmV7_[T1_ORR_R] = new ORR_R_T1(this);
    isaTableArmV7_[T2_ORR_R] = new ORR_R_T2(this);
    isaTableArmV7_[T1_RSB_I] = new RSB_I_T1(this);
    isaTableArmV7_[T2_RSB_I] = new RSB_I_T2(this);
    isaTableArmV7_[T1_RSB_R] = new RSB_R_T1(this);
    isaTableArmV7_[T1_SBFX] = new SBFX_T1(this);
    isaTableArmV7_[T1_SDIV] = new SDIV_T1(this);
    isaTableArmV7_[T1_SMULL] = new SMULL_T1(this);
    isaTableArmV7_[T1_STMDB] = new STMDB_T1(this);
    isaTableArmV7_[T1_STMIA] = new STMIA_T1(this);
    isaTableArmV7_[T1_STR_I] = new STR_I_T1(this);
    isaTableArmV7_[T2_STR_I] = new STR_I_T2(this);
    isaTableArmV7_[T1_STR_R] = new STR_R_T1(this);
    isaTableArmV7_[T2_STR_R] = new STR_R_T2(this);
    isaTableArmV7_[T1_STRB_I] = new STRB_I_T1(this);
    isaTableArmV7_[T2_STRB_I] = new STRB_I_T2(this);
    isaTableArmV7_[T3_STRB_I] = new STRB_I_T3(this);
    isaTableArmV7_[T1_STRB_R] = new STRB_R_T1(this);
    isaTableArmV7_[T2_STRB_R] = new STRB_R_T2(this);
    isaTableArmV7_[T1_STRD_I] = new STRD_I_T1(this);
    isaTableArmV7_[T1_STRH_I] = new STRH_I_T1(this);
    isaTableArmV7_[T2_STRH_I] = new STRH_I_T2(this);
    isaTableArmV7_[T3_STRH_I] = new STRH_I_T3(this);
    isaTableArmV7_[T1_SUB_I] = new SUB_I_T1(this);
    isaTableArmV7_[T2_SUB_I] = new SUB_I_T2(this);
    isaTableArmV7_[T3_SUB_I] = new SUB_I_T3(this);
    isaTableArmV7_[T1_SUB_R] = new SUB_R_T1(this);
    isaTableArmV7_[T2_SUB_R] = new SUB_R_T2(this);
    isaTableArmV7_[T1_SUBSP_I] = new SUBSP_I_T1(this);
    isaTableArmV7_[T1_SXTAB] = new SXTAB_T1(this);
    isaTableArmV7_[T1_SXTB] = new SXTB_T1(this);
    isaTableArmV7_[T1_TBB] = new TBB_T1(this);
    isaTableArmV7_[T1_TST_I] = new TST_I_T1(this);
    isaTableArmV7_[T1_TST_R] = new TST_R_T1(this);
    isaTableArmV7_[T1_UBFX] = new UBFX_T1(this);
    isaTableArmV7_[T1_UDIV] = new UDIV_T1(this);
    isaTableArmV7_[T1_UMULL] = new UMULL_T1(this);
    isaTableArmV7_[T1_UXTAB] = new UXTAB_T1(this);
    isaTableArmV7_[T1_UXTAH] = new UXTAH_T1(this);
    isaTableArmV7_[T1_UXTB] = new UXTB_T1(this);
    isaTableArmV7_[T1_UXTH] = new UXTH_T1(this);
}

}  // namespace debugger
