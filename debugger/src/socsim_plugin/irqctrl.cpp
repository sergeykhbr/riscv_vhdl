/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interrupt controller functional model.
 */

#include "api_core.h"
#include "irqctrl.h"
#include <riscv-isa.h>
#include "coreservices/icpuriscv.h"

namespace debugger {

IrqPort::IrqPort(IService *parent, const char *portname, int idx) {
    parent_ = parent;
    parent->registerPortInterface(portname, static_cast<IWire *>(this));
    idx_ = idx;
    level_ = false;
};

void IrqPort::raiseLine() {
    level_ = true;
    static_cast<IrqController *>(parent_)->setPendingBit(idx_);
}

void IrqPort::setLevel(bool level) {
    if (!level_ && level) {
        raiseLine();
    } else if (level_ && !level) {
        lowerLine();
    }
}

IrqController::IrqController(const char *name) : RegMemBankGeneric(name),
    irq_mask(static_cast<IService *>(this), "irq_mask", 0x00),
    irq_pending(static_cast<IService *>(this), "irq_pending", 0x04),
    irq_clear(static_cast<IService *>(this), "irq_clear", 0x08),
    irq_raise(static_cast<IService *>(this), "irq_raise", 0x0c),
    isr_table_l(static_cast<IService *>(this), "isr_table_l", 0x10),
    isr_table_m(static_cast<IService *>(this), "isr_table_m", 0x14),
    dbg_cause_l(static_cast<IService *>(this), "dbg_cause_l", 0x18),
    dbg_cause_m(static_cast<IService *>(this), "dbg_cause_m", 0x1c),
    dbg_epc_l(static_cast<IService *>(this), "dbg_epc_l", 0x20),
    dbg_epc_m(static_cast<IService *>(this), "dbg_epc_m", 0x24),
    irq_lock(static_cast<IService *>(this), "irq_lock", 0x28),
    irq_cause(static_cast<IService *>(this), "irq_cause", 0x2c) {
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("CPU", &cpu_);
    registerAttribute("IrqTotal", &irqTotal_);

    char portname[256];
    for (int i = 1; i < IRQ_MAX; i++) {
        RISCV_sprintf(portname, sizeof(portname), "irq%d", i);
        irqlines_[i] = new IrqPort(this, portname, i);
    }

    cpu_.make_string("");
    irqTotal_.make_uint64(4);

    irq_mask.setValue(0x1e);
    irq_lock.setValue(1);
    //memset(&regs_, 0, sizeof(regs_));
    //regs_.irq_mask = 0x1e;
    //regs_.irq_lock = 1;
}

IrqController::~IrqController() {
}

void IrqController::postinitService() {
    RegMemBankGeneric::postinitService();

    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't find IClock interface %s", cpu_.to_string());
        return;
    }

    icpu_ = static_cast<ICpuGeneric *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CPU_GENERIC));
    if (!icpu_) {
        RISCV_error("Can't find ICpuRiscV interface %s", cpu_.to_string());
        return;
    }
    uint64_t t = iclk_->getStepCounter();
    iclk_->registerStepCallback(static_cast<IClockListener *>(this), t + 1);
}

