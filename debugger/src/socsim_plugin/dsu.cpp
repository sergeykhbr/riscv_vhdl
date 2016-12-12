/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug Support Unit (DSU) functional model.
 */

#include "api_core.h"
#include "dsu.h"

namespace debugger {

DSU::DSU(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IDbgNbResponse *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("CPU", &cpu_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    cpu_.make_string("");
}

DSU::~DSU() {
}

void DSU::postinitService() {
    icpu_ = static_cast<ICpuRiscV *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CPU_RISCV));
    if (!icpu_) {
        RISCV_error("Can't find ICpuRiscV interface %s", cpu_.to_string());
    }
}

void DSU::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off64 = (trans->addr - getBaseAddress()) & mask;
    if (!icpu_) {
        trans->response = MemResp_Error;
        return;
    }
    uint64_t region = (off64 >> 15) & 0x3;
    //uint64_t addr = (off64 >> 3) & 0xFFF;

    if (region < 3) {
        RISCV_error("b_transport() to debug port NOT SUPPORTED", NULL);
        trans->response = MemResp_Error;
        return;
    }
    trans->response = MemResp_Valid;
    // @todo Memory mapped registers not related to debug port
}

void DSU::nb_transport(Axi4TransactionType *trans, IAxi4NbResponse *cb) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off64 = (trans->addr - getBaseAddress()) & mask;
    if (!icpu_) {
        trans->response = MemResp_Error;
        cb->nb_response(trans);
        return;
    }

    nb_trans_.p_axi_trans = trans;
    nb_trans_.iaxi_cb = cb;

    bool skip = false;
    nb_trans_.dbg_trans.write = 0;
    if (trans->action == MemAction_Write) {
        nb_trans_.dbg_trans.write = 1;
        if (trans->xsize == 4) {
            shifter32_ = (trans->wpayload.b64[0] << 32) | (shifter32_ >> 32);
            nb_trans_.dbg_trans.wdata = shifter32_;
            if ((trans->addr & 0x4) == 0) {
                skip = true;
            }
        } else {
            nb_trans_.dbg_trans.wdata = trans->wpayload.b64[0];
        }
    }

    nb_trans_.dbg_trans.addr = (off64 >> 3) & 0xFFF;
    nb_trans_.dbg_trans.region = (off64 >> 15) & 0x3;
    if (nb_trans_.dbg_trans.region == 3) {
        // @todo Memory mapped registers not related to debug port
        trans->response = MemResp_Valid;
        trans->rpayload.b64[0] = 0;
        cb->nb_response(trans);
    } else if (skip) {
        trans->response = MemResp_Valid;
        trans->rpayload.b64[0] = 0;
        cb->nb_response(trans);
    } else {
        icpu_->nb_transport_debug_port(&nb_trans_.dbg_trans, this);
    }
}

void DSU::nb_response_debug_port(DebugPortTransactionType *trans) {
    nb_trans_.p_axi_trans->response = MemResp_Valid;
    if (nb_trans_.p_axi_trans->xsize == 4
        && (nb_trans_.p_axi_trans->addr & 0x4)) {
        nb_trans_.p_axi_trans->rpayload.b64[0] = trans->rdata >> 32;
    } else {
        nb_trans_.p_axi_trans->rpayload.b64[0] = trans->rdata;
    }
    nb_trans_.iaxi_cb->nb_response(nb_trans_.p_axi_trans);
}

}  // namespace debugger

