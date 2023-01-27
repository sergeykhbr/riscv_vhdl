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

#include <api_core.h>
#include "bus_generic.h"

namespace debugger {

BusGeneric::BusGeneric(const char *name) : IService(name),
    IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("AddrWidth", &addrWidth_);
    RISCV_mutex_init(&mutexBAccess_);
    RISCV_mutex_init(&mutexNBAccess_);
    RISCV_register_hap(static_cast<IHap *>(this));
    imaphash_ = 0;

    memset(imemtbl_, 0, sizeof(imemtbl_));
    for (int i = 0; i < HASH_TBL_SIZE; i++) {
        imemtbl_[i].devlist.make_list(0);
    }
    addrWidth_.make_int64(39);      // 39-bits address width for FU740
}

BusGeneric::~BusGeneric() {
    RISCV_mutex_destroy(&mutexBAccess_);
    RISCV_mutex_destroy(&mutexNBAccess_);
    if (imaphash_) {
        delete [] imaphash_;
    }
}

void BusGeneric::postinitService() {
    ADDR_MASK_ = (1ull << addrWidth_.to_int()) - 1;
    HASH_MASK_ = (1ull << HASH_ADDR_WIDTH) - 1;

    HASH_LVL1_OFFSET_ = addrWidth_.to_int() - HASH_ADDR_WIDTH;
    HASH_LVL2_OFFSET_ = addrWidth_.to_int() - 2*HASH_ADDR_WIDTH;

    IMemoryOperation *imem;
    for (unsigned i = 0; i < listMap_.size(); i++) {
        const AttributeType &dev = listMap_[i];
        if (dev.is_string()) {
            imem = static_cast<IMemoryOperation *>(RISCV_get_service_iface(
                    dev.to_string(), IFACE_MEMORY_OPERATION));
            if (imem == 0) {
                RISCV_error("Can't find slave device %s", dev.to_string());
                continue;
            }
            map(imem);
        } else if (dev.is_list() && dev.size() == 2) {
            const AttributeType &devname = dev[0u];
            const AttributeType &portname = dev[1];
            imem = static_cast<IMemoryOperation *>(
                RISCV_get_service_port_iface(devname.to_string(),
                                             portname.to_string(),
                                             IFACE_MEMORY_OPERATION));
            if (imem == 0) {
                RISCV_error("Can't find slave device %s:%s", 
                    devname.to_string(), portname.to_string());
                continue;
            }
            map(imem);
        }
    }
}

/** We need correctly mapped device list to compute hash, postinit
    doesn't allow to guarantee order of initialization. */
void BusGeneric::hapTriggered(EHapType type,
                              uint64_t param,
                              const char *descr) {
    RISCV_mutex_lock(&mutexNBAccess_);
    RISCV_mutex_lock(&mutexBAccess_);
    maphash();
    RISCV_mutex_unlock(&mutexBAccess_);
    RISCV_mutex_unlock(&mutexNBAccess_);
}

ETransStatus BusGeneric::b_transport(Axi4TransactionType *trans) {
    ETransStatus ret = TRANS_OK;
    uint32_t sz;
    IMemoryOperation *memdev = 0;

    RISCV_mutex_lock(&mutexBAccess_);

    getMapedDevice(trans, &memdev, &sz);

    if (memdev == 0) {
        RISCV_error("Blocking request to unmapped address "
                    "%08" RV_PRI64 "x", trans->addr);
        memset(trans->rpayload.b8, 0xFF, trans->xsize);
        ret = TRANS_ERROR;
    } else {
        memdev->b_transport(trans);
        RISCV_debug("[%08" RV_PRI64 "x] => [%08x %08x]",
            trans->addr,
            trans->rpayload.b32[1], trans->rpayload.b32[0]);
    }
    RISCV_mutex_unlock(&mutexBAccess_);
    return ret;
}

ETransStatus BusGeneric::nb_transport(Axi4TransactionType *trans,
                               IAxi4NbResponse *cb) {
    ETransStatus ret = TRANS_OK;
    IMemoryOperation *memdev = 0;
    uint32_t sz;

    RISCV_mutex_lock(&mutexNBAccess_);

    getMapedDevice(trans, &memdev, &sz);

    if (memdev == 0) {
        RISCV_error("Non-blocking request from %d to unmapped address "
                    "%08" RV_PRI64 "x", trans->source_idx, trans->addr);
        memset(trans->rpayload.b8, 0xFF, trans->xsize);
        trans->response = MemResp_Error;
        cb->nb_response(trans);
        ret = TRANS_ERROR;
    } else {
        memdev->nb_transport(trans, cb);
        RISCV_debug("Non-blocking request to [%08" RV_PRI64 "x]",
                    trans->addr);
    }
    RISCV_mutex_unlock(&mutexNBAccess_);
    return ret;
}

void BusGeneric::getMapedDevice(Axi4TransactionType *trans,
                         IMemoryOperation **pdev, uint32_t *sz) {
    IMemoryOperation *imem;
    uint64_t bar, barsz;
    *pdev = 0;
    *sz = 0;

    uint64_t hashidx = (trans->addr & ADDR_MASK_) >> HASH_LVL1_OFFSET_;
    HashTableItemType &item = imemtbl_[hashidx];
    if (item.idev) {
        *pdev = item.idev;
    } else if (item.nxtlvlena) {
        for (unsigned i = 0; i < item.devlist.size(); i++) {
            imem = static_cast<IMemoryOperation *>(item.devlist[i].to_iface());
            bar = imem->getBaseAddress();
            barsz = imem->getLength();
            if (bar <= trans->addr && trans->addr < (bar + barsz)) {
                if (!(*pdev) || imem->getPriority() > (*pdev)->getPriority()) {
                    *pdev = imem;
                }
            }
        }
    }
}

void BusGeneric::maphash() {
    IMemoryOperation *imem;
    uint64_t first, last;
    uint64_t bar;
    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        bar = imem->getBaseAddress();
        first = (bar >> HASH_LVL1_OFFSET_) & HASH_MASK_;
        last = ((bar + imem->getLength()) >> HASH_LVL1_OFFSET_) & HASH_MASK_;

        for (uint64_t n = first; n <= last; n++) {
            HashTableItemType &item = imemtbl_[n];
            if (!item.nxtlvlena && !item.idev) {
                item.idev = imem;
            } else if (item.idev) {
                item.nxtlvlena = true;
                item.devlist.new_list_item().make_iface(item.idev);
                item.devlist.new_list_item().make_iface(imem);
                item.idev = 0;
            } else {
                item.devlist.new_list_item().make_iface(imem);
            }
        }
    }
}

}  // namespace debugger
