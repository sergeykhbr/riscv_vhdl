/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
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

SerialDbgService::SerialDbgService(const char *name) 
    : IService(name) {
    registerInterface(static_cast<ITap *>(this));
    registerAttribute("Timeout", &timeout_);
    registerAttribute("Port", &port_);

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
    // hardcoded scale in tapuart, no need in clock detection
    /** Automatic baudrate definition on hardware side:
     *    [0] 0x55 - to init baud rate detector
     *    [1] 0x55 - to confirm baud rate value.
     */
    char tbuf[5] = {0x55, 0x55, 0x55, 0x55};
    iserial_->writeData(tbuf, 4);
}

void SerialDbgService::predeleteService() {
    if (iserial_) {
        iserial_->unregisterRawListener(static_cast<IRawListener *>(this));
    }
}

int SerialDbgService::read(uint64_t addr, int bytes, uint8_t *obuf) {
    uint32_t align_addr;
    uint32_t align_offset;
    int align_length;
    uint32_t obuf_len;
    uint32_t obuf_off = 0;

    align_offset = addr & 0x3ul;
    align_addr = addr & ~0x3ul;
    align_length = static_cast<int>((bytes + align_offset + 3) & ~0x3ul);

    if (!iserial_) {
        return TAP_ERROR;
    }
    int bytes_to_read = align_length;
    pkt_.fields.magic = MAGIC_ID;
    pkt_.fields.addr = align_addr;
    while (bytes_to_read) {
        pkt_.fields.cmd = (0x2 << 6);
        if (bytes_to_read > UART_MST_BURST_BYTES_MAX) {
            req_count_ = UART_MST_BURST_BYTES_MAX;
            pkt_.fields.cmd |= 0x3F;
        } else {
            req_count_ = bytes_to_read;
            pkt_.fields.cmd |= ((bytes_to_read / 4) - 1) & 0x3F;
        }
        rd_count_ = 0;
        wait_bytes_ = req_count_;
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

        obuf_len = rd_count_;
        if ((obuf_off + obuf_len) > static_cast<uint32_t>(bytes)) {
            // exclude alignment bytes from buffer end
            obuf_len = static_cast<uint32_t>(bytes) - obuf_off;
        }
        if (align_offset) {
            if (bytes_to_read == align_length) {
                memcpy(&obuf[obuf_off], &rx_buf_[align_offset], obuf_len);
                obuf_off += (bytes_to_read - align_offset);
            } else {
                memcpy(&obuf[obuf_off], rx_buf_, obuf_len);
                obuf_off += obuf_len;
            }
        } else {
            memcpy(&obuf[obuf_off], rx_buf_, obuf_len);
            obuf_off += obuf_len;
        }
        pkt_.fields.addr += static_cast<unsigned>(rd_count_);
        bytes_to_read -= rd_count_;
    }
    return bytes;
}

int SerialDbgService::write(uint64_t addr, int bytes, uint8_t *ibuf) {
    uint32_t align_addr;
    uint32_t align_offset;
    uint32_t align_offset0;
    int align_length;
    uint32_t ibuf_len;
    uint32_t ibuf_off = 0;
    uint32_t ubytes = static_cast<uint32_t>(bytes);

    if (!iserial_) {
        return TAP_ERROR;
    }

    align_offset = addr & 0x3ul;
    align_addr = addr & ~0x3ul;
    align_length = static_cast<int>((bytes + align_offset + 3) & ~0x3ul);
    align_offset0 = align_offset;

    int bytes_to_write = align_length;
    pkt_.fields.magic = MAGIC_ID;
    pkt_.fields.addr = align_addr;
    while (bytes_to_write) {
        pkt_.fields.cmd = (0x3 << 6);
        if (bytes_to_write > UART_MST_BURST_BYTES_MAX) {
            req_count_ = UART_MST_BURST_BYTES_MAX;
            pkt_.fields.cmd |= 0x3F;
        } else {
            req_count_ = bytes_to_write;
            pkt_.fields.cmd |= ((bytes_to_write / 4) - 1) & 0x3F;
        }

        ibuf_len = req_count_;
        // Alignment at the and of buffer
        if (ibuf_off + req_count_ - align_offset0 > ubytes) {
            read(pkt_.fields.addr, 8, pkt_.fields.data8);
            ibuf_len = align_offset0 + ubytes - ibuf_off;
        }
        // Alignment at the begin of buffer
        if (align_offset0 != 0) {
            read(align_addr, align_offset, pkt_.fields.data8);
            ibuf_len -= align_offset;
        }

        memcpy(&pkt_.fields.data8[align_offset0], &ibuf[ibuf_off], ibuf_len);
        ibuf_off += ibuf_len;
        align_offset0 = 0;      // only for zero step

        wait_bytes_ = 4;
        rd_count_ = 0;
        RISCV_event_clear(&event_block_);
        iserial_->writeData(pkt_.buf, UART_REQ_HEADER_SZ + req_count_);

        // Waiting "ACK\n" handshake
        if (RISCV_event_wait_ms(&event_block_, timeout_.to_int()) != 0) {
            RISCV_error("Writing [%08" RV_PRI64 "x] failed", addr);
            return TAP_ERROR;
        }

        bytes_to_write -= req_count_;
        pkt_.fields.addr += static_cast<unsigned>(req_count_);
    }
    return bytes;
}

void SerialDbgService::updateData(const char *buf, int buflen) {
    memcpy(&rx_buf_[rd_count_], buf, buflen);
    rd_count_ += buflen;
    if (rd_count_ < wait_bytes_) {
        return;
    }
    RISCV_event_set(&event_block_);
}

}  // namespace debugger
