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

#include "cmd_br_riscv.h"

namespace debugger {

CmdBrRiscv::CmdBrRiscv(ITap *tap) : CmdBrGeneric(tap) {
}

void CmdBrRiscv::getSwBreakpointInstr(Reg64Type *instr) {
    if ((instr->val & 0x3) == 0x3) {
        instr->buf32[0] = 0x00100073;  // EBREAK instruction
    } else {
        instr->buf16[0] = 0x9002;      // C.EBREAK instruction
    }
}

}  // namespace debugger
