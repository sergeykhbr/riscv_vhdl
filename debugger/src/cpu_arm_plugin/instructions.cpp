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
#include "instructions.h"
#include "cpu_arm7_func.h"

namespace debugger {

bool check_cond(CpuCortex_Functional *icpu, uint32_t cond) {
    switch (cond) {
    case Cond_EQ:
        return icpu->getZ() == 1;
    case Cond_NE:
        return icpu->getZ() == 0;
    case Cond_CS:
        return icpu->getC() == 1;
    case Cond_CC:
        return icpu->getC() == 0;
    case Cond_MI:
        return icpu->getN() == 1;
    case Cond_PL:
        return icpu->getN() == 0;
    case Cond_VS:
        return icpu->getV() == 1;
    case Cond_VC:
        return icpu->getV() == 0;
    case Cond_HI:
        return icpu->getC() && !icpu->getZ();
    case Cond_LS:
        return !icpu->getC() || icpu->getZ();
    case Cond_GE:
        return !(icpu->getN() ^ icpu->getV());
    case Cond_LT:
        return (icpu->getN() ^ icpu->getV()) == 1;
    case Cond_GT:
        return !icpu->getZ() && !(icpu->getN() ^ icpu->getV());
    case Cond_LE:
        return icpu->getZ() || (icpu->getN() ^ icpu->getV());
    default:;
        return true;
    }
}

ArmInstruction::ArmInstruction(CpuCortex_Functional *icpu, const char *name,
                                    const char *bits) {
    icpu_ = icpu;
    R = icpu->getpRegs();
    name_.make_string(name);
    mask_ = 0;
    opcode_ = 0;
    for (int i = 0; i < 32; i++) {
        switch (bits[i]) {
        case '0':
            break;
        case '1':
            opcode_ |= (1 << (31 - i));
            break;
        case '?':
            mask_ |= (1 << (31 - i));
            break;
        default:;
        }
    }
    mask_ ^= ~0;
}

T1Instruction::T1Instruction(CpuCortex_Functional *icpu,
                                   const char *name) {
    icpu_ = icpu;
    R = icpu->getpRegs();
    name_.make_string(name);
    mask_ = 0;
    opcode_ = 0;
    mask_ ^= ~0;
}

bool T1Instruction::ConditionPassed() {
    if (icpu_->InITBlock()) {
        return check_cond(icpu_, icpu_->ITBlockCondition());
    }
    return true;
}

uint32_t T1Instruction::AddWithCarry(uint32_t x, uint32_t y, uint32_t carry_in,
                                    uint32_t *overflow, uint32_t *carry_out) {
    uint64_t unsigned_sum = static_cast<uint64_t>(x) +
                            static_cast<uint64_t>(y) + carry_in;
    int64_t signed_sum = static_cast<int64_t>(static_cast<int>(x)) +
                         static_cast<int64_t>(static_cast<int>(y)) + carry_in;
    uint32_t result = static_cast<uint32_t>(unsigned_sum);

    *carry_out = 0;
    if (static_cast<uint64_t>(result) != unsigned_sum) {
        *carry_out = 1;
    }
    *overflow = 0;
    if (static_cast<int64_t>(static_cast<int32_t>(result)) != signed_sum) {
        *overflow = 1;
    }
    return result;
}

SRType T1Instruction::DecodeImmShift(uint32_t type, uint32_t imm5, uint32_t *shift_n) {
    SRType ret = SRType_None;
    switch (type) {
    case 0:
        ret = SRType_LSL;
        *shift_n = imm5;
        break;
    case 1:
        ret = SRType_LSR;
        *shift_n = imm5;
        if (imm5 == 0) {
            *shift_n = 32;
        }
        break;
    case 2:
        ret = SRType_ASR;
        *shift_n = imm5;
        if (imm5 == 0) {
            *shift_n = 32;
        }
        break;
    case 3:
        if (imm5 == 0) {
            ret = SRType_RRX;
            *shift_n = 1;
        } else {
            ret = SRType_ROR;
            *shift_n = imm5;
        }
        break;
    default:;
    }
    return ret;
}

SRType T1Instruction::DecodeRegShift(uint32_t type) {
    static const SRType shift_select[4] = {
        SRType_LSL,
        SRType_LSR,
        SRType_ASR,
        SRType_ROR
    };
    return shift_select[type & 0x3];
}

uint32_t T1Instruction::Shift(uint32_t value, SRType type, int amount, uint32_t carry_in) {
    uint32_t carry_out;
    return Shift_C(value, type, amount, carry_in, &carry_out);
}

uint32_t T1Instruction::Shift_C(uint32_t value, SRType type, int amount,
                                uint32_t carry_in, uint32_t *carry_out) {
    uint32_t result = 0;
    switch (type) {
    case SRType_None:   // Identical to SRType_LSL with amount == 0
        result = value;
        *carry_out = carry_in;
        break;
    case SRType_LSL:
        if (amount == 0) {
            result = value;
            *carry_out = carry_in;
        } else {
            result = LSL_C(value, amount, carry_out);
        }
        break;
    case SRType_LSR:
        result = LSR_C(value, amount, carry_out);
        break;
    case SRType_ASR:
        result = ASR_C(value, amount, carry_out);
        break;
    case SRType_ROR:
        result = ROR_C(value, amount, carry_out);
        break;
    case SRType_RRX:
        result = RRX_C(value, carry_in, carry_out);
        break;
    }
    return result;
}

uint32_t T1Instruction::LSL_C(uint32_t x, int n, uint32_t *carry_out) {
    if (n > 31) {
        return 0;
    }
    uint64_t extended_x = static_cast<uint64_t>(x) << n;
    *carry_out = static_cast<uint32_t>((extended_x >> 32) & 1);
    return static_cast<uint32_t>(extended_x);
}

uint32_t T1Instruction::LSR_C(uint32_t x, int n, uint32_t *carry_out) {
    if (n > 31) {
        return 0;
    }
    uint64_t extended_x = static_cast<uint64_t>(x);
    *carry_out = static_cast<uint32_t>((extended_x >> (n-1)) & 1);
    return static_cast<uint32_t>(extended_x >> n);
}

uint32_t T1Instruction::ASR_C(uint32_t x, int n, uint32_t *carry_out) {
    if (n > 31) {
        if (x & 0x80000000) {
            return ~0;
        } else {
            return 0;
        }
    }
    int64_t extended_x = static_cast<int64_t>(static_cast<int32_t>(x));
    *carry_out = static_cast<uint32_t>((extended_x >> (n-1)) & 1);
    return static_cast<uint32_t>(extended_x >> n);
}

uint32_t T1Instruction::ROR_C(uint32_t x, int n, uint32_t *carry_out) {
    int m = n % 32;
    uint32_t result;
    uint32_t c_out;
    if (m == 0) {
        result = x;
    } else {
        result = LSR_C(x, m, &c_out) | LSL_C(x, 32-m, &c_out);
    }
    *carry_out = (result >> 31) & 1;
    return result;
}

uint32_t T1Instruction::ROL_C(uint32_t x, int n, uint32_t *carry_out) {
    int m = n % 32;
    uint32_t result;
    uint32_t c_out;
    if (m == 0) {
        result = x;
    } else {
        result = LSL_C(x, m, &c_out) | LSR_C(x, 32-m, &c_out);
    }
    *carry_out = result & 1;
    return result;
}

uint32_t T1Instruction::RRX_C(uint32_t x, uint32_t carry_in,
                              uint32_t *carry_out) {
    uint32_t result = (carry_in << 31) | (x >> 1);
    *carry_out = x & 1;
    return result;
}

uint32_t T1Instruction::ThumbExpandImmWithC(uint32_t imm12,
                                            uint32_t *carry_out) {
    uint32_t imm32;
    uint32_t t8 = imm12 & 0xFF;
    if ((imm12 & 0xC00) == 0) {
        switch ((imm12 >> 8) & 0x3) {
        case 0:
            imm32 = t8;
            break;
        case 1:
            if ((imm12 & 0xFF) == 0) {
                RISCV_error("%s", "UNPREDICTABLE");
            }
            imm32 = (t8 << 16) | t8;
            break;
        case 2:
            if ((imm12 & 0xFF) == 0) {
                RISCV_error("%s", "UNPREDICTABLE");
            }
            imm32 = (t8 << 24) | (t8 << 8);
            break;
        case 3:
            if ((imm12 & 0xFF) == 0) {
                RISCV_error("%s", "UNPREDICTABLE");
            }
            imm32 = (t8 << 24) | (t8 << 16) | (t8 << 8) | t8;
            break;
        default:;
        }
        *carry_out = icpu_->getC();
    } else {
        uint32_t unrotated_value = (1 << 7) | (imm12 & 0x7F);
        imm32 = ROR_C(unrotated_value, (imm12 >> 7), carry_out);
    }
    return imm32;
}

void T1Instruction::BranchWritePC(uint32_t npc) {
    icpu_->setReg(Reg_pc, npc);
    icpu_->setBranch(npc);
}

void T1Instruction::BXWritePC(uint32_t npc) {
    if ((npc & 0xFFFFFF80) == 0xFFFFFF80) {
        icpu_->exitException(npc);
        return;
    }
    if (npc & 0x1) {
        icpu_->setInstrMode(THUMB_mode);
        npc &= ~0x1ul;
    } else {
        icpu_->setInstrMode(ARM_mode);
    }

    icpu_->setReg(Reg_pc, npc);
    icpu_->setBranch(npc);
}

void T1Instruction::ALUWritePC(uint32_t npc) {
    if (npc & 0x1) {
        icpu_->setInstrMode(THUMB_mode);
        npc &= ~0x1ul;
    } else {
        icpu_->setInstrMode(ARM_mode);
    }
    icpu_->setReg(Reg_pc, npc);
    icpu_->setBranch(npc);
}

void T1Instruction::LoadWritePC(uint32_t address) {
    uint32_t npc;
    trans_.action = MemAction_Read;
    trans_.addr = address;
    trans_.xsize = 4;
    trans_.wstrb = 0;
    icpu_->dma_memop(&trans_);

    npc = trans_.rpayload.b32[0];
    if ((npc & 0xFFFFFF80) == 0xFFFFFF80) {
        icpu_->exitException(npc);
        return;
    }
    if (npc & 0x1) {
        icpu_->setInstrMode(THUMB_mode);
        npc &= ~0x1ul;
    } else {
        icpu_->setInstrMode(ARM_mode);
    }
    icpu_->setReg(Reg_pc, npc);
    icpu_->setBranch(npc);
}

IFace *ArmInstruction::getInterface(const char *name) {
    return icpu_->getInterface(name);
}

IFace *T1Instruction::getInterface(const char *name) {
    return icpu_->getInterface(name);
}

int ArmInstruction::exec(Reg64Type *payload) {
    if (check_cond(icpu_, payload->buf32[0] >> 28)) {
        return exec_checked(payload);
    }
    return 4;
}

uint32_t ArmInstruction::shift12(DataProcessingType::reg_bits_type instr,
                                uint32_t reg, uint64_t Rs) {
    uint32_t ret = reg;
    uint32_t shift = instr.shift;
    bool imm5_zero = shift == 0;
    if (instr.sh_sel) {
        shift = static_cast<uint32_t>(Rs & 0xFF);
        imm5_zero = false;
    }
    switch (instr.sh_type) {
    case 0:     // logical left
        if (shift > 31) {
            ret = 0;
        } else {
            ret = ret << shift;
        }
        break;
    case 1:     // logical right
        if (imm5_zero || (shift > 31)) {
            ret = 0;
        } else {
            ret >>= shift;
        }
        break;
    case 2:     // arith. right
        if (imm5_zero || (shift > 31)) {
            ret = 0;
            if (ret & 0x80000000) {
                ret = ~ret;
            }
        } else {
            ret =
                static_cast<uint32_t>((static_cast<int>(ret) >> shift));
        }
        break;
    case 3:     // rotate right
        if (imm5_zero) {
            ret = (icpu_->getC() << 31) | (ret >> 1);
        } else {
            ret = (ret >> shift) | (ret << (32 - shift));
        }
        break;
    }
    return ret;
}

uint32_t ArmInstruction::imm12(DataProcessingType::imm_bits_type instr) {
    uint32_t rsh = 2*instr.rotate;
    uint32_t imm = (instr.imm >> rsh) | (instr.imm << (32 - rsh));
    return imm;
}

}  // namespace debugger
