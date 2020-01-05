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

namespace debugger {

#define GENERATE_REF_ZEPHYR

UART::UART(const char *name) : RegMemBankGeneric(name),
    status_(static_cast<IService *>(this), "status", 0x00),
    scaler_(static_cast<IService *>(this), "scaler", 0x04),
    fwcpuid_(static_cast<IService *>(this), "fwcpuid", 0x08),
    data_(static_cast<IService *>(this), "data", 0x10) {
    registerInterface(static_cast<ISerial *>(this));
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("FifoSize", &fifoSize_);
    registerAttribute("IrqControl", &irqctrl_);
    registerAttribute("Clock", &clock_);
    registerAttribute("CmdExecutor", &cmdexec_);

    listeners_.make_list(0);
    RISCV_mutex_init(&mutexListeners_);

    rxfifo_ = 0;
    rx_total_ = 0;
    pcmd_ = 0;

    tx_total_ = 0;
    tx_wcnt_ = 0;
    tx_rcnt_ = 0;
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
    uint64_t baseoff = baseAddress_.to_uint64();
    status_.setBaseAddress(baseoff + status_.getBaseAddress());
    scaler_.setBaseAddress(baseoff + scaler_.getBaseAddress());
    fwcpuid_.setBaseAddress(baseoff + fwcpuid_.getBaseAddress());
    data_.setBaseAddress(baseoff + data_.getBaseAddress());

    rxfifo_ = new char[fifoSize_.to_int()];
    p_rx_wr_ = rxfifo_;
    p_rx_rd_ = rxfifo_;

    iwire_ = static_cast<IWire *>(
        RISCV_get_service_port_iface(irqctrl_[0u].to_string(),
                                     irqctrl_[1].to_string(),
                                     IFACE_WIRE));
    if (!iwire_) {
        RISCV_error("Can't find IWire interface %s", irqctrl_[0u].to_string());
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
        pcmd_ = new UartCmdType(static_cast<IService *>(this), getObjName());
        icmdexec_->registerCommand(pcmd_);
    }

#ifdef GENERATE_REF_ZEPHYR
    iclk_->registerStepCallback(static_cast<IClockListener *>(this), 1);
#endif
}

void UART::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(pcmd_);
    }
}

void UART::setScaler(uint32_t scaler) {
}

int UART::writeData(const char *buf, int sz) {
    if (rxfifo_ == 0) {
        return 0;
    }
    if (sz > (fifoSize_.to_int() - rx_total_)) {
        sz = (fifoSize_.to_int() - rx_total_);
    }
    for (int i = 0; i < sz; i++) {
        rx_total_++;
        *p_rx_wr_ = buf[i];
        if ((++p_rx_wr_) >= (rxfifo_ + fifoSize_.to_int())) {
            p_rx_wr_ = rxfifo_;
        }
    }

    if (status_.getTyped().b.rx_irq_ena) {
        iwire_->raiseLine();
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
#ifdef GENERATE_REF_ZEPHYR
    iclk_->registerStepCallback(static_cast<IClockListener *>(this), t+1);
    if (iclk_->getExecCounter() == 6454+0*4) {
        iwire_->raiseLine();
    }
    if (iclk_->getExecCounter() == 14558+0*4) {
        writeData("help\r\n", 6);
        iwire_->raiseLine();
    }
#else
    if (tx_total_) {
        tx_rcnt_ = (tx_rcnt_ + 1) % FIFOSZ;
        tx_total_--;

        if (tx_total_ == 0 && status_.getTyped().b.tx_irq_ena) {
            iwire_->raiseLine();
        }
    }
#endif
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

#if!defined(GENERATE_REF_ZEPHYR)
    if (status_.getTyped().b.tx_irq_ena) {
        iwire_->raiseLine();
    }
#endif

    //if (tx_total_ < FIFOSZ) {
    //    tx_fifo_[tx_wcnt_] = v;
    //    tx_wcnt_ = (tx_wcnt_ + 1) % FIFOSZ;
    //    tx_total_++;
    //}

    //iclk_->moveStepCallback(static_cast<IClockListener *>(this),
    //                        t + 10*2*scaler_.getValue().val);

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

uint64_t UART::STATUS_TYPE::aboutToRead(uint64_t cur_val) {
    UART *p = static_cast<UART *>(parent_);
    value_type t;
    t.v = cur_val;
    if (p->getRxTotal() == 0) {
        t.b.rx_fifo_empty = 1;
        t.b.rx_fifo_full = 0;
    } else if (p->getRxTotal() >= (p->getFifoSize() - 1)) {
        t.b.rx_fifo_empty = 0;
        t.b.rx_fifo_full = 1;
    } else {
        t.b.rx_fifo_empty = 0;
        t.b.rx_fifo_full = 0;
    }

    if (p->getTxTotal() == 0) {
        t.b.tx_fifo_empty = 0;//1;
        t.b.tx_fifo_full = 0;
    } else if (p->getTxTotal() == FIFOSZ) {
        t.b.tx_fifo_empty = 0;
        t.b.tx_fifo_full = 1;
    } else {
        t.b.tx_fifo_empty = 0;
        t.b.tx_fifo_full = 0;
    }
    return t.v;
}
uint64_t UART::SCALER_TYPE::aboutToWrite(uint64_t new_val) {
    UART *p = static_cast<UART *>(parent_);
    p->setScaler(static_cast<uint32_t>(new_val));
    return new_val;    
}

uint64_t UART::DATA_TYPE::aboutToRead(uint64_t cur_val) {
    UART *p = static_cast<UART *>(parent_);
    return static_cast<uint8_t>(p->getByte());
}

uint64_t UART::DATA_TYPE::aboutToWrite(uint64_t new_val) {
    UART *p = static_cast<UART *>(parent_);
    p->putByte(static_cast<char>(new_val));
    return new_val;    
}

}  // namespace debugger

