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

#include "dmifunc.h"
#include <riscv-isa.h>

namespace debugger {

DmiFunctional::DmiFunctional(IFace *parent) {
    iparent_ = parent;
}

DmiFunctional::~DmiFunctional() {
}


ETransStatus DmiFunctional::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();

    // Only 4-bytes requests:
    if (trans->action == MemAction_Read) {
        readreg(off >> 2);
    } else {
        if (trans->wstrb & 0x00FF) {
            writereg(off >> 2, trans->wpayload.b32[0]);
        } else if (trans->wstrb & 0xFF00) {
            writereg((off + 4) >> 2, trans->wpayload.b32[1]);
        }
    }
    return TRANS_OK;
}



void DmiFunctional::readreg(uint64_t idx) {
}

void DmiFunctional::writereg(uint64_t idx, uint32_t w32) {
}


}  // namespace debugger

