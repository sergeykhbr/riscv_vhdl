/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "uart.h"

#define FAST_UART_SIM

namespace debugger {

UART::UART(const char *name) : RegMemBankGeneric(name),
    txdata_(static_cast<IService *>(this), "txdata", 0x00),
    rxdata_(static_cast<IService *>(this), "rxdata", 0x04),
    txctrl_(static_cast<IService *>(this), "txctrl", 0x08),
    rxctrl_(static_cast<IService *>(this), "rxctrl", 0x0C),
    ie_(static_cast<IService *>(this), "ie", 0x10),
    ip_(static_cast<IService *>(this), "ip", 0x14),
    scaler_(static_cast<IService *>(this), "scaler", 0x18),
    fwcpuid_(static_cast<IService *>(this), "fwcpuid", 0x1C) {
    registerInterface(static_cast<ISerial *>(this));
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("FifoSize", &fifoSize_);
    registerAttribute("IrqController", &irqctrl_);
    registerAttribute("IrqIdRx", &irqidrx_);
    registerAttribute("IrqIdTx", &irqidtx_);
    registerAttribute("Clock", &clock_);
    registerAttribute("CmdExecutor", &cmdexec_);

    listeners_.make_list(0);
    RISCV_mutex_init(&mutexListeners_);

    rxfifo_ = 0;
    rx_total_ = 0;
    pcmd_ = 0;

    tx_total_ = 0;
    tx_wcnt_ = 0;
    t_cb_cnt_ = 0;
}

UART::~UART() {
    RISCV_mutex_destroy(&mutexListeners_);
    if (rxfifo_) {
        delete [] rxfifo_;
    }
    if (pcmd_) {
        delete pcmd_;
    }
}

void UART::postinitService() {
    RegMemBankGeneric::postinitService();

    rxfifo_ = new char[fifoSize_.to_int()];
    p_rx_wr_ = rxfifo_;
    p_rx_rd_ = rxfifo_;

    iirq_ = static_cast<IIrqController *>(
        RISCV_get_service_iface(irqctrl_.to_string(),
                                     IFACE_IRQ_CONTROLLER));
    if (!iirq_) {
        RISCV_error("Can't find IIrqController interface %s",
                    irqctrl_.to_string());
    }

    iclk_ = static_cast<IClock *>(
            RISCV_get_service_iface(clock_.to_string(), 
                                    IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't get IClock interface %s",
                    clock_.to_string());
    }

    icmdexec_ = static_cast<ICmdExecutor *>(
            RISCV_get_service_iface(cmdexec_.to_string(), 
                                    IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
                    cmdexec_.to_string());
    } else {
        pcmd_ = new UartCmdType(static_cast<IService *>(this),
                                0,
                                getObjName());
        icmdexec_->registerCommand(pcmd_);
    }
}

void UART::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(pcmd_);
    }
}

uint32_t UART::getScaler() {
#ifdef FAST_UART_SIM
    return 100;
#else
    return 2*scaler_.getValue().val;
#endif
}

int UART::writeData(const char *buf, int sz) {
    if (rxfifo_ == 0) {
        return 0;
    }
    if (static_cast<uint32_t>(sz) > 
        (fifoSize_.to_uint32() - rx_total_)) {
        sz = (fifoSize_.to_uint32() - rx_total_);
    }
    for (int i = 0; i < sz; i++) {
        rx_total_++;
        *p_rx_wr_ = buf[i];
        if ((++p_rx_wr_) >= (rxfifo_ + fifoSize_.to_int())) {
            p_rx_wr_ = rxfifo_;
        }
    }

    if (ie_.getTyped().b.rxwm
        && rx_total_ > rxctrl_.getTyped().b.rxcnt) {
        iirq_->requestInterrupt(static_cast<IService *>(this),
                              irqidrx_.to_int());
    }
    return sz;
}

void UART::registerRawListener(IFace *listener) {
    AttributeType lstn(listener);
    RISCV_mutex_lock(&mutexListeners_);
    listeners_.add_to_list(&lstn);
    RISCV_mutex_unlock(&mutexListeners_);
}

