/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug Support Unit (DSU) functional model.
 */

#include "api_core.h"
#include "types_amba.h"
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
    uint64_t off = ((payload->addr - getBaseAddress()) & mask) >> 4;
    if (payload->rw) {
        if (payload->xsize == 4) {
            if (payload->addr & 0x4) {
                wdata_ &= ~0xFFFFFFFF00000000LL;
                wdata_ |= static_cast<uint64_t>(payload->wpayload[0]) << 32;
                ihostio_->write(static_cast<uint16_t>(off), wdata_);
            } else {
                wdata_ &= ~0xFFFFFFFFLL;
                wdata_ |= static_cast<uint64_t>(payload->wpayload[0]);
            }
        } else {
            if (payload->wstrb & 0x00FF) {
                wdata_ = (static_cast<uint64_t>(payload->wpayload[1]) << 32)
                      |  static_cast<uint64_t>(payload->wpayload[0]);
            } else if (payload->wstrb & 0xFF00) {
                wdata_ = (static_cast<uint64_t>(payload->wpayload[3]) << 32)
                      |  static_cast<uint64_t>(payload->wpayload[2]);
            }
            ihostio_->write(static_cast<uint16_t>(off), wdata_);
        }
    } else {
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
}

}  // namespace debugger

