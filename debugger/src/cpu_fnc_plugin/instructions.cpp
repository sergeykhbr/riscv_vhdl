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
#include "riscv-isa.h"
#include "instructions.h"
#include "cpu_riscv_func.h"

namespace debugger {

RiscvInstruction::RiscvInstruction(CpuRiver_Functional *icpu, const char *name,
                                    const char *bits) {
    icpu_ = icpu;
    R = icpu->getpRegs();
    RF = &icpu->getpRegs()[ICpuRiscV::RegFpu_Offset];
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

}  // namespace debugger
