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
 */

#include <api_core.h>
#include "greth_gen1.h"

namespace debugger {

/** Class registration in the Core */
GrethGeneric::GrethGeneric(const char *name)
    : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IAxi4NbResponse *>(this));
    registerAttribute("IP", &ip_);
    registerAttribute("MAC", &mac_);
    registerAttribute("Bus", &bus_);
    registerAttribute("Transport", &transport_);
    registerAttribute("SysBusMasterID", &sysBusMasterID_);

    memset(txbuf_, 0, sizeof(txbuf_));
    seq_cnt_ = 35;
    RISCV_event_create(&event_tap_, "UART_event_tap");
}

GrethGeneric::~GrethGeneric() {
    RISCV_event_close(&event_tap_);
}

void GrethGeneric::postinitService() {
    ibus_ = static_cast<IMemoryOperation *>(
       RISCV_get_service_iface(bus_.to_string(), IFACE_MEMORY_OPERATION));

    if (!ibus_) {
        RISCV_error("Bus interface '%s' not found",
                    bus_.to_string());
        return;
    }

    itransport_ = static_cast<ILink *>(
        RISCV_get_service_iface(transport_.to_string(), IFACE_LINK));

    if (!itransport_) {
        RISCV_error("UDP interface '%s' not found",
                    bus_.to_string());
        return;
    }

    AttributeType clks;
    RISCV_get_clock_services(&clks);
    if (clks.size()) {
        iclk0_ = static_cast<IClock *>(clks[0u].to_iface());
    } else {
        RISCV_error("CPUs not found", NULL);
    }

    // Get global settings:
    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["SimEnable"].to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void GrethGeneric::busyLoop() {
    int bytes;
    uint8_t *tbuf;
    uint32_t bytes_to_read;
    UdpEdclCommonType req;
    RISCV_info("Ethernet thread was started", NULL);
    trans_.source_idx = sysBusMasterID_.to_int();

    while (isEnabled()) {
        bytes =
            itransport_->readData(rxbuf_, static_cast<int>(sizeof(rxbuf_)));

        if (bytes == 0) {
            continue;
        }

        req.control.word = read32(&rxbuf_[2]);
        req.address      = read32(&rxbuf_[6]);
        if (seq_cnt_ != req.control.request.seqidx) {
            sendNAK(&req);
            continue;
        }

        trans_.addr = req.address;
        if (req.control.request.write == 0) {
            trans_.action = MemAction_Read;
            trans_.wstrb = 0;
            tbuf = &txbuf_[10];
            bytes = sizeof(UdpEdclCommonType) + req.control.request.len;
        } else {
            trans_.action = MemAction_Write;
            tbuf = &rxbuf_[10];
            bytes = sizeof(UdpEdclCommonType);
        }
        bytes_to_read = req.control.request.len;
        while (bytes_to_read) {
            trans_.xsize = bytes_to_read;
            if (trans_.xsize > 8) {
                trans_.xsize = 8;
            }
            if (trans_.action == MemAction_Write) {
                memcpy(trans_.wpayload.b8, tbuf, trans_.xsize);
                trans_.wstrb = (1 << trans_.xsize) - 1;
            }
            RISCV_event_clear(&event_tap_);
            ibus_->nb_transport(&trans_, this);
            if (RISCV_event_wait_ms(&event_tap_, 500) != 0) {
                RISCV_error("CPU queue callback timeout", NULL);
            } else if (trans_.action == MemAction_Read) {
                memcpy(tbuf, trans_.rpayload.b8, trans_.xsize);
            }
            tbuf += trans_.xsize;
            trans_.addr += trans_.xsize;
            bytes_to_read -= trans_.xsize;
        }

        req.control.response.nak = 0;
        req.control.response.seqidx = seq_cnt_;
        write32(&txbuf_[2], req.control.word);

        seq_cnt_++;
        itransport_->sendData(txbuf_, bytes);
    }
}

void GrethGeneric::nb_response(Axi4TransactionType *trans) {
    RISCV_event_set(&event_tap_);
}

ETransStatus GrethGeneric::b_transport(Axi4TransactionType *trans) {
    RISCV_error("ETH Slave registers not implemented", NULL);
    return TRANS_OK;
}

void GrethGeneric::sendNAK(UdpEdclCommonType *req) {
    req->control.response.nak = 1;
    req->control.response.seqidx = seq_cnt_;
    req->control.response.len = 0;
    write32(&txbuf_[2], req->control.word);
    write32(&txbuf_[6], req->address);

    itransport_->sendData(txbuf_, sizeof(UdpEdclCommonType));
}

uint32_t GrethGeneric::read32(uint8_t *buf) {
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3] << 0);
}

void GrethGeneric::write32(uint8_t *buf, uint32_t v) {
    buf[0] = static_cast<char>((v >> 24) & 0xFF);
    buf[1] = static_cast<char>((v >> 16) & 0xFF);
    buf[2] = static_cast<char>((v >> 8) & 0xFF);
    buf[3] = static_cast<char>(v & 0xFF);
}

}  // namespace debugger
