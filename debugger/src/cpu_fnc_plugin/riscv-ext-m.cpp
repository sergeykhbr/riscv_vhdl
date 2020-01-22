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
        int64_t res;
        u.value = payload->buf32[0];
        if (R[u.bits.rs2]) {
            res = static_cast<int64_t>(R[u.bits.rs1])
                 / static_cast<int64_t>(R[u.bits.rs2]);
        } else {
            res = -1;
        }
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
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
        uint64_t res;
        u.value = payload->buf32[0];
        if (R[u.bits.rs2]) {
            res = R[u.bits.rs1] / R[u.bits.rs2];
        } else {
            // Difference relative x86
            res = ~0ull;
        }
        icpu_->setReg(u.bits.rd, res);
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
        uint32_t a1 = static_cast<uint32_t>(R[u.bits.rs1]);
        uint32_t a2 = static_cast<uint32_t>(R[u.bits.rs2]);
        int32_t res;
        if (a2) {
            // DIVUW also sign-extended
            res = static_cast<int32_t>(a1 / a2);
        } else {
            res = -1;
        }
        icpu_->setReg(u.bits.rd, static_cast<int64_t>(res));
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
        int64_t res;
        if (divisor) {
            // ! Integer overflow on x86
            res = static_cast<int64_t>(divident) /
                  static_cast<int64_t>(divisor);
        } else {
            res = -1;
        }
        res <<= 32;
        res >>= 32;
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
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
        int64_t res;
        u.value = payload->buf32[0];
        res = static_cast<int64_t>(R[u.bits.rs1])
                * static_cast<int64_t>(R[u.bits.rs2]);
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
        return 4;
    }
};

/**
 * @brief The MULH signed multiplication
 *
 * MULH, MULHU, and MULHSU perform the same multiplication but return
 * the upper XLEN bits of the full 2*XLEN-bit product, for signed*signed,
 * unsigned*unsigned, and signed*unsigned multiplication respectively
 */
