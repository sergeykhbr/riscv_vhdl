/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      GPIO functional model.
 */

#include "api_core.h"
#include "gpio.h"

namespace debugger {

GPIO::GPIO(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("DIP", &dip_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    dip_.make_uint64(0);

    memset(&regs_, 0, sizeof(regs_));
}

GPIO::~GPIO() {
}

void GPIO::postinitService() {
    regs_.dip = static_cast<uint32_t>(dip_.to_uint64());
}

void GPIO::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((payload->addr - getBaseAddress()) & mask) / 4;
    uint32_t *mem_ = reinterpret_cast<uint32_t *>(&regs_);
    if (payload->rw) {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            if (payload->wstrb & (0xf << 4*i)) {
                mem_[off + i] = payload->wpayload[i];
            }

            if (off + i == (reinterpret_cast<uint64_t>(&regs_.led) 
                          - reinterpret_cast<uint64_t>(&regs_))) {
                RISCV_info("LED = %02x", regs_.led);
            }
        }
    } else {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            payload->rpayload[i] = mem_[i + off];
        }
    }
}

}  // namespace debugger