void UART::unregisterRawListener(IFace *listener) {
    for (unsigned i = 0; i < listeners_.size(); i++) {
        IFace *iface = listeners_[i].to_iface();
        if (iface == listener) {
            RISCV_mutex_lock(&mutexListeners_);
            listeners_.remove_from_list(i);
            RISCV_mutex_unlock(&mutexListeners_);
            break;
        }
    }
}

void UART::getListOfPorts(AttributeType *list) {
    list->make_list(0);
}

int UART::openPort(const char *port, AttributeType settings) {
    return 0;
}

void UART::closePort() {
}

void UART::stepCallback(uint64_t t) {
    bool sent = false;
    if (tx_total_) {
        sent = true;
        tx_total_--;
    }

    if (sent && ie_.getTyped().b.txwm
        && tx_total_ < txctrl_.getTyped().b.txcnt) {
        iirq_->requestInterrupt(static_cast<IService*>(this),
                                irqidtx_.to_int());
    } else {
        iclk_->moveStepCallback(static_cast<IClockListener *>(this),
                                t + getScaler());
    }
}

void UART::putByte(char v) {
    char tbuf[2] = {v};
    uint64_t t = iclk_->getStepCounter();
    RISCV_info("[%" RV_PRI64 "d]Set data = %s", t, tbuf);

    RISCV_mutex_lock(&mutexListeners_);
    for (unsigned n = 0; n < listeners_.size(); n++) {
        IRawListener *lstn = static_cast<IRawListener *>(
                            listeners_[n].to_iface());

        lstn->updateData(&v, 1);
    }
    RISCV_mutex_unlock(&mutexListeners_);

#if 0
    // temporary disabled it but physically correct. Implement big enough UART buffer to compare with simulation
    if (tx_total_ < FIFOSZ) {
        tx_fifo_[tx_wcnt_] = v;
        tx_wcnt_ = (tx_wcnt_ + 1) % FIFOSZ;
        tx_total_++;
    }

    iclk_->moveStepCallback(static_cast<IClockListener *>(this),
                            t + getScaler());
#endif
}

char UART::getByte() {
    char ret = 0;
    if (rx_total_ == 0) {
        return ret;
    } else {
        ret = *p_rx_rd_;
        rx_total_--;
        if ((++p_rx_rd_) >= (rxfifo_ + fifoSize_.to_int())) {
            p_rx_rd_ = rxfifo_;
        }
    }
    return ret;
}

uint32_t UART::SCALER_TYPE::aboutToWrite(uint32_t new_val) {
//    UART *p = static_cast<UART *>(parent_);
    return new_val;    
}

uint32_t UART::FWCPUID_TYPE::aboutToWrite(uint32_t new_val) {
    if (new_val == 0 || getValue().val == 0) {
        // Not-zero value can be written only in cleared register
        return new_val;
    } else {
        return 0;
    }
}

uint32_t UART::TXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    UART *p = static_cast<UART *>(parent_);
    cur_val = 0;
    if (p->getTxTotal() == FIFOSZ) {
        cur_val |= 1ull << 31;  // TX FIFO full flag
    }
    return cur_val;
}

uint32_t UART::TXDATA_TYPE::aboutToWrite(uint32_t new_val) {
    UART *p = static_cast<UART *>(parent_);
    p->putByte(static_cast<char>(new_val));
    return new_val;    
}

uint32_t UART::RXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    UART *p = static_cast<UART *>(parent_);
    cur_val = 0;
    if (p->getRxTotal() == 0) {
        cur_val |= 1ull << 31;  // RX FIFO empty flag
    } else {
        cur_val = static_cast<uint8_t>(p->getByte());
    }
    return cur_val;
}

uint32_t UART::IPDATA_TYPE::aboutToRead(uint32_t cur_val) {
    UART *p = static_cast<UART *>(parent_);
    value_type ret;
    ret.v = 0;
    if (p->getTxTotal() < p->getTxWatermark()) {
        ret.b.txwm = 1;
    }
    if (p->getRxTotal() > p->getRxWatermark()) {
        ret.b.rxwm = 1;
    }
    return ret.v;
}

}  // namespace debugger

