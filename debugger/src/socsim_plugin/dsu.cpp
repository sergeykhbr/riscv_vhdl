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
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("CPU", &cpu_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    cpu_.make_string("");
    map_ = reinterpret_cast<DsuMapType *>(0);
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
    void *poff = reinterpret_cast<void *>(off64);

    if (!icpu_) {
        return;
    }

    if (poff < &map_->ureg) {
        // CSR region:
        if (trans->action == MemAction_Write) {
            regionCsrWr(off64, trans);
        } else {
            regionCsrRd(off64, trans);
        }
    } else if (poff < &map_->udbg) {
        // CPU Registers region:
        if (trans->action == MemAction_Write) {
            regionRegWr(off64, trans);
        } else {
            regionRegRd(off64, trans);
        }
    } else if (poff < &map_->end) {
        // Debugging region:
        if (trans->action == MemAction_Write) {
            regionDebugWr(off64, trans);
        } else {
            regionDebugRd(off64, trans);
        }
    }
}

void DSU::regionCsrRd(uint64_t off, Axi4TransactionType *trans) {
    uint64_t rdata;
    rdata = icpu_->getCsr(off >> 3);
    read64(rdata, trans->addr, trans->xsize, trans->rpayload.b32);
}

void DSU::regionCsrWr(uint64_t off, Axi4TransactionType *trans) {
    // Transaction could be 4/5 bytes length
    if (write64(&wdata_, trans->addr, trans->xsize, trans->wpayload.b32)) {
        icpu_->setCsr(off >> 3, wdata_);
    }
}

void DSU::regionRegRd(uint64_t off64, Axi4TransactionType *trans) {
    void *poff = reinterpret_cast<void *>(off64 & ~0x7);

    uint64_t rdata = 0;
    if (poff < &map_->ureg.v.pc) {
        uint64_t idx = 
            off64 - reinterpret_cast<uint64_t>(&map_->ureg.v.iregs[0]);
        idx >>= 3;
        rdata = icpu_->getReg(idx);
    } else if (poff == &map_->ureg.v.pc) {
        rdata = icpu_->getPC();
    } else if (poff == &map_->ureg.v.npc) {
        rdata = icpu_->getNPC();
    }
     read64(rdata, off64, trans->xsize, trans->rpayload.b32);
}

void DSU::regionRegWr(uint64_t off64, Axi4TransactionType *trans) {
    void *poff = reinterpret_cast<void *>(off64 & ~0x7);
    bool rdy = write64(&wdata_, trans->addr, 
                            trans->xsize, trans->wpayload.b32);
    if (rdy) {
        return;
    }

    if (poff < &map_->ureg.v.pc) {
        uint64_t idx = 
            off64 - reinterpret_cast<uint64_t>(&map_->ureg.v.iregs[0]);
        idx >>= 3;
        icpu_->setReg(idx, wdata_);
    } else if (poff == &map_->ureg.v.pc) {
        icpu_->setPC(wdata_);
    } else if (poff == &map_->ureg.v.npc) {
        icpu_->setNPC(wdata_);
    }
}

void DSU::regionDebugRd(uint64_t off64, Axi4TransactionType *trans) {
    void *poff = reinterpret_cast<void *>(off64 & ~0x7);

    uint64_t rdata = 0;
    if (poff == &map_->udbg.v.control) {
        if (icpu_->isHalt()) {
            rdata |= 1;
        }
    } else if (poff == &map_->udbg.v.step_cnt) {
        rdata = step_cnt_;
    } else if (poff == &map_->udbg.v.add_breakpoint) {
        icpu_->addBreakpoint(wdata_);
    } else if (poff == &map_->udbg.v.remove_breakpoint) {
        icpu_->removeBreakpoint(wdata_);
    }
    read64(rdata, off64, trans->xsize, trans->rpayload.b32);
}

void DSU::regionDebugWr(uint64_t off64, Axi4TransactionType *trans) {
    void *poff = reinterpret_cast<void *>(off64 & ~0x7);
    bool rdy = write64(&wdata_, trans->addr, 
                            trans->xsize, trans->wpayload.b32);
    if (rdy) {
        return;
    }

    if (poff == &map_->udbg.v.control) {
        DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
        ctrl.val = wdata_;
        // CPU Run Control register
        if (ctrl.bits.halt) {
            icpu_->halt();
        } else if (ctrl.bits.stepping) {
            icpu_->step(step_cnt_);
        } else {
            icpu_->go();
        }
    } else if (poff == &map_->udbg.v.step_cnt) {
        step_cnt_ = wdata_;
    } else if (poff == &map_->udbg.v.add_breakpoint) {
        icpu_->addBreakpoint(wdata_);
    } else if (poff == &map_->udbg.v.remove_breakpoint) {
        icpu_->removeBreakpoint(wdata_);
    }
}

void DSU::read64(uint64_t reg, uint64_t off, 
                uint8_t xsize, uint32_t *payload) {
    if (xsize == 4) {
        if ((off & 0x4) == 0) {
            payload[0] = static_cast<uint32_t>(reg);
            payload[1] = static_cast<uint32_t>(reg >> 32);
            payload[2] = payload[0];
            payload[3] = payload[1];
        } else {
            payload[0] = static_cast<uint32_t>(reg >> 32);
            payload[1] = payload[0];
            payload[2] = payload[0];
            payload[3] = payload[0];
        }
    } else {
        payload[0] = static_cast<uint32_t>(reg);
        payload[1] = static_cast<uint32_t>(reg >> 32);
    }
}

bool DSU::write64(uint64_t *reg, uint64_t off, 
                    uint8_t xsize, uint32_t *payload) {
    bool ready = true;
    if (xsize == 4) {
        if ((off & 0x4) == 0) {
            lsb_of_64(reg, payload[0]);
            ready = false;
        } else {
            msb_of_64(reg, payload[0]);
        }
    } else {
        lsb_of_64(reg, payload[0]);
        msb_of_64(reg, payload[1]);
    }
    return ready;
}


void DSU::msb_of_64(uint64_t *val, uint32_t dw) {
    *val &= ~0xFFFFFFFF00000000ull;
    *val |= static_cast<uint64_t>(dw) << 32;
}

void DSU::lsb_of_64(uint64_t *val, uint32_t dw) {
    *val &= ~0xFFFFFFFFull;
    *val |= dw;
}

}  // namespace debugger

