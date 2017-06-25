/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debuger transport level via Serial link implementation.
 * @details
 *             Write command: 
 *                 Send     [11.Length-1].Addr[63:0].Data[31:0]*(x Length)
 *             Read command: 
 *                 Send     [10.Length-1].Addr[63:0]
 *                 Receive  Data[31:0]*(x Length)
 */

#include "api_core.h"
#include "serial_dbglink.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(SerialDbgService)

SerialDbgService::SerialDbgService(const char *name) 
    : IService(name) {
    registerInterface(static_cast<ITap *>(this));
    registerAttribute("Timeout", &timeout_);
    registerAttribute("Port", &port_);

    timeout_.make_int64(100);
    port_.make_string("");
    RISCV_event_create(&event_block_, "SerialDbg_event_block");
}

SerialDbgService::~SerialDbgService() {
    RISCV_event_close(&event_block_);
}

void SerialDbgService::postinitService() {
    iserial_ = static_cast<ISerial *>(
        RISCV_get_service_iface(port_.to_string(), IFACE_SERIAL));
    if (!iserial_) {
        RISCV_error("Can't find ISerial interface %s", port_.to_string());
        return;
    }
    iserial_->registerRawListener(static_cast<IRawListener *>(this));
    /** Automatic baudrate definition on hardware side:
     *    [0] 0x55 - to init baud rate detector
     *    [1] 0x55 - to confirm baud rate value.
     */
    char tbuf[3] = {0x55, 0x55};
    iserial_->writeData(tbuf, 2);
}

void SerialDbgService::predeleteService() {
    if (iserial_) {
        iserial_->unregisterRawListener(static_cast<IRawListener *>(this));
    }
}

int SerialDbgService::read(uint64_t addr, int bytes, uint8_t *obuf) {
    if (!iserial_) {
        return TAP_ERROR;
    }
    if (bytes <= 0 || (bytes & 0x3) != 0) {
        RISCV_error("Unaligned read %d", bytes);
        return TAP_ERROR;
    }
    int bytes_to_read = bytes;
    uint8_t *tout = obuf;
    pkt_.fields.addr = addr;
    while (bytes_to_read) {
        pkt_.fields.cmd = (0x2 << 6);
        if (bytes_to_read > 4 * UART_MST_BURST_MAX) {
            req_count_ = 4 * UART_MST_BURST_MAX;
            pkt_.fields.cmd |= 0x3F;
        } else {
            req_count_ = bytes_to_read;
            pkt_.fields.cmd |= ((bytes_to_read / 4) - 1) & 0x3F;
        }
        rd_count_ = 0;
        RISCV_event_clear(&event_block_);
        iserial_->writeData(pkt_.buf, UART_REQ_HEADER_SZ);

        if (RISCV_event_wait_ms(&event_block_, timeout_.to_int()) != 0) {
            RISCV_error("Reading [%08" RV_PRI64 "x] failed", addr);
            return TAP_ERROR;
        }
        if (rd_count_ != req_count_) {
            RISCV_error("Read bytes %d of %d", rd_count_, req_count_);
            return TAP_ERROR;
        }

        memcpy(tout, pkt_.fields.data, rd_count_);
        tout += rd_count_;
        pkt_.fields.addr += static_cast<unsigned>(rd_count_);
        bytes_to_read -= rd_count_;
    }
    return bytes;
}

int SerialDbgService::write(uint64_t addr, int bytes, uint8_t *ibuf) {
    if (!iserial_) {
        return TAP_ERROR;
    }
    if (bytes <= 0 || (bytes & 0x3) != 0) {
        RISCV_error("Unaligned write %d", bytes);
        return TAP_ERROR;
    }
    int bytes_to_write = bytes;
    uint8_t *tin = ibuf;
    pkt_.fields.addr = addr;
    while (bytes_to_write) {
        pkt_.fields.cmd = (0x3 << 6);
        if (bytes_to_write > 4 * UART_MST_BURST_MAX) {
            req_count_ = 4 * UART_MST_BURST_MAX;
            pkt_.fields.cmd |= 0x3F;
        } else {
            req_count_ = bytes_to_write;
            pkt_.fields.cmd |= ((bytes_to_write / 4) - 1) & 0x3F;
        }
        memcpy(pkt_.fields.data, tin, req_count_);
        iserial_->writeData(pkt_.buf, UART_REQ_HEADER_SZ + req_count_);

        tin += req_count_;
        bytes_to_write -= req_count_;
        pkt_.fields.addr += static_cast<unsigned>(req_count_);
    }
    return bytes;
}

void SerialDbgService::updateData(const char *buf, int buflen) {
    char *tbuf = &pkt_.buf[UART_REQ_HEADER_SZ + rd_count_];
    memcpy(tbuf, buf, buflen);
    rd_count_ += buflen;
    if (rd_count_ < req_count_) {
        return;
    }
    RISCV_event_set(&event_block_);
}

}  // namespace debugger
