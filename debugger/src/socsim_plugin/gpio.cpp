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
    registerInterface(static_cast<IWire *>(this));
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

ETransStatus GPIO::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask) / 4;
    uint32_t *mem_ = reinterpret_cast<uint32_t *>(&regs_);
    trans->response = MemResp_Valid;
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            if (trans->wstrb & (0xf << 4*i)) {
                mem_[off + i] = trans->wpayload.b32[i];
            }

            if (off + i == (reinterpret_cast<uint64_t>(&regs_.led) 
                          - reinterpret_cast<uint64_t>(&regs_))) {
                /*ISignalListener *ilistener;
                for (unsigned n = 0; n < listOfListerners_.size(); n++) {
                    ilistener = static_cast<ISignalListener *>(
                                    listOfListerners_[n].to_iface());
                    ilistener->updateSignal(0, 8, regs_.led & 0xFF);
                }*/
            }
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            trans->rpayload.b32[i] = mem_[i + off];
        }
    }
    return TRANS_OK;
}

/*void GPIO::setLevel(int start, int width, uint64_t value) {
    uint64_t t = value >> start;
    uint64_t msk = (1LL << width) - 1;
    uint64_t prev = dip_.to_uint64() & ~(msk << start);
    t &= msk;
    dip_.make_uint64(prev | (t << start));
    RISCV_info("set level pins[%d:%d] <= %" RV_PRI64 "x", start - 1, width, t);
}*/

}  // namespace debugger
