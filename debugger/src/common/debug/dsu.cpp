/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug Support Unit (DSU) functional model.
 */

#include <api_core.h>
#include "dsu.h"

namespace debugger {

DSU::DSU(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IDbgNbResponse *>(this));
    registerAttribute("CPU", &cpu_);
    registerAttribute("Bus", &bus_);
    registerAttribute("Reset", &reset_);

    cpu_.make_string("");
    soft_reset_ = 0x0;  // Active LOW
}

DSU::~DSU() {
}

void DSU::postinitService() {
    icpu_ = static_cast<ICpuGeneric *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CPU_GENERIC));
    if (!icpu_) {
        RISCV_error("Can't find ICpuGeneric interface %s", cpu_.to_string());
    }
    ibus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(bus_.to_string(), IFACE_MEMORY_OPERATION));
    if (!ibus_) {
        RISCV_error("Can't find IBus interface %s", bus_.to_string());
    }
    irst_ = static_cast<IReset *>(
        RISCV_get_service_iface(reset_.to_string(), IFACE_RESET));
    if (!irst_) {
        RISCV_error("Can't find IReset interface %s", reset_.to_string());
    }
}

ETransStatus DSU::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off64 = (trans->addr - getBaseAddress()) & mask;
    if (!icpu_) {
        trans->response = MemResp_Error;
        return TRANS_ERROR;
    }
    uint64_t region = (off64 >> 15) & 0x3;

    if (region < 3) {
        RISCV_error("b_transport() to debug port NOT SUPPORTED", NULL);
        trans->response = MemResp_Error;
        return TRANS_ERROR;
    }

    if (trans->action == MemAction_Read) {
        readLocal(off64 & 0x7fff, trans);
    } else {
        writeLocal(off64 & 0x7fff, trans);
    }

    trans->response = MemResp_Valid;
    // @todo Memory mapped registers not related to debug port
    return TRANS_OK;
}

ETransStatus DSU::nb_transport(Axi4TransactionType *trans,
                               IAxi4NbResponse *cb) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off64 = (trans->addr - getBaseAddress()) & mask;
    if (!icpu_) {
        trans->response = MemResp_Error;
        cb->nb_response(trans);
        return TRANS_ERROR;
    }

    nb_trans_.p_axi_trans = trans;
    nb_trans_.iaxi_cb = cb;

    nb_trans_.dbg_trans.write = 0;
    nb_trans_.dbg_trans.bytes = trans->xsize;
    if (trans->action == MemAction_Write) {
        nb_trans_.dbg_trans.write = 1;
        nb_trans_.dbg_trans.wdata = trans->wpayload.b64[0];
    }

    ETransStatus ret = TRANS_OK;
    nb_trans_.dbg_trans.addr = off64 & 0x7FFF;
    nb_trans_.dbg_trans.region = (off64 >> 15) & 0x3;
    if (nb_trans_.dbg_trans.region == 3) {
        ret = b_transport(trans);
        cb->nb_response(trans);
    } else {
        icpu_->nb_transport_debug_port(&nb_trans_.dbg_trans, this);
    }
    return ret;
}

void DSU::nb_response_debug_port(DebugPortTransactionType *trans) {
    nb_trans_.p_axi_trans->response = MemResp_Valid;
    nb_trans_.p_axi_trans->rpayload.b64[0] = trans->rdata;
    nb_trans_.iaxi_cb->nb_response(nb_trans_.p_axi_trans);
}

void DSU::readLocal(uint64_t off, Axi4TransactionType *trans) {
    switch (off >> 3) {
    case 0:
        trans->rpayload.b64[0] = soft_reset_;
        break;
    case 8:
        //trans->rpayload.b64[0] = ibus_->bus_utilization()[0].w_cnt;
        break;
    case 9:
        //trans->rpayload.b64[0] = ibus_->bus_utilization()[0].r_cnt;
        break;
    case 12:
        //trans->rpayload.b64[0] = ibus_->bus_utilization()[2].w_cnt;
        break;
    case 13:
        //trans->rpayload.b64[0] = ibus_->bus_utilization()[2].r_cnt;
        break;
    default:
        trans->rpayload.b64[0] = 0;
    }
    if (trans->xsize == 4 && (off & 0x4) == 0x4) {
        trans->rpayload.b64[0] >>= 32;
    }
}

void DSU::writeLocal(uint64_t off, Axi4TransactionType *trans) {
    if (trans->xsize == 4) {
        if ((off & 0x4) == 0) {
            wdata64_ = trans->wpayload.b32[0];
            return;
        } else {
            wdata64_ |= static_cast<uint64_t>(trans->wpayload.b32[0]) << 32;
        }
    } else {
        wdata64_ = trans->wpayload.b64[0];
    }
    switch (off >> 3) {
    case 0: // soft reset
        if (wdata64_ & 0x1) {
            irst_->powerOnPressed();
        } else {
            irst_->powerOnReleased();
        }
        soft_reset_ = wdata64_;
        break;
    default:;
    }
}


}  // namespace debugger

