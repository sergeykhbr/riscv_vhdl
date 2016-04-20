/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interrupt controller functional model.
 */

#include "api_core.h"
#include "irqctrl.h"

namespace debugger {

IrqController::IrqController(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IWire *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("HostIO", &hostio_);
    registerAttribute("CSR_MIPI", &mipi_);

    baseAddress_.make_uint64(0);
    mipi_.make_uint64(0x783);
    length_.make_uint64(0);
    hostio_.make_string("");

    memset(&regs_, 0, sizeof(regs_));
    regs_.irq_mask = ~0;
}

IrqController::~IrqController() {
}

void IrqController::postinitService() {
    ihostio_ = static_cast<IHostIO *>(
        RISCV_get_service_iface(hostio_.to_string(), IFACE_HOSTIO));
    if (!ihostio_) {
        RISCV_error("Can't find IHostIO interface %s", hostio_.to_string());
    }
}

void IrqController::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((payload->addr - getBaseAddress()) & mask) / 4;
    if (payload->rw) {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            if (((payload->wstrb >> 4*i) & 0xFF) == 0) {
                continue;
            }

            switch (off + i) {
            case 0:
                regs_.irq_mask = payload->wpayload[i];
                RISCV_info("Set irq_mask = %08x", payload->wpayload[i]);
                break;
            case 1:
                regs_.irq_pending = payload->wpayload[i];
                RISCV_info("Set irq_pending = %08x", payload->wpayload[i]);
                break;
            case 2:
                regs_.irq_pending &= ~payload->wpayload[i];
                RISCV_info("Set irq_clear = %08x", payload->wpayload[i]);
                break;
            case 3:
                regs_.irq_pending |= payload->wpayload[i];
                RISCV_info("Set irq_rise = %08x", payload->wpayload[i]);
                break;
            case 4:
                regs_.irq_handler &= ~0xFFFFFFFFLL;
                regs_.irq_handler |= payload->wpayload[i];
                RISCV_info("Set irq_handler[31:0] = %08x", payload->wpayload[i]);
                break;
            case 5:
                regs_.irq_handler &= ~0xFFFFFFFF00000000LL;
                regs_.irq_handler |= (static_cast<uint64_t>(payload->wpayload[i]) << 32);
                RISCV_info("Set irq_handler[63:32] = %08x", payload->wpayload[i]);
                break;
            case 6:
                regs_.dbg_cause &= ~0xFFFFFFFFLL;
                regs_.dbg_cause |= payload->wpayload[i];
                RISCV_info("Set dbg_cause[31:0] = %08x", payload->wpayload[i]);
                break;
            case 7:
                regs_.dbg_cause &= ~0xFFFFFFFF00000000LL;
                regs_.dbg_cause |= (static_cast<uint64_t>(payload->wpayload[i]) << 32);
                RISCV_info("Set dbg_cause[63:32] = %08x", payload->wpayload[i]);
                break;
            case 8:
                regs_.dbg_epc &= ~0xFFFFFFFFLL;
                regs_.dbg_epc |= payload->wpayload[i];
                RISCV_info("Set dbg_epc[31:0] = %08x", payload->wpayload[i]);
                break;
            case 9:
                regs_.dbg_epc &= ~0xFFFFFFFF00000000LL;
                regs_.dbg_epc |= (static_cast<uint64_t>(payload->wpayload[i]) << 32);
                RISCV_info("Set dbg_epc[63:32] = %08x", payload->wpayload[i]);
                break;
            default:;
            }
        }
    } else {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            switch (off + i) {
            case 0:
                payload->rpayload[i] = regs_.irq_mask;
                RISCV_info("Get irq_mask = %08x", payload->rpayload[i]);
                break;
            case 1:
                payload->rpayload[i] = regs_.irq_pending;
                RISCV_info("Get irq_pending = %08x", payload->rpayload[i]);
                break;
            case 2:
                payload->rpayload[i] = 0;
                RISCV_info("Get irq_clear = %08x", payload->rpayload[i]);
                break;
            case 3:
                payload->rpayload[i] = 0;
                RISCV_info("Get irq_rise = %08x", payload->rpayload[i]);
                break;
            case 4:
                payload->rpayload[i] = static_cast<uint32_t>(regs_.irq_handler);
                RISCV_info("Get irq_handler[31:0] = %08x", payload->rpayload[i]);
                break;
            case 5:
                payload->rpayload[i] = static_cast<uint32_t>(regs_.irq_handler >> 32);
                RISCV_info("Get irq_handler[63:32] = %08x", payload->rpayload[i]);
                break;
            case 6:
                payload->rpayload[i] = static_cast<uint32_t>(regs_.dbg_cause);
                RISCV_info("Get dbg_cause[31:0] = %08x", payload->rpayload[i]);
                break;
            case 7:
                payload->rpayload[i] = static_cast<uint32_t>(regs_.dbg_cause >> 32);
                RISCV_info("Get dbg_cause[63:32] = %08x", payload->rpayload[i]);
                break;
            case 8:
                payload->rpayload[i] = static_cast<uint32_t>(regs_.dbg_epc);
                RISCV_info("Get dbg_epc[31:0] = %08x", payload->rpayload[i]);
                break;
            case 9:
                payload->rpayload[i] = static_cast<uint32_t>(regs_.dbg_epc >> 32);
                RISCV_info("Get dbg_epc[63:32] = %08x", payload->rpayload[i]);
                break;
            default:
                payload->rpayload[i] = ~0;
            }
        }
    }
}

void IrqController::riseLine() {
    if ((regs_.irq_mask & 0x1) == 0) {
        ihostio_->write(static_cast<uint16_t>(mipi_.to_uint64()), 1);
        RISCV_info("Raise interrupt", NULL);
    }
}

}  // namespace debugger

