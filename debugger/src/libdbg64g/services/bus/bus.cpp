/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System Bus class declaration (AMBA or whatever).
 */

#include <api_core.h>
#include "bus.h"
#include "coreservices/icpugen.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(Bus)

Bus::Bus(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("DSU", &dsu_);
    RISCV_mutex_init(&mutexBAccess_);
    RISCV_mutex_init(&mutexNBAccess_);
}

Bus::~Bus() {
    RISCV_mutex_destroy(&mutexBAccess_);
    RISCV_mutex_destroy(&mutexNBAccess_);
}

void Bus::postinitService() {
    if (dsu_.is_string()) {
        idsu_ = static_cast<IDsuGeneric *>(
            RISCV_get_service_iface(dsu_.to_string(), IFACE_DSU_GENERIC));
        if (!idsu_) {
            RISCV_debug("Can't find IDsuGeneric interface %s",
                        dsu_.to_string());
        }
    }

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
                    dev[0u].to_string(), dev[1].to_string());
                continue;
            }
            map(imem);
        }
    }

    AttributeType clks;
    RISCV_get_clock_services(&clks);
    if (clks.size()) {
        iclk0_ = static_cast<IClock *>(clks[0u].to_iface());
    } else {
        RISCV_error("CPUs not found", NULL);
    }
}

ETransStatus Bus::b_transport(Axi4TransactionType *trans) {
    ETransStatus ret = TRANS_OK;
    uint32_t sz;
    IMemoryOperation *memdev = 0;

    RISCV_mutex_lock(&mutexBAccess_);

    getMapedDevice(trans, &memdev, &sz);

    if (memdev == 0) {
        RISCV_error("[%" RV_PRI64 "d] Blocking request to unmapped address "
                    "%08" RV_PRI64 "x", iclk0_->getStepCounter(), trans->addr);
        memset(trans->rpayload.b8, 0xFF, trans->xsize);
        ret = TRANS_ERROR;
    } else {
        memdev->b_transport(trans);
        RISCV_debug("[%08" RV_PRI64 "x] => [%08x %08x]",
            trans->addr,
            trans->rpayload.b32[1], trans->rpayload.b32[0]);
    }

    // Update Bus utilization counters:
    if (idsu_) {
        if (trans->action == MemAction_Read) {
            idsu_->incrementRdAccess(trans->source_idx);
        } else if (trans->action == MemAction_Write) {
            idsu_->incrementWrAccess(trans->source_idx);
        }
    }
    RISCV_mutex_unlock(&mutexBAccess_);
    return ret;
}

ETransStatus Bus::nb_transport(Axi4TransactionType *trans,
                               IAxi4NbResponse *cb) {
    ETransStatus ret = TRANS_OK;
    IMemoryOperation *memdev = 0;
    uint32_t sz;

    RISCV_mutex_lock(&mutexNBAccess_);

    getMapedDevice(trans, &memdev, &sz);

    if (memdev == 0) {
        RISCV_error("[%" RV_PRI64 "d] Non-blocking request to unmapped address "
                    "%08" RV_PRI64 "x", iclk0_->getStepCounter(), trans->addr);
        memset(trans->rpayload.b8, 0xFF, trans->xsize);
        trans->response = MemResp_Error;
        cb->nb_response(trans);
        ret = TRANS_ERROR;
    } else {
        memdev->nb_transport(trans, cb);
        RISCV_debug("Non-blocking request to [%08" RV_PRI64 "x]",
                    trans->addr);
    }

    // Update Bus utilization counters:
    if (idsu_) {
        if (trans->action == MemAction_Read) {
            idsu_->incrementRdAccess(trans->source_idx);
        } else if (trans->action == MemAction_Write) {
            idsu_->incrementWrAccess(trans->source_idx);
        }
    }
    RISCV_mutex_unlock(&mutexNBAccess_);
    return ret;
}

void Bus::getMapedDevice(Axi4TransactionType *trans,
                         IMemoryOperation **pdev, uint32_t *sz) {
    IMemoryOperation *imem;
    uint64_t bar, barsz;
    *pdev = 0;
    *sz = 0;
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
