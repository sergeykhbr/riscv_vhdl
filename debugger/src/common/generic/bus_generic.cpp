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
#include "coreservices/icpugen.h"
#include "debug/dsumap.h"

namespace debugger {

BusGeneric::BusGeneric(const char *name) : IService(name),
    IHap(HAP_ConfigDone),
    busUtil_(static_cast<IService *>(this), "bus_util",
            DSUREG(ulocal.v.bus_util[0]),
            sizeof(DsuMapType::local_regs_type::\
                   local_region_type::mst_bus_util_type)) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("DefaultSlave", &defaultSlave_);
    registerAttribute("UseHash", &useHash_);
    RISCV_mutex_init(&mutexBAccess_);
    RISCV_mutex_init(&mutexNBAccess_);
    RISCV_register_hap(static_cast<IHap *>(this));
    busUtil_.setPriority(10);     // Overmap DSU registers
    imaphash_ = 0;
    idefmem_ = 0;
}

BusGeneric::~BusGeneric() {
    RISCV_mutex_destroy(&mutexBAccess_);
    RISCV_mutex_destroy(&mutexNBAccess_);
}

void BusGeneric::postinitService() {
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

    if (defaultSlave_.is_string()) {
        idefmem_ = static_cast<IMemoryOperation *>(RISCV_get_service_iface(
                    defaultSlave_.to_string(), IFACE_MEMORY_OPERATION));
        if (idefmem_ == 0) {
            RISCV_error("Can't find default slave device %s",
                        defaultSlave_.to_string());
        }
    } else if (defaultSlave_.is_list() && defaultSlave_.size() == 2) {
        const AttributeType &devname = defaultSlave_[0u];
        const AttributeType &portname = defaultSlave_[1];
        idefmem_ = static_cast<IMemoryOperation *>(
            RISCV_get_service_port_iface(devname.to_string(),
                                         portname.to_string(),
                                         IFACE_MEMORY_OPERATION));
        if (idefmem_ == 0) {
            RISCV_error("Can't find default slave device %s:%s", 
                devname.to_string(), portname.to_string());
        }
    }
}

/** We need correctly mapped device list to compute hash, postinit
    doesn't allow to guarantee order of initialization. */
void BusGeneric::hapTriggered(IFace *isrc,
                                 EHapType type,
                                 const char *descr) {
    if (!useHash_.to_bool()) {
        return;
    }

    IMemoryOperation *imem;
    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        maphash(imem);
    }
}

ETransStatus BusGeneric::b_transport(Axi4TransactionType *trans) {
    ETransStatus ret = TRANS_OK;
    uint32_t sz;
    IMemoryOperation *memdev = 0;

    RISCV_mutex_lock(&mutexBAccess_);

    if (itranslator_) {
        itranslator_->translate(trans);
    }

    getMapedDevice(trans, &memdev, &sz);

    if (memdev == 0) {
        RISCV_error("Blocking request to unmapped address "
                    "%08" RV_PRI64 "x", trans->addr);
        if (idefmem_) {
            idefmem_->b_transport(trans);
        } else {
            memset(trans->rpayload.b8, 0xFF, trans->xsize);
        }
        ret = TRANS_ERROR;
    } else {
        memdev->b_transport(trans);
        RISCV_debug("[%08" RV_PRI64 "x] => [%08x %08x]",
            trans->addr,
            trans->rpayload.b32[1], trans->rpayload.b32[0]);
    }

    // Update Bus utilization counters:
    if (trans->source_idx >= 0 && trans->source_idx < 8) {
        if (trans->action == MemAction_Read) {
            busUtil_.getpR64()[2*trans->source_idx + 1]++;
        } else if (trans->action == MemAction_Write) {
            busUtil_.getpR64()[2*trans->source_idx]++;
        }
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

    if (itranslator_) {
        itranslator_->translate(trans);
    }
    getMapedDevice(trans, &memdev, &sz);

    if (memdev == 0) {
        RISCV_error("Non-blocking request to unmapped address "
                    "%08" RV_PRI64 "x", trans->addr);
        if (idefmem_) {
            idefmem_->nb_transport(trans, cb);
        } else {
            memset(trans->rpayload.b8, 0xFF, trans->xsize);
            trans->response = MemResp_Error;
            cb->nb_response(trans);
        }
        ret = TRANS_ERROR;
    } else {
        memdev->nb_transport(trans, cb);
        RISCV_debug("Non-blocking request to [%08" RV_PRI64 "x]",
                    trans->addr);
    }

    // Update Bus utilization counters:
    if (trans->source_idx >= 0 && trans->source_idx < 8) {
        if (trans->action == MemAction_Read) {
            busUtil_.getpR64()[2*trans->source_idx + 1]++;
        } else if (trans->action == MemAction_Write) {
            busUtil_.getpR64()[2*trans->source_idx]++;
        }
    }
    RISCV_mutex_unlock(&mutexNBAccess_);
    return ret;
}

void BusGeneric::getMapedDevice(Axi4TransactionType *trans,
                         IMemoryOperation **pdev, uint32_t *sz) {
    IMemoryOperation *imem;
    *pdev = 0;
    *sz = 0;
    if (useHash_.to_bool()) {
        imem = getHashedDevice(trans->addr);
        if (imem) {
            *pdev = imem;
            return;
        }
    }

    uint64_t bar, barsz;
    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        bar = imem->getBaseAddress();
        barsz = imem->getLength();
        if (bar <= trans->addr && trans->addr < (bar + barsz)) {
            if (!(*pdev) || imem->getPriority() > (*pdev)->getPriority()) {
                *pdev = imem;
            }
        }
    }
}

}  // namespace debugger
