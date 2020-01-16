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
