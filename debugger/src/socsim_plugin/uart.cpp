/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UART functional model.
 */

#include "api_core.h"
#include "uart.h"

namespace debugger {

static const uint32_t UART_STATUS_TX_FULL     = 0x00000001;
static const uint32_t UART_STATUS_TX_EMPTY    = 0x00000002;
static const uint32_t UART_STATUS_RX_FULL     = 0x00000010;
static const uint32_t UART_STATUS_RX_EMPTY    = 0x00000020;
static const uint32_t UART_STATUS_ERR_PARITY  = 0x00000100;
static const uint32_t UART_STATUS_ERR_STOPBIT = 0x00000200;


UART::UART(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("Console", &console_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    console_.make_string("");

    memset(&regs_, 0, sizeof(regs_));
    regs_.status = UART_STATUS_TX_EMPTY | UART_STATUS_RX_EMPTY;
}

UART::~UART() {
}

void UART::postinitService() {
    iconsole_ = static_cast<IConsole *>(
        RISCV_get_service_iface(console_.to_string(), IFACE_CONSOLE));
    if (!iconsole_) {
        RISCV_error("Console %s not found", console_.to_string());
    }
}

void UART::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((payload->addr - getBaseAddress()) & mask) / 4;
    if (payload->rw) {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            if ((payload->wstrb & (0xf << 4*i)) == 0) {
                continue;
            }
            switch (off + i) {
            case 0:
                regs_.data = payload->wpayload[i];
                RISCV_info("Set data = %s", &regs_.data);
                if (regs_.data == '\r' || regs_.data == '\n') {
                    if (input_.size()) {
                        input_ = "<serialconsole> " + input_ + "\n";
                        if (iconsole_) {
                            iconsole_->writeBuffer(input_.c_str());
                        }
                        input_.clear();
                    }
                } else {
                    input_ += static_cast<char>(regs_.data);
                }
                break;
            case 1:
                regs_.status = payload->wpayload[i];
                RISCV_info("Set status = %08x", regs_.status);
                break;
            case 2:
                regs_.scaler = payload->wpayload[i];
                RISCV_info("Set scaler = %d", regs_.scaler);
                break;
            default:;
            }
        }
    } else {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            switch (off + i) {
            case 0:
                RISCV_debug("Get data = %02x", regs_.data);
                payload->rpayload[i] = 0;
                break;
            case 1:
                payload->rpayload[i] = regs_.status;
                RISCV_info("Get status = %08x", regs_.status);
                break;
            case 2:
                payload->rpayload[i] = regs_.scaler;
                RISCV_info("Get scaler = %d", regs_.scaler);
                break;
            default:
                payload->rpayload[i] = ~0;
            }
        }
    }
}

}  // namespace debugger

