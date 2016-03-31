/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
* @brief      GNSS stub module functional model.
 */

#include "api_core.h"
#include "types_amba.h"
#include "gnss_stub.h"

namespace debugger {

GNSSStub::GNSSStub(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("IrqControl", &irqctrl_);
    registerAttribute("ClkSource", &clksrc_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
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

void GNSSStub::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((payload->addr - getBaseAddress()) & mask);
    if (payload->rw) {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            if (((payload->wstrb >> 4*i) & 0xf) == 0) {
                continue;
            }
            regs_.tmr.rw_MsLength = payload->wpayload[i];
            if (regs_.tmr.rw_MsLength) {
                iclk_->registerStepCallback(
                    static_cast<IClockListener *>(this), 
                    iclk_->getStepCounter() + regs_.tmr.rw_MsLength);

            }
            if ((off + 4*i) == OFFSET(&regs_.tmr.rw_MsLength)) {
                RISCV_info("Set rw_MsLength = %d", regs_.tmr.rw_MsLength);
            }
        }
    } else {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            payload->rpayload[i] = 0;
            if ((off + 4*i) == OFFSET(&regs_.tmr.rw_MsLength)) {
                payload->rpayload[i] = regs_.tmr.rw_MsLength;
                RISCV_info("Get rw_MsLength = %d", regs_.tmr.rw_MsLength);
            }
        }
    }
}

void GNSSStub::stepCallback(uint64_t t) {
    iwire_->riseLine();
    iwire_->lowerLine();
    if (regs_.tmr.rw_MsLength) {
        iclk_->registerStepCallback(static_cast<IClockListener *>(this),
                                    t + regs_.tmr.rw_MsLength);
    }
}

}  // namespace debugger

