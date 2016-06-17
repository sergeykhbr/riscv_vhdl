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
static const uint32_t UART_CONTROL_RX_IRQ_ENA = 0x00002000;
static const uint32_t UART_CONTROL_TX_IRQ_ENA = 0x00004000;
static const uint32_t UART_CONTROL_PARITY_ENA = 0x00008000;


UART::UART(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<ISerial *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("IrqLine", &irqLine_);
    registerAttribute("IrqControl", &irqctrl_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    irqLine_.make_uint64(0);
    irqctrl_.make_string("");
    listeners_.make_list(0);

    memset(&regs_, 0, sizeof(regs_));
    regs_.status = UART_STATUS_TX_EMPTY | UART_STATUS_RX_EMPTY;

    p_rx_wr_ = rxfifo_;
    p_rx_rd_ = rxfifo_;
    rx_total_ = 0;
}

UART::~UART() {
}

void UART::postinitService() {
    iwire_ = static_cast<IWire *>(
        RISCV_get_service_iface(irqctrl_.to_string(), IFACE_WIRE));
    if (!iwire_) {
        RISCV_error("Can't find IWire interface %s", irqctrl_.to_string());
    }
}

int UART::writeData(const char *buf, int sz) {
    if (sz > (RX_FIFO_SIZE - rx_total_)) {
        sz = (RX_FIFO_SIZE - rx_total_);
    }
    for (int i = 0; i < sz; i++) {
        rx_total_++;
        *p_rx_wr_ = buf[i];
        if ((++p_rx_wr_) >= (rxfifo_ + RX_FIFO_SIZE)) {
            p_rx_wr_ = rxfifo_;
        }
    }
#if 1
    // line ending
    rx_total_++;
    *p_rx_wr_ = '\r';
    if ((++p_rx_wr_) >= (rxfifo_ + RX_FIFO_SIZE)) {
        p_rx_wr_ = rxfifo_;
    }
#endif

    if (regs_.status & UART_CONTROL_RX_IRQ_ENA) {
        iwire_->raiseLine(irqLine_.to_int());
    }
    return sz;
}

void UART::registerRawListener(IFace *listener) {
    AttributeType lstn(listener);
    listeners_.add_to_list(&lstn);
}

void UART::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((payload->addr - getBaseAddress()) & mask) / 4;
    char wrdata;
    if (payload->rw) {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            if ((payload->wstrb & (0xf << 4*i)) == 0) {
                continue;
            }
            switch (off + i) {
            case 0:
                regs_.status = payload->wpayload[i];
                RISCV_info("Set status = %08x", regs_.status);
                break;
            case 1:
                regs_.scaler = payload->wpayload[i];
                RISCV_info("Set scaler = %d", regs_.scaler);
                break;
            case 4:
                wrdata = static_cast<char>(payload->wpayload[i]);
                RISCV_info("Set data = %s", &regs_.data);
                for (unsigned n = 0; n < listeners_.size(); n++) {
                    IRawListener *lstn = static_cast<IRawListener *>(
                                        listeners_[n].to_iface());

                    lstn->updateData(&wrdata, 1);
                }
                break;
            default:;
            }
        }
    } else {
        for (uint64_t i = 0; i < payload->xsize/4; i++) {
            switch (off + i) {
            case 0:
                if (0) {
                    regs_.status &= ~UART_STATUS_TX_EMPTY;
                } else {
                    regs_.status |= UART_STATUS_TX_EMPTY;
                }
                if (rx_total_ == 0) {
                    regs_.status |= UART_STATUS_RX_EMPTY;
                } else {
                    regs_.status &= ~UART_STATUS_RX_EMPTY;
                }
                payload->rpayload[i] = regs_.status;
                RISCV_info("Get status = %08x", regs_.status);
                break;
            case 1:
                payload->rpayload[i] = regs_.scaler;
                RISCV_info("Get scaler = %d", regs_.scaler);
                break;
            case 4:
                if (rx_total_ == 0) {
                    payload->rpayload[i] = 0;
                } else {
                    payload->rpayload[i] = *p_rx_rd_;
                    rx_total_--;
                    if ((++p_rx_rd_) >= (rxfifo_ + RX_FIFO_SIZE)) {
                        p_rx_rd_ = rxfifo_;
                    }
                }
                RISCV_debug("Get data = %02x", (payload->rpayload[i] & 0xFF));
                break;
            default:
                payload->rpayload[i] = ~0;
            }
        }
    }
}

}  // namespace debugger

