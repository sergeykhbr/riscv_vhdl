/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System Bus class declaration (AMBA or whatever).
 */

#include "api_core.h"
#include "bus.h"
#include "coreservices/icpuriscv.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(Bus)

Bus::Bus(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IBus *>(this));
    registerAttribute("MapList", &listMap_);

    listMap_.make_list(0);
    imap_.make_list(0);
    RISCV_mutex_init(&mutexBAccess_);
    RISCV_mutex_init(&mutexNBAccess_);
    memset(info_, 0, sizeof(info_));
}

Bus::~Bus() {
    RISCV_mutex_destroy(&mutexBAccess_);
    RISCV_mutex_destroy(&mutexNBAccess_);
}

void Bus::postinitService() {
    IMemoryOperation *imem;
    for (unsigned i = 0; i < listMap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(RISCV_get_service_iface(
                listMap_[i].to_string(), IFACE_MEMORY_OPERATION));
        if (imem == 0) {
            RISCV_error("Can't find slave device %s", listMap_[i].to_string());
            continue;
        }
        map(imem);
    }

    AttributeType clks;
    RISCV_get_clock_services(&clks);
    if (clks.size()) {
        iclk0_ = static_cast<IClock *>(clks[0u].to_iface());
    } else {
        RISCV_error("CPUs not found", NULL);
    }
}

void Bus::map(IMemoryOperation *imemop) {
    AttributeType t1(imemop);
    imap_.add_to_list(&t1);
}

ETransStatus Bus::b_transport(Axi4TransactionType *trans) {
    IMemoryOperation *imem;
    bool unmapped = true;
    ETransStatus ret = TRANS_OK;

    RISCV_mutex_lock(&mutexBAccess_);

    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        if (imem->getBaseAddress() <= trans->addr
            && trans->addr < (imem->getBaseAddress() + imem->getLength())) {

            imem->b_transport(trans);
            unmapped = false;
            break;
            /// @todo Check memory overlapping
        }
    }

    if (unmapped) {
        RISCV_error("[%" RV_PRI64 "d] Read from unmapped address "
                    "%08" RV_PRI64 "x", iclk0_->getStepCounter(), trans->addr);
        memset(trans->rpayload.b8, 0xFF, trans->xsize);
        ret = TRANS_ERROR;
    } else {
        RISCV_debug("[%08" RV_PRI64 "x] => [%08x %08x]",
            trans->addr,
            trans->rpayload.b32[1], trans->rpayload.b32[0]);
    }

    // Update Bus utilization counters:
    if (trans->action == MemAction_Read) {
        info_[trans->source_idx].r_cnt++;
    } else if (trans->action == MemAction_Write) {
        info_[trans->source_idx].w_cnt++;
    }
    RISCV_mutex_unlock(&mutexBAccess_);
    return ret;
}

ETransStatus Bus::nb_transport(Axi4TransactionType *trans,
                               IAxi4NbResponse *cb) {
    IMemoryOperation *imem;
    bool unmapped = true;
    ETransStatus ret = TRANS_OK;

    RISCV_mutex_lock(&mutexNBAccess_);

    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        if (imem->getBaseAddress() <= trans->addr
            && trans->addr < (imem->getBaseAddress() + imem->getLength())) {

            imem->nb_transport(trans, cb);
            unmapped = false;
            break;
        }
    }

    if (unmapped) {
        RISCV_error("[%" RV_PRI64 "d] Non-blocking request to unmapped address "
                    "%08" RV_PRI64 "x", iclk0_->getStepCounter(), trans->addr);
        memset(trans->rpayload.b8, 0xFF, trans->xsize);
        trans->response = MemResp_Error;
        cb->nb_response(trans);
        ret = TRANS_ERROR;
    } else {
        RISCV_debug("Non-blocking request to [%08" RV_PRI64 "x]",
                    trans->addr);
    }

    // Update Bus utilization counters:
    if (trans->action == MemAction_Read) {
        info_[trans->source_idx].r_cnt++;
    } else if (trans->action == MemAction_Write) {
        info_[trans->source_idx].w_cnt++;
    }
    RISCV_mutex_unlock(&mutexNBAccess_);
    return ret;
}

BusUtilType *Bus::bus_utilization() {
    return info_;
}

}  // namespace debugger
