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
 * Packet format:
 *             Write command: 
 *                 Send     [11.Length-1].Addr[63:0].Data[31:0]*(x Length)
 *             Read command: 
 *                 Send     [10.Length-1].Addr[63:0]
 *                 Receive  Data[31:0]*(x Length)
 */

#include "api_core.h"
#include "uartmst.h"
#include "periphmap.h"

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


UartMst::UartMst(const char *name)  : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IAxi4NbResponse *>(this));
    registerInterface(static_cast<ISerial *>(this));
    registerAttribute("Bus", &bus_);

    listeners_.make_list(0);
    bus_.make_string("");

    RISCV_event_create(&event_tap_, "UartMst_event_tap");
    RISCV_event_create(&event_request_, "UartMst_event_request");
    RISCV_event_create(&event_accept_, "UartMst_event_accept");
    RISCV_mutex_init(&mutexListeners_);

    memset(&regs_, 0, sizeof(regs_));
    regs_.status = UART_STATUS_TX_EMPTY | UART_STATUS_RX_EMPTY;

    txbuf_sz_ = 0;
    baudrate_detect_ = false;
}

UartMst::~UartMst() {
    RISCV_event_close(&event_tap_);
    RISCV_event_close(&event_request_);
    RISCV_event_close(&event_accept_);
    RISCV_mutex_destroy(&mutexListeners_);
}

void UartMst::postinitService() {
    ibus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(bus_.to_string(), IFACE_MEMORY_OPERATION));
    if (!ibus_) {
        RISCV_error("Can't find IBus interface %s", bus_.to_string());
    }
    run();
}

void UartMst::busyLoop() {
    UartMstPacketType packet;
    RISCV_info("UartMst thread was started", NULL);
    trans_.source_idx = CFG_NASTI_MASTER_MSTUART;          // Hardcoded in VHDL value

    int burst_sz;
    while (isEnabled()) {
        if (RISCV_event_wait_ms(&event_request_, 50) != 0) {
            continue;
        }
        RISCV_event_clear(&event_request_);
        memcpy(&packet, txbuf_.buf, sizeof(packet));
        RISCV_event_set(&event_accept_);
        txbuf_sz_ = 0;

        if (!baudrate_detect_) {
            // Symbol 0x55 runs baudrate detector
            if (packet.cmd == 0x55) {
                baudrate_detect_ = true;
            }
            continue;
        }

        if ((packet.cmd & 0x80) == 0) {
            RISCV_error("Wrong request format", NULL);
            continue;
        }

        trans_.addr = packet.addr.val & 0xFFFFFFFF;
        trans_.xsize = 4;
        if ((packet.cmd & 0x40) == 0) {
            trans_.action = MemAction_Read;
        } else {
            trans_.action = MemAction_Write;
            trans_.wstrb = (1 << trans_.xsize) - 1;
        }

        burst_sz = (packet.cmd & 0x3F) + 1;
        for (int i = 0; i < burst_sz; i++) {
            if (trans_.action == MemAction_Write) {
                trans_.wpayload.b32[0] = packet.data[0].buf32[i];
            }
            RISCV_event_clear(&event_tap_);
            ibus_->nb_transport(&trans_, this);
            if (RISCV_event_wait_ms(&event_tap_, 500) != 0) {
                RISCV_error("CPU queue callback timeout", NULL);
            } else if (trans_.action == MemAction_Read) {
                RISCV_mutex_lock(&mutexListeners_);
                for (unsigned n = 0; n < listeners_.size(); n++) {
                    IRawListener *lstn = static_cast<IRawListener *>(
                                        listeners_[n].to_iface());
                    lstn->updateData(
                        reinterpret_cast<char *>(trans_.rpayload.b8),
                        trans_.xsize);
                }
                RISCV_mutex_unlock(&mutexListeners_);
            }
            trans_.addr += 4;
        }
    }
}

int UartMst::writeData(const char *buf, int sz) {
    if (sz > static_cast<int>(sizeof(UartMstPacketType))) {
        RISCV_error("Request will be truncated", NULL);
        sz = sizeof(UartMstPacketType);
    }
    memcpy(txbuf_.buf, buf, sz);
    txbuf_sz_ = sz;
    RISCV_event_clear(&event_accept_);
    RISCV_event_set(&event_request_);
    RISCV_event_wait_ms(&event_accept_, 500);
    return sz;
}

void UartMst::registerRawListener(IFace *listener) {
    AttributeType lstn(listener);
    RISCV_mutex_lock(&mutexListeners_);
    listeners_.add_to_list(&lstn);
    RISCV_mutex_unlock(&mutexListeners_);
}

void UartMst::unregisterRawListener(IFace *listener) {
    RISCV_mutex_lock(&mutexListeners_);
    for (unsigned i = 0; i < listeners_.size(); i++) {
        IFace *iface = listeners_[i].to_iface();
        if (iface == listener) {
            listeners_.remove_from_list(i);
            break;
        }
    }
    RISCV_mutex_unlock(&mutexListeners_);
}

void UartMst::getListOfPorts(AttributeType *list) {
    list->make_list(0);
}

int UartMst::openPort(const char *port, AttributeType settings) {
    return 0;
}

void UartMst::closePort() {
}

void UartMst::nb_response(Axi4TransactionType *trans) {
    RISCV_event_set(&event_tap_);
}

}  // namespace debugger

