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
#include "memlut.h"
#include <iostream>
#include <string.h>

namespace debugger {

MemoryLUT::MemoryLUT(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("MemTarget", &memTarget_);
    registerAttribute("MemOffset", &memOffset_);

    memTarget_.make_string("");
    memOffset_.make_uint64(0);
}

void MemoryLUT::postinitService() {
    itarget_ = static_cast<IMemoryOperation *>(RISCV_get_service_iface(
                            memTarget_.to_string(), IFACE_MEMORY_OPERATION));
    if (!itarget_) {
        RISCV_error("Can't find IMemoryOperation interface %s",
                    memTarget_.to_string());
    }
}

ETransStatus MemoryLUT::b_transport(Axi4TransactionType *trans) {
    if (!itarget_) {
        return TRANS_ERROR;
    }
    uint64_t off = trans->addr - getBaseAddress();
    trans->addr = memOffset_.to_uint64() + off;
    return itarget_->b_transport(trans);
}

}  // namespace debugger

