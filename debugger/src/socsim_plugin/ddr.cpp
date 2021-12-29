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
    mem_.bid = 0;
    mem_.prv = 0;
    mem_.nxt = 0;
    memset(mem_.m, 0, sizeof(mem_.m));
}

DDR::~DDR() {
    MemBlockType *t = mem_.nxt;
    MemBlockType *t2;
    while (t) {
        t2 = t->nxt;
        delete t;
        t = t2;
    }
}

void DDR::postinitService() {
}

ETransStatus DDR::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    uint8_t *data = getpMem(off);
    if (trans->action == MemAction_Read) {
        memcpy(trans->rpayload.b8, data, trans->xsize);
    } else {
        memcpy(data, trans->wpayload.b8, trans->xsize);
    }
    return TRANS_OK;
}

uint8_t *DDR::getpMem(uint64_t addr) {
    MemBlockType *b = &mem_;
    uint64_t bid = addr >> 10;
    uint8_t *ret;
    while (b->nxt && bid > b->bid) {
        b = b->nxt;
    }

    if (b->bid < bid) {
        MemBlockType *pnew = new MemBlockType;
        pnew->bid = bid;
        memset(pnew->m, 0, sizeof(pnew->m));
        pnew->nxt = b->nxt;
        pnew->prv = b;
        b->nxt = pnew;
        if (pnew->nxt) {
            pnew->nxt->prv = pnew;
        }
        b = pnew;
    } else if (b->bid == bid) {
        // Do nothing
    } else {
        MemBlockType *pnew = new MemBlockType;
        pnew->bid = bid;
        memset(pnew->m, 0, sizeof(pnew->m));
        pnew->prv = b->prv;
        pnew->nxt = b;
        b->prv = pnew;
        if (pnew->prv) {
            pnew->prv->nxt = pnew;
        }
        b = pnew;
    }
    ret = &b->m[addr & 0x3FF];
    return ret;
}

}  // namespace debugger
