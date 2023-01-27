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

#include "cmd_br_arm7.h"

namespace debugger {

/*CmdBrArm::CmdBrArm(uint64_t dmibar, ITap *tap) : CmdBrGeneric(dmibar, tap) {
}

void CmdBrArm::getSwBreakpointInstr(Reg64Type *instr, uint32_t *len) {
    //instr->buf32[0] = 0xFFDEFFE7;  // SWI
    //*len = 4;
    instr->buf32[0] = 0xBE00;  // SWI
    *len = 2;
}
*/
}  // namespace debugger
