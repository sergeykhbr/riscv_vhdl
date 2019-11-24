/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RF front-end black-box model.
 */

#include "api_core.h"
#include "rfctrl.h"

namespace debugger {

RfController::RfController(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("SubSystemConfig", &subsystemConfig_);

    memset(&regs_, 0, sizeof(regs_));
}

RfController::~RfController() {
}

void RfController::postinitService() {
}

ETransStatus RfController::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask) / 4;
    trans->response = MemResp_Valid;
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            if ((trans->wstrb & (0xf << 4*i)) == 0) {
                continue;
            }
            switch (off + i) {
            case (0x2c >> 2):
                RISCV_info("Run channel %d", trans->wpayload.b32[i]);
                regs_.run = 0xf;    // simulate loading delay
                break;
            case (0x3c >> 2): //rw_ant_status
                regs_.rw_ant_status &= ~0x3;  //enable power ant1/2
                regs_.rw_ant_status |= trans->wpayload.b32[i];
                break;
            default:;
            }
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize/4; i++) {
            switch (off + i) {
            case (0x2c >> 2): // run
                regs_.run >>= 1;
                trans->rpayload.b32[i] = regs_.run;
                break;
            case (0x3c >> 2): //rw_ant_status
                trans->rpayload.b32[i] = regs_.rw_ant_status;
                break;
            case (0x40 >> 2): //rw_ant_status
                trans->rpayload.b32[i] = subsystemConfig_.to_uint32();
                break;
            default:
                trans->rpayload.b32[i] = ~0;
            }
        }
    }
    return TRANS_OK;
}

}  // namespace debugger