/*ETransStatus IrqController::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask) / 4;
    uint32_t t1;
    trans->response = MemResp_Valid;
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            if (((trans->wstrb >> 4*i) & 0xFF) == 0) {
                continue;
            }

            switch (off + i) {
            case 0:
                regs_.irq_mask = trans->wpayload.b32[i] & 0x1e;
                RISCV_info("Set irq_mask = %08x", trans->wpayload.b32[i]);
                break;
            case 1:
                regs_.irq_pending = trans->wpayload.b32[i] & 0x1e;
                RISCV_info("Set irq_pending = %08x", trans->wpayload.b32[i]);
                break;
            case 2:
                t1 = regs_.irq_pending;
                regs_.irq_pending &= ~trans->wpayload.b32[i];
                if (t1 && !regs_.irq_pending) {
                    icpu_->lowerSignal(SIGNAL_XExternal);
                }
                RISCV_info("Set irq_clear = %08x", trans->wpayload.b32[i]);
                break;
            case 3:
                regs_.irq_pending |= trans->wpayload.b32[i];
                RISCV_info("Set irq_rise = %08x", trans->wpayload.b32[i]);
                break;
            case 4:
                regs_.isr_table &= ~0xFFFFFFFFLL;
                regs_.isr_table |= trans->wpayload.b32[i];
                RISCV_info("Set irq_handler[31:0] = %08x",
                            trans->wpayload.b32[i]);
                break;
            case 5:
                regs_.isr_table &= ~0xFFFFFFFF00000000LL;
                regs_.isr_table |= 
                    (static_cast<uint64_t>(trans->wpayload.b32[i]) << 32);
                RISCV_info("Set irq_handler[63:32] = %08x",
                            trans->wpayload.b32[i]);
                break;
            case 6:
                regs_.dbg_cause &= ~0xFFFFFFFFLL;
                regs_.dbg_cause |= trans->wpayload.b32[i];
                RISCV_info("Set dbg_cause[31:0] = %08x",
                            trans->wpayload.b32[i]);
                break;
            case 7:
                regs_.dbg_cause &= ~0xFFFFFFFF00000000LL;
                regs_.dbg_cause |= 
                    (static_cast<uint64_t>(trans->wpayload.b32[i]) << 32);
                RISCV_info("Set dbg_cause[63:32] = %08x",
                            trans->wpayload.b32[i]);
                break;
            case 8:
                regs_.dbg_epc &= ~0xFFFFFFFFLL;
                regs_.dbg_epc |= trans->wpayload.b32[i];
                RISCV_info("Set dbg_epc[31:0] = %08x", trans->wpayload.b32[i]);
                break;
            case 9:
                regs_.dbg_epc &= ~0xFFFFFFFF00000000LL;
                regs_.dbg_epc |= 
                    (static_cast<uint64_t>(trans->wpayload.b32[i]) << 32);
                RISCV_info("Set dbg_epc[63:32] = %08x",
                            trans->wpayload.b32[i]);
                break;
            case 10:
                regs_.irq_lock = trans->wpayload.b32[i];
                RISCV_info("Set irq_ena = %08x", trans->wpayload.b32[i]);
                if (regs_.irq_lock == 0 && regs_.irq_pending) {
                    icpu_->lowerSignal(SIGNAL_XExternal);
                }
                break;
            case 11:
                regs_.irq_cause_idx = trans->wpayload.b32[i];
                RISCV_info("Set irq_cause_idx = %08x", trans->wpayload.b32[i]);
                break;
            default:;
            }
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            switch (off + i) {
            case 0:
                trans->rpayload.b32[i] = regs_.irq_mask;
                RISCV_info("Get irq_mask = %08x", trans->rpayload.b32[i]);
                break;
            case 1:
                trans->rpayload.b32[i] = regs_.irq_pending;
                RISCV_info("Get irq_pending = %08x", trans->rpayload.b32[i]);
                break;
            case 2:
                trans->rpayload.b32[i] = 0;
                RISCV_info("Get irq_clear = %08x", trans->rpayload.b32[i]);
                break;
            case 3:
                trans->rpayload.b32[i] = 0;
                RISCV_info("Get irq_rise = %08x", trans->rpayload.b32[i]);
                break;
            case 4:
                trans->rpayload.b32[i] =
                    static_cast<uint32_t>(regs_.isr_table);
                RISCV_info("Get irq_handler[31:0] = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 5:
                trans->rpayload.b32[i] 
                    = static_cast<uint32_t>(regs_.isr_table >> 32);
                RISCV_info("Get irq_handler[63:32] = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 6:
                trans->rpayload.b32[i] =
                    static_cast<uint32_t>(regs_.dbg_cause);
                RISCV_info("Get dbg_cause[31:0] = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 7:
                trans->rpayload.b32[i] =
                    static_cast<uint32_t>(regs_.dbg_cause >> 32);
                RISCV_info("Get dbg_cause[63:32] = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 8:
                trans->rpayload.b32[i] = static_cast<uint32_t>(regs_.dbg_epc);
                RISCV_info("Get dbg_epc[31:0] = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 9:
                trans->rpayload.b32[i] =
                    static_cast<uint32_t>(regs_.dbg_epc >> 32);
                RISCV_info("Get dbg_epc[63:32] = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 10:
                trans->rpayload.b32[i] = regs_.irq_lock;
                RISCV_info("Get irq_ena = %08x",
                            trans->rpayload.b32[i]);
                break;
            case 11:
                trans->rpayload.b32[i] = regs_.irq_cause_idx;
                RISCV_info("Get irq_cause_idx = %08x",
                            trans->rpayload.b32[i]);
                break;
            default:
                trans->rpayload.b32[i] = ~0;
            }
        }
    }
    return TRANS_OK;
}
*/
void IrqController::stepCallback(uint64_t t) {
    iclk_->registerStepCallback(static_cast<IClockListener *>(this), t + 1);
    if (irq_lock.getValue().val == 1) {
        return;
    }
    if (~irq_mask.getValue().val & irq_pending.getValue().val) {
        icpu_->raiseSignal(SIGNAL_XExternal);   // PLIC interrupt (external)
        RISCV_debug("Raise interrupt", NULL);
    }
}

void IrqController::setPendingBit(int idx) {
    Reg32Type t = irq_pending.getValue();
    t.val |= (0x1 << idx);
    irq_pending.setValue(t.val);
    RISCV_info("request Interrupt %d", idx);
}

void IrqController::clearPendingBit(int idx) {
    Reg32Type t = irq_pending.getValue();
    uint32_t prev = t.val;
    t.val &= ~(1ul << idx);
    irq_pending.setValue(t.val);
    if (prev && !t.val) {
        icpu_->lowerSignal(SIGNAL_XExternal);
    }
}

uint32_t IrqController::IRQ_CLEAR_TYPE::aboutToWrite(uint32_t nxt_val) {
    IrqController *p = static_cast<IrqController *>(parent_);
    int i = 0;
    while (nxt_val) {
        if (nxt_val & 0x1) {
            p->clearPendingBit(i);
        }
        i++;
        nxt_val >>= 1;
    }
    return nxt_val;
}

}  // namespace debugger

