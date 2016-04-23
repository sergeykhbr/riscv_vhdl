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
            regionControlWr(off >> 2, payload);
        } else {
            regionControlRd(off >> 2, payload);
        }
    }
}

void DSU::regionCsrRd(uint64_t off, Axi4TransactionType *payload) {
    uint64_t rdata;
    ihostio_->read(static_cast<uint16_t>(off), &rdata);
    if (payload->addr & 0x4) {
        payload->rpayload[0] = static_cast<uint32_t>(rdata >> 32);
        payload->rpayload[1] = payload->rpayload[0];
        payload->rpayload[2] = payload->rpayload[0];
        payload->rpayload[3] = payload->rpayload[0];
    } else {
        payload->rpayload[0] = static_cast<uint32_t>(rdata);
        payload->rpayload[1] = static_cast<uint32_t>(rdata >> 32);
        payload->rpayload[2] = payload->rpayload[0];
        payload->rpayload[3] = payload->rpayload[1];
    }
}

void DSU::regionCsrWr(uint64_t off, Axi4TransactionType *payload) {

    if (payload->xsize == 4) {
        if (payload->addr & 0x4) {
            msb_of_64(&wdata_, payload->wpayload[0]);
            ihostio_->write(static_cast<uint16_t>(off), wdata_);
        } else {
            lsb_of_64(&wdata_, payload->wpayload[0]);
        }
    } else {
        if (payload->wstrb & 0x00FF) {
            lsb_of_64(&wdata_, payload->wpayload[0]);
            msb_of_64(&wdata_, payload->wpayload[1]);
        } else if (payload->wstrb & 0xFF00) {
            lsb_of_64(&wdata_, payload->wpayload[2]);
            msb_of_64(&wdata_, payload->wpayload[3]);
        }
        ihostio_->write(static_cast<uint16_t>(off), wdata_);
    }
}

void DSU::regionControlRd(uint64_t off, Axi4TransactionType *payload) {
}

void DSU::regionControlWr(uint64_t off, Axi4TransactionType *payload) {
    ICpuRiscV *idbg = ihostio_->getCpuInterface();
    switch (off) {
    case 0:     // CPU Run Control register
        if (payload->wpayload[0] & 0x1) {
            idbg->halt();
        } else if (payload->wpayload[0] & 0x2) {
            idbg->step(regs_.step_cnt);
        } else {
            idbg->go();
        }
        break;
    case 1:
        break;
    case 2:
        lsb_of_64(&regs_.step_cnt, payload->wpayload[0]);
        break;
    case 3:
        msb_of_64(&regs_.step_cnt, payload->wpayload[0]);
        break;
    default:;
    }
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

