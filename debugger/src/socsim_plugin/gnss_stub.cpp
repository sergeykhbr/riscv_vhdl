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
    registerAttribute("IrqControl", &irqctrl_);
    registerAttribute("ClkSource", &clksrc_);

    memset(&regs_, 0, sizeof(regs_));
    
    regs_.misc.GenericChanCfg = 12;       // [4:0] gps
    regs_.misc.GenericChanCfg |= 12 << 5; // [8:5] glo
    regs_.misc.GenericChanCfg |= 2 << 9;  // [10:9] sbas
    regs_.misc.GenericChanCfg |= 6 << 11; // [14:11] gal
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
        RISCV_get_service_port_iface(irqctrl_[0u].to_string(),
                                     irqctrl_[1].to_string(),
                                     IFACE_WIRE));
    if (!iwire_) {
        RISCV_error("Can't find IWire interface %s", irqctrl_[0u].to_string());
    }
}

ETransStatus GNSSStub::b_transport(Axi4TransactionType *trans) {
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
        uint8_t *m = reinterpret_cast<uint8_t *>(&regs_);
        memcpy(trans->rpayload.b8, &m[off], trans->xsize);
    }
    return TRANS_OK;
}

void GNSSStub::stepCallback(uint64_t t) {
    iwire_->raiseLine();
    if (regs_.tmr.rw_MsLength) {
        regs_.tmr.rw_tow++;
        regs_.tmr.rw_tod++;
        iclk_->registerStepCallback(static_cast<IClockListener *>(this),
                                    t + regs_.tmr.rw_MsLength);
    }
}

}  // namespace debugger