class MULH : public RiscvInstruction {
 public:
    MULH(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "MULH", "0000001??????????001?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        int64_t res;
        u.value = payload->buf32[0];
        uint64_t a1 = R[u.bits.rs1];
        uint64_t a2 = R[u.bits.rs2];
        uint64_t a1s = a1;
        uint64_t a2s = a2;
        bool inva1 = 0;
        bool inva2 = 0;
        bool zero;
        bool inv;
        if ((a1 >> 63) && (a1 << 1)) {  // not -0
            a1s = ~a1 + 1;
            inva1 = true;
        }
        if ((a2 >> 63) && (a2 << 1)) {  // not -0
            a2s = ~a2 + 1;
            inva2 = true;
        }
        zero = a1s == 0 || a2s == 0;
        inv = inva1 ^ inva2;

        struct LevelType {
            uint64_t val[2];
        };
        LevelType lvl0[32];
        LevelType lvl1[16];
        LevelType lvl2[8];
        LevelType lvl3[4];
        LevelType lvl4[2];
        LevelType lvl5;
        uint64_t carry;
        uint64_t msb1, msb2, msbres;
        for (int i = 0; i < 32; i++) {
            switch ((a2s >> 2*i) & 0x3) {
            case 0:
                lvl0[i].val[0] = 0;
                lvl0[i].val[1] = 0;
                break;
            case 1:
                lvl0[i].val[0] = a1s;
                lvl0[i].val[1] = 0;
                break;
            case 2:
                lvl0[i].val[0] = a1s << 1;
                lvl0[i].val[1] = a1s >> 63;
                break;
            case 3:
                lvl0[i].val[0] = (a1s << 1) + a1s;
                // (a1[63] & a2[63]) | (a1[63] & !res[63])
                msb1 = (a1s >> 62) & 1;
                msb2 = (a1s >> 63) & 1;
                msbres = lvl0[i].val[0] >> 63;
                carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
                lvl0[i].val[1] = (a1s >> 63) + carry;
                break;
            default:;
            }
        }

        for (int i = 0; i < 16; i++) {
            lvl1[i].val[0] = (lvl0[2*i+1].val[0] << 2) + lvl0[2*i].val[0];
            msb1 = (lvl0[2*i+1].val[0] >> 61) & 1;
            msb2 = (lvl0[2*i].val[0] >> 63) & 1;
            msbres = lvl1[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl1[i].val[1] = (lvl0[2*i+1].val[1] << 2) | (lvl0[2*i+1].val[0] >> 62);
            lvl1[i].val[1] += lvl0[2*i].val[1] + carry;
        }

        for (int i = 0; i < 8; i++) {
            lvl2[i].val[0] = (lvl1[2*i+1].val[0] << 4) + lvl1[2*i].val[0];
            msb1 = (lvl1[2*i+1].val[0] >> 59) & 1;
            msb2 = (lvl1[2*i].val[0] >> 63) & 1;
            msbres = lvl2[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl2[i].val[1] = (lvl1[2*i+1].val[1] << 4) | (lvl1[2*i+1].val[0] >> 60);
            lvl2[i].val[1] += lvl1[2*i].val[1] + carry;
        }

        for (int i = 0; i < 4; i++) {
            lvl3[i].val[0] = (lvl2[2*i+1].val[0] << 8) + lvl2[2*i].val[0];
            msb1 = (lvl2[2*i+1].val[0] >> 55) & 1;
            msb2 = (lvl2[2*i].val[0] >> 63) & 1;
            msbres = lvl3[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl3[i].val[1] = (lvl2[2*i+1].val[1] << 8) | (lvl2[2*i+1].val[0] >> 56);
            lvl3[i].val[1] += lvl2[2*i].val[1] + carry;
        }

        for (int i = 0; i < 2; i++) {
            lvl4[i].val[0] = (lvl3[2*i+1].val[0] << 16) + lvl3[2*i].val[0];
            msb1 = (lvl3[2*i+1].val[0] >> 47) & 1;
            msb2 = (lvl3[2*i].val[0] >> 63) & 1;
            msbres = lvl4[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl4[i].val[1] = (lvl3[2*i+1].val[1] << 16) | (lvl3[2*i+1].val[0] >> 48);
            lvl4[i].val[1] += lvl3[2*i].val[1] + carry;
        }

        lvl5.val[0] = (lvl4[1].val[0] << 32) + lvl4[0].val[0];
        msb1 = (lvl4[1].val[0] >> 31) & 1;
        msb2 = (lvl4[0].val[0] >> 63) & 1;
        msbres = lvl5.val[0] >> 63;
        carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
        lvl5.val[1] = (lvl4[1].val[1] << 32) | (lvl4[1].val[0] >> 32);
        lvl5.val[1] += lvl4[0].val[1] + carry;

        if (zero) {
            res = 0;
        } else if (inv) {
            res = ~lvl5.val[1];
        } else {
            res = lvl5.val[1];
        }
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
        return 4;
    }
};

class MULHSU : public RiscvInstruction {
 public:
    MULHSU(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "MULHSU", "0000001??????????010?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        int64_t res;
        u.value = payload->buf32[0];
        uint64_t a1 = R[u.bits.rs1];
        uint64_t a2 = R[u.bits.rs2];
        uint64_t a1s = a1;
        bool inv = 0;
        bool zero;
        if ((a1 >> 63) && (a1 << 1)) {  // not -0
            a1s = ~a1 + 1;
            inv = true;
        }
        zero = a1s == 0 || a2 == 0;

        struct LevelType {
            uint64_t val[2];
        };
        LevelType lvl0[32];
        LevelType lvl1[16];
        LevelType lvl2[8];
        LevelType lvl3[4];
        LevelType lvl4[2];
        LevelType lvl5;
        uint64_t carry;
        uint64_t msb1, msb2, msbres;
        for (int i = 0; i < 32; i++) {
            switch ((a2 >> 2*i) & 0x3) {
            case 0:
                lvl0[i].val[0] = 0;
                lvl0[i].val[1] = 0;
                break;
            case 1:
                lvl0[i].val[0] = a1s;
                lvl0[i].val[1] = 0;
                break;
            case 2:
                lvl0[i].val[0] = a1s << 1;
                lvl0[i].val[1] = a1s >> 63;
                break;
            case 3:
                lvl0[i].val[0] = (a1s << 1) + a1s;
                // (a1[63] & a2[63]) | (a1[63] & !res[63])
                msb1 = (a1s >> 62) & 1;
                msb2 = (a1s >> 63) & 1;
                msbres = lvl0[i].val[0] >> 63;
                carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
                lvl0[i].val[1] = (a1s >> 63) + carry;
                break;
            default:;
            }
        }

        for (int i = 0; i < 16; i++) {
            lvl1[i].val[0] = (lvl0[2*i+1].val[0] << 2) + lvl0[2*i].val[0];
            msb1 = (lvl0[2*i+1].val[0] >> 61) & 1;
            msb2 = (lvl0[2*i].val[0] >> 63) & 1;
            msbres = lvl1[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl1[i].val[1] = (lvl0[2*i+1].val[1] << 2) | (lvl0[2*i+1].val[0] >> 62);
            lvl1[i].val[1] += lvl0[2*i].val[1] + carry;
        }

        for (int i = 0; i < 8; i++) {
            lvl2[i].val[0] = (lvl1[2*i+1].val[0] << 4) + lvl1[2*i].val[0];
            msb1 = (lvl1[2*i+1].val[0] >> 59) & 1;
            msb2 = (lvl1[2*i].val[0] >> 63) & 1;
            msbres = lvl2[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl2[i].val[1] = (lvl1[2*i+1].val[1] << 4) | (lvl1[2*i+1].val[0] >> 60);
            lvl2[i].val[1] += lvl1[2*i].val[1] + carry;
        }

        for (int i = 0; i < 4; i++) {
            lvl3[i].val[0] = (lvl2[2*i+1].val[0] << 8) + lvl2[2*i].val[0];
            msb1 = (lvl2[2*i+1].val[0] >> 55) & 1;
            msb2 = (lvl2[2*i].val[0] >> 63) & 1;
            msbres = lvl3[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl3[i].val[1] = (lvl2[2*i+1].val[1] << 8) | (lvl2[2*i+1].val[0] >> 56);
            lvl3[i].val[1] += lvl2[2*i].val[1] + carry;
        }

        for (int i = 0; i < 2; i++) {
            lvl4[i].val[0] = (lvl3[2*i+1].val[0] << 16) + lvl3[2*i].val[0];
            msb1 = (lvl3[2*i+1].val[0] >> 47) & 1;
            msb2 = (lvl3[2*i].val[0] >> 63) & 1;
            msbres = lvl4[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl4[i].val[1] = (lvl3[2*i+1].val[1] << 16) | (lvl3[2*i+1].val[0] >> 48);
            lvl4[i].val[1] += lvl3[2*i].val[1] + carry;
        }

        lvl5.val[0] = (lvl4[1].val[0] << 32) + lvl4[0].val[0];
        msb1 = (lvl4[1].val[0] >> 31) & 1;
        msb2 = (lvl4[0].val[0] >> 63) & 1;
        msbres = lvl5.val[0] >> 63;
        carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
        lvl5.val[1] = (lvl4[1].val[1] << 32) | (lvl4[1].val[0] >> 32);
        lvl5.val[1] += lvl4[0].val[1] + carry;

        if (zero) {
            res = 0;
        } else if (inv) {
            res = ~lvl5.val[1];
        } else {
            res = lvl5.val[1];
        }
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
        return 4;
    }
};

class MULHU : public RiscvInstruction {
 public:
    MULHU(CpuRiver_Functional *icpu)
        : RiscvInstruction(icpu, "MULHU", "0000001??????????011?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        int64_t res;
        u.value = payload->buf32[0];
        uint64_t a1 = R[u.bits.rs1];
        uint64_t a2 = R[u.bits.rs2];

        struct LevelType {
            uint64_t val[2];
        };
        LevelType lvl0[32];
        LevelType lvl1[16];
        LevelType lvl2[8];
        LevelType lvl3[4];
        LevelType lvl4[2];
        LevelType lvl5;
        uint64_t carry;
        uint64_t msb1, msb2, msbres;
        for (int i = 0; i < 32; i++) {
            switch ((a2 >> 2*i) & 0x3) {
            case 0:
                lvl0[i].val[0] = 0;
                lvl0[i].val[1] = 0;
                break;
            case 1:
                lvl0[i].val[0] = a1;
                lvl0[i].val[1] = 0;
                break;
            case 2:
                lvl0[i].val[0] = a1 << 1;
                lvl0[i].val[1] = a1 >> 63;
                break;
            case 3:
                lvl0[i].val[0] = (a1 << 1) + a1;
                // (a1[63] & a2[63]) | (a1[63] & !res[63])
                msb1 = (a1 >> 62) & 1;
                msb2 = (a1 >> 63) & 1;
                msbres = lvl0[i].val[0] >> 63;
                carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
                lvl0[i].val[1] = (a1 >> 63) + carry;
                break;
            default:;
            }
        }

        for (int i = 0; i < 16; i++) {
            lvl1[i].val[0] = (lvl0[2*i+1].val[0] << 2) + lvl0[2*i].val[0];
            msb1 = (lvl0[2*i+1].val[0] >> 61) & 1;
            msb2 = (lvl0[2*i].val[0] >> 63) & 1;
            msbres = lvl1[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl1[i].val[1] = (lvl0[2*i+1].val[1] << 2) | (lvl0[2*i+1].val[0] >> 62);
            lvl1[i].val[1] += lvl0[2*i].val[1] + carry;
        }

        for (int i = 0; i < 8; i++) {
            lvl2[i].val[0] = (lvl1[2*i+1].val[0] << 4) + lvl1[2*i].val[0];
            msb1 = (lvl1[2*i+1].val[0] >> 59) & 1;
            msb2 = (lvl1[2*i].val[0] >> 63) & 1;
            msbres = lvl2[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl2[i].val[1] = (lvl1[2*i+1].val[1] << 4) | (lvl1[2*i+1].val[0] >> 60);
            lvl2[i].val[1] += lvl1[2*i].val[1] + carry;
        }

        for (int i = 0; i < 4; i++) {
            lvl3[i].val[0] = (lvl2[2*i+1].val[0] << 8) + lvl2[2*i].val[0];
            msb1 = (lvl2[2*i+1].val[0] >> 55) & 1;
            msb2 = (lvl2[2*i].val[0] >> 63) & 1;
            msbres = lvl3[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl3[i].val[1] = (lvl2[2*i+1].val[1] << 8) | (lvl2[2*i+1].val[0] >> 56);
            lvl3[i].val[1] += lvl2[2*i].val[1] + carry;
        }

        for (int i = 0; i < 2; i++) {
            lvl4[i].val[0] = (lvl3[2*i+1].val[0] << 16) + lvl3[2*i].val[0];
            msb1 = (lvl3[2*i+1].val[0] >> 47) & 1;
            msb2 = (lvl3[2*i].val[0] >> 63) & 1;
            msbres = lvl4[i].val[0] >> 63;
            carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
            lvl4[i].val[1] = (lvl3[2*i+1].val[1] << 16) | (lvl3[2*i+1].val[0] >> 48);
            lvl4[i].val[1] += lvl3[2*i].val[1] + carry;
        }

        lvl5.val[0] = (lvl4[1].val[0] << 32) + lvl4[0].val[0];
        msb1 = (lvl4[1].val[0] >> 31) & 1;
        msb2 = (lvl4[0].val[0] >> 63) & 1;
        msbres = lvl5.val[0] >> 63;
        carry = (msb1 & msb2) | ((msb1 | msb2) & !msbres);
        lvl5.val[1] = (lvl4[1].val[1] << 32) | (lvl4[1].val[0] >> 32);
        lvl5.val[1] += lvl4[0].val[1] + carry;

        res = lvl5.val[1];
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
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
        int32_t resw;
        int64_t res;

        resw = m1 * m2;
        res = static_cast<int64_t>(resw);
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
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
        int64_t res;
        u.value = payload->buf32[0];
        if (R[u.bits.rs2]) {
            res = static_cast<int64_t>(R[u.bits.rs1])
                 % static_cast<int64_t>(R[u.bits.rs2]);
        } else {
            res = static_cast<int64_t>(R[u.bits.rs1]);
        }
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
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
        uint64_t res;
        u.value = payload->buf32[0];
        if (R[u.bits.rs2]) {
            res = R[u.bits.rs1] % R[u.bits.rs2];
        } else {
            res = R[u.bits.rs1];
        }
        icpu_->setReg(u.bits.rd, res);
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
        int64_t res;
        u.value = payload->buf32[0];
        int32_t a1 = static_cast<int32_t>(R[u.bits.rs1]);
        int32_t a2 = static_cast<int32_t>(R[u.bits.rs2]);
        if (a2) {
            // To avoid integer overflow exception on x86 use int64_t
            res = static_cast<int64_t>(a1) % static_cast<int64_t>(a2);
        } else {
            res = a1;
        }
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
        return 4;
    }
};

class REMUW : public RiscvInstruction {
public:
    REMUW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "REMUW", "0000001??????????111?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        uint32_t a1 = static_cast<uint32_t>(R[u.bits.rs1]);
        uint32_t a2 = static_cast<uint32_t>(R[u.bits.rs2]);
        int32_t resw;
        int64_t res;
        u.value = payload->buf32[0];
        if (a2) {
            resw = static_cast<int32_t>(a1 % a2);
        } else {
            resw = a1;
        }
        res = static_cast<int64_t>(resw);
        icpu_->setReg(u.bits.rd, static_cast<uint64_t>(res));
        return 4;
    }
};

void CpuRiver_Functional::addIsaExtensionM() {
    addSupportedInstruction(new DIV(this));
    addSupportedInstruction(new DIVU(this));
    addSupportedInstruction(new DIVUW(this));
    addSupportedInstruction(new DIVW(this));
    addSupportedInstruction(new MUL(this));
    addSupportedInstruction(new MULH(this));
    addSupportedInstruction(new MULHSU(this));
    addSupportedInstruction(new MULHU(this));
    addSupportedInstruction(new MULW(this));
    addSupportedInstruction(new REM(this));
    addSupportedInstruction(new REMU(this));
    addSupportedInstruction(new REMW(this));
    addSupportedInstruction(new REMUW(this));

    uint64_t isa = portCSR_.read(CSR_misa).val;
    portCSR_.write(CSR_misa, isa | (1LL << ('M' - 'A')));
}

}  // namespace debugger
