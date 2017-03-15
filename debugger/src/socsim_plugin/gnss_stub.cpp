/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
* @brief      GNSS stub module functional model.
 */

#include "api_core.h"
#include "gnss_stub.h"

namespace debugger {

GNSSStub::GNSSStub(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("IrqLine", &irqLine_);
    registerAttribute("IrqControl", &irqctrl_);
    registerAttribute("ClkSource", &clksrc_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    irqLine_.make_uint64(0);
    irqctrl_.make_string("");
    clksrc_.make_string("");

    memset(&regs_, 0, sizeof(regs_));
}

GNSSStub::~GNSSStub() {
}

void GNSSStub::postinitService() {
    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(clksrc_.to_string(), IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't find IClock interface %s", clksrc_.to_string());
    }

    iwire_ = static_cast<IWire *>(
        RISCV_get_service_iface(irqctrl_.to_string(), IFACE_WIRE));
    if (!iwire_) {
        RISCV_error("Can't find IWire interface %s", irqctrl_.to_string());
    }
}

void GNSSStub::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask);
    trans->response = MemResp_Valid;
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            if (((trans->wstrb >> 4*i) & 0xf) == 0) {
                continue;
            }
            if (regs_.tmr.rw_MsLength == 0 && trans->wpayload.b32[i] != 0) {
                iclk_->registerStepCallback(
                    static_cast<IClockListener *>(this), 
                    iclk_->getStepCounter() + trans->wpayload.b32[i]);
            }
            regs_.tmr.rw_MsLength = trans->wpayload.b32[i];
            if ((off + 4*i) == OFFSET(&regs_.tmr.rw_MsLength)) {
                RISCV_info("Set rw_MsLength = %d", regs_.tmr.rw_MsLength);
            }
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            trans->rpayload.b32[i] = 0;
            if ((off + 4*i) == OFFSET(&regs_.tmr.rw_MsLength)) {
                trans->rpayload.b32[i] = regs_.tmr.rw_MsLength;
                RISCV_info("Get rw_MsLength = %d", regs_.tmr.rw_MsLength);
            }
        }
    }
}

void GNSSStub::stepCallback(uint64_t t) {
    iwire_->raiseLine(irqLine_.to_int());
    if (regs_.tmr.rw_MsLength) {
        iclk_->registerStepCallback(static_cast<IClockListener *>(this),
                                    t + regs_.tmr.rw_MsLength);
    }
}

}  // namespace debugger

