/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "ddr.h"

namespace debugger {

DDR::DDR(const char *name) : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    mem_ = 0;
}

DDR::~DDR() {
    if (mem_) {
        delete [] mem_;
    }
}

void DDR::postinitService() {
    mem_  = new char [getLength()];
}

ETransStatus DDR::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    if (trans->action == MemAction_Read) {
        memcpy(trans->rpayload.b8, &mem_[off], trans->xsize);
    } else {
        memcpy(&mem_[off], trans->wpayload.b8, trans->xsize);
    }
    return TRANS_OK;
}

}  // namespace debugger
