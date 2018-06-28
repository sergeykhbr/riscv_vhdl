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

#include "api_utils.h"
#include "arm-isa.h"
#include "instructions.h"
#include "cpu_arm7_func.h"

namespace debugger {

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

IFace *ArmInstruction::getInterface(const char *name) {
    return icpu_->getInterface(name);
}

int ArmInstruction::exec(Reg64Type *payload) {
#if 1
    if (icpu_->getPC() == 0xf24) {
        bool st = true;
    }
#endif
    if (check_cond(payload->buf32[0] >> 28)) {
        return exec_checked(payload);
    }
    return 4;
}

bool ArmInstruction::check_cond(uint32_t cond) {
    switch (cond) {
    case Cond_EQ:
        return icpu_->getZ() == 1;
    case Cond_NE:
        return icpu_->getZ() == 0;
    case Cond_CS:
        return icpu_->getC() == 1;
    case Cond_CC:
        return icpu_->getC() == 0;
    case Cond_MI:
        return icpu_->getN() == 1;
    case Cond_PL:
        return icpu_->getN() == 0;
    case Cond_VS:
        return icpu_->getV() == 1;
    case Cond_VC:
        return icpu_->getV() == 0;
    case Cond_HI:
        return icpu_->getC() && !icpu_->getZ();
    case Cond_LS:
        return !icpu_->getC() || icpu_->getZ();
    case Cond_GE:
        return !(icpu_->getN() ^ icpu_->getV());
    case Cond_LT:
        return (icpu_->getN() ^ icpu_->getV()) == 1;
    case Cond_GT:
        return !icpu_->getZ() && !(icpu_->getN() ^ icpu_->getV());
    case Cond_LE:
        return icpu_->getZ() || (icpu_->getN() ^ icpu_->getV());
    default:;
        return true;
    }
}

uint32_t ArmInstruction::shift12(DataProcessingType::reg_bits_type instr,
                                uint32_t reg, uint64_t Rs) {
    uint32_t ret = reg;
    uint32_t shift = instr.shift;
    if (instr.sh_sel) {
        shift = static_cast<uint32_t>(Rs & 0x1f);
    }
    switch (instr.sh_type) {
    case 0:     // logical left
        ret <<= shift;
        break;
    case 1:     // logical right
        ret >>= shift;
        break;
    case 2:     // arith. right
        ret =
            static_cast<uint32_t>((static_cast<int>(ret) >> shift));
        break;
    case 3:     // rotate right
        ret = (ret >> shift) | (ret << (32 - shift));
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
