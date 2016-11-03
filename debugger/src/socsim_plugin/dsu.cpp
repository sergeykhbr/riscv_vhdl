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
    registerAttribute("HostIO", &hostio_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    hostio_.make_string("");
    map_ = reinterpret_cast<DsuMapType *>(0);
}

DSU::~DSU() {
}

void DSU::postinitService() {
    ihostio_ = static_cast<IHostIO *>(
        RISCV_get_service_iface(hostio_.to_string(), IFACE_HOSTIO));
    if (!ihostio_) {
        RISCV_error("Can't find IHostIO interface %s", hostio_.to_string());
    }
}

void DSU::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((payload->addr - getBaseAddress()) & mask);
    if (off < 0x10000) {
        if (payload->rw) {
            regionCsrWr(off >> 4, payload);
        } else {
            regionCsrRd(off >> 4, payload);
        }
    } else {
        /// @warning Hardware doesn't support this features:
        off -= 0x10000;
        if (payload->rw) {
            regionControlWr(off, payload);
        } else {
            regionControlRd(off, payload);
        }
    }
}

void DSU::regionCsrRd(uint64_t off, Axi4TransactionType *payload) {
    uint64_t rdata;
    if (!ihostio_) {
        return;
    }
    ihostio_->read(static_cast<uint16_t>(off), &rdata);
    read64(rdata, payload->addr, payload->xsize, payload->rpayload);
}

void DSU::regionCsrWr(uint64_t off, Axi4TransactionType *payload) {
    if (!ihostio_) {
        return;
    }
    if (write64(&wdata_, payload->addr, payload->xsize, payload->wpayload)) {
        ihostio_->write(static_cast<uint16_t>(off), wdata_);
    }
}

void DSU::regionControlRd(uint64_t off, Axi4TransactionType *payload) {
    if (!ihostio_) {
        return;
    }
    ICpuRiscV *idbg = ihostio_->getCpuInterface();
    void *off64 = reinterpret_cast<void *>(off & ~0x7);
    uint64_t val = 0;
    if (off64 == &map_->control) {
        // CPU Run Control register
        if (idbg->isHalt()) {
            val |= 1;
        }
        read64(val, off, payload->xsize, payload->rpayload);
    } else if (off64 == &map_->step_cnt) {
        read64(step_cnt_, off, payload->xsize, payload->rpayload);
    } else if (off64 == &map_->add_breakpoint) {
    } else if (off64 == &map_->remove_breakpoint) {
    } else if (off64 >= &map_->cpu_regs[0] 
        && off64 < &map_->cpu_regs[DSU_GENERAL_CORE_REGS_NUM] ) {
        uint64_t idx = reinterpret_cast<uint64_t>(off64) 
                - reinterpret_cast<uint64_t>(&map_->cpu_regs);
        idx /= 8;
        val = idbg->getReg(idx);
        read64(val, off, payload->xsize, payload->rpayload);
    } else if (off64 == &map_->pc) {
        val = idbg->getPC();
        read64(val, off, payload->xsize, payload->rpayload);
    } else if (off64 == &map_->npc) {
        val = idbg->getNPC();
        read64(val, off, payload->xsize, payload->rpayload);
    }
}

void DSU::regionControlWr(uint64_t off, Axi4TransactionType *payload) {
    if (!ihostio_) {
        return;
    }
    ICpuRiscV *idbg = ihostio_->getCpuInterface();
    void *off64 = reinterpret_cast<void *>(off & ~0x7);
    if (off == reinterpret_cast<uint64_t>(&map_->control)) {
        // CPU Run Control register
        if (payload->wpayload[0] & 0x1) {
            idbg->halt();
        } else if (payload->wpayload[0] & 0x2) {
            idbg->step(step_cnt_);
        } else {
            idbg->go();
        }
    } else if (off64 == &map_->step_cnt) {
        write64(&step_cnt_, off, payload->xsize, payload->wpayload);
    } else if (off64 == &map_->add_breakpoint) {
        bool rdy = write64(&wdata_, payload->addr, 
                            payload->xsize, payload->wpayload);
        if (rdy) {
            idbg->addBreakpoint(wdata_);
        }
    } else if (off64 == &map_->remove_breakpoint) {
        bool rdy = write64(&wdata_, payload->addr, 
                            payload->xsize, payload->wpayload);
        if (rdy) {
            idbg->removeBreakpoint(wdata_);
        }
    } else if (off64 >= &map_->cpu_regs[0] 
        && off64 < &map_->cpu_regs[DSU_GENERAL_CORE_REGS_NUM] ) {
        uint64_t idx = reinterpret_cast<uint64_t>(off64) 
                - reinterpret_cast<uint64_t>(map_->cpu_regs);
        idx /= 8;

        bool rdy = write64(&wdata_, payload->addr, 
                            payload->xsize, payload->wpayload);
        if (rdy) {
            idbg->setReg(idx, wdata_);
        }
    } else if (off64 == &map_->pc) {
        bool rdy = write64(&wdata_, payload->addr, 
                            payload->xsize, payload->wpayload);
        if (rdy) {
            idbg->setPC(wdata_);
        }
    } else if (off64 == &map_->npc) {
        bool rdy = write64(&wdata_, payload->addr, 
                            payload->xsize, payload->wpayload);
        if (rdy) {
            idbg->setNPC(wdata_);
        }
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

