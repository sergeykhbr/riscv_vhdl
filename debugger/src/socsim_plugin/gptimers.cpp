/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      General Purpose Timers model.
 */

#include "api_core.h"
#include "gptimers.h"

namespace debugger {

GPTimers::GPTimers(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
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

GPTimers::~GPTimers() {
}

void GPTimers::postinitService() {
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

void GPTimers::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask) / 4;
    trans->response = MemResp_Valid;
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            if ((trans->wstrb & (0xf << 4*i)) == 0) {
                continue;
            }
            switch (off + i) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                regs_.pending = trans->wpayload.b32[i];
                RISCV_info("Set pending = %08x", trans->wpayload.b32[i]);
                break;
            case 16 + 0:
                regs_.timer[0].control = trans->wpayload.b32[i];
                if (regs_.timer[0].control & TIMER_CONTROL_ENA) {
                    iclk_->registerStepCallback(
                        static_cast<IClockListener *>(this), 
                        iclk_->getStepCounter() + regs_.timer[0].init_value);
                }
                RISCV_info("Set [0].control = %08x", trans->wpayload.b32[i]);
                break;
            case 16 + 2:
                regs_.timer[0].cur_value &= ~0xFFFFFFFFLL;
                regs_.timer[0].cur_value |= trans->wpayload.b32[i];
                RISCV_info("Set cur_value[31:0] = %x", trans->wpayload.b32[i]);
                break;
            case 16 + 3:
                regs_.timer[0].cur_value &= ~0xFFFFFFFF00000000LL;
                regs_.timer[0].cur_value |= 
                    (static_cast<uint64_t>(trans->wpayload.b32[i]) << 32);
                RISCV_info("Set cur_value[63:32] = %x",
                            trans->wpayload.b32[i]);
                break;
            case 16 + 4:
                regs_.timer[0].init_value &= ~0xFFFFFFFFLL;
                regs_.timer[0].init_value |= trans->wpayload.b32[i];
                RISCV_info("Set init_value[31:0] = %x",
                            trans->wpayload.b32[i]);
                break;
            case 16 + 5:
                regs_.timer[0].init_value &= ~0xFFFFFFFF00000000LL;
                regs_.timer[0].init_value |= 
                    (static_cast<uint64_t>(trans->wpayload.b32[i]) << 32);
                RISCV_info("Set init_value[63:32] = %x",
                            trans->wpayload.b32[i]);
                break;
            default:;
            }
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            switch (off + i) {
            case 0:
                regs_.highcnt = iclk_->getStepCounter();
                trans->rpayload.b32[i] = static_cast<uint32_t>(regs_.highcnt);
                break;
            case 1:
                regs_.highcnt = iclk_->getStepCounter();
                trans->rpayload.b32[i] =
                    static_cast<uint32_t>(regs_.highcnt >> 32);
                break;
            case 16 + 0:
                trans->rpayload.b32[i] = regs_.timer[0].control;
                RISCV_info("Get [0].control = %08x", trans->rpayload.b32[i]);
                break;
            default:
                trans->rpayload.b32[i] = ~0;
            }
        }
    }
}

void GPTimers::stepCallback(uint64_t t) {
    iwire_->raiseLine(irqLine_.to_int());
    if (regs_.timer[0].control & TIMER_CONTROL_ENA) {
        iclk_->registerStepCallback(static_cast<IClockListener *>(this),
                                    t + regs_.timer[0].init_value);
    }
}


}  // namespace debugger

