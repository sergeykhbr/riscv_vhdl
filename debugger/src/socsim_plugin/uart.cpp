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

//#define GENERATE_PRECISE_DHRYSTONE_RTL_TRACE

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
    if (tx_total_) {
        tx_rcnt_ = (tx_rcnt_ + 1) % FIFOSZ;
        tx_total_--;

        if (tx_total_ == 0 && status_.getTyped().b.tx_irq_ena) {
            iwire_->raiseLine();
        }
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

#if!defined(GENERATE_PRECISE_DHRYSTONE_RTL_TRACE)
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

#ifdef GENERATE_PRECISE_DHRYSTONE_RTL_TRACE
enum ETxFifoState {
    TxFifo_Empty,
    TxFifo_NotEmpty,
    TxFifo_Full
};

struct dbg_state_type {
    uint64_t tm_to_switch;
    ETxFifoState next_state;
};

dbg_state_type dbg_steps[] = {
    {180, TxFifo_NotEmpty},
    {630, TxFifo_Full},
    {1582, TxFifo_NotEmpty},
    {1582+21, TxFifo_Full},
    {2921, TxFifo_NotEmpty},
    {2921+21, TxFifo_Full},
    {4260, TxFifo_NotEmpty},
    {4260+21, TxFifo_Full},
    {5599, TxFifo_NotEmpty},
    {5599+21, TxFifo_Full},
    {6931, TxFifo_NotEmpty},
    {6931+21, TxFifo_Full},
    {8270, TxFifo_NotEmpty},
    {8270+21, TxFifo_Full},
    {9609, TxFifo_NotEmpty},    // 'k'
    {9609+21, TxFifo_Full},
    {10948, TxFifo_NotEmpty},   // ' '
    {10948+21, TxFifo_Full},
    {12280, TxFifo_NotEmpty},   // 'n'
    {12280+21, TxFifo_Full},
    {13619, TxFifo_NotEmpty},   // 'o'
    {13619+21, TxFifo_Full},
    {14958, TxFifo_NotEmpty},   // 't'
    {14958+21, TxFifo_Full},
    {16297, TxFifo_NotEmpty},   // ' '
    {16297+21, TxFifo_Full},
    {17629, TxFifo_NotEmpty},   // 'f'
    {17629+21, TxFifo_Full},
    {18967, TxFifo_NotEmpty},   // 'o'
    {18967+21, TxFifo_Full},
    {20307, TxFifo_NotEmpty},   // 'u'
    {20307+21, TxFifo_Full},
    {21646, TxFifo_NotEmpty},   // 'n'
    {21646+21, TxFifo_Full},
    {22978, TxFifo_NotEmpty},   // 'd'
    {22978+21, TxFifo_Full},
    {24317, TxFifo_NotEmpty},   // '.'
    {24317+21, TxFifo_Full},
    {25656, TxFifo_NotEmpty},   // ' '
    {25656+21, TxFifo_Full},
    {26995, TxFifo_NotEmpty},   // 'E'
    {26995+21, TxFifo_Full},
    {28327, TxFifo_NotEmpty},   // 'n'
    {28327+21, TxFifo_Full},
    {29666, TxFifo_NotEmpty},   // 'a'
    {29666+21, TxFifo_Full},
    {31005, TxFifo_NotEmpty},   // 'b'
    {31005+21, TxFifo_Full},
    {32344, TxFifo_NotEmpty},   // 'l'
    {32344+21, TxFifo_Full},
    {33676, TxFifo_NotEmpty},   // 'e'
    {33676+21, TxFifo_Full},
    {35015, TxFifo_NotEmpty},   // ' '
    {35015+21, TxFifo_Full},
    {36354, TxFifo_NotEmpty},   // 'D'
    {36354+21, TxFifo_Full},
    {37693, TxFifo_NotEmpty},   // 'I'
    {37693+21, TxFifo_Full},
    {39025, TxFifo_NotEmpty},   // 'P'
    {39025+21, TxFifo_Full},
    {40364, TxFifo_NotEmpty},   // ' '
    {40364+21, TxFifo_Full},
    {41703, TxFifo_NotEmpty},   // 'i'
    {41703+21, TxFifo_Full},
    {43042, TxFifo_NotEmpty},   // 'n'
    {43042+21, TxFifo_Full},
    {44373, TxFifo_NotEmpty},   // 't'
    {44373+21, TxFifo_Full},
    {45713, TxFifo_NotEmpty},   // '_'
    {45713+21, TxFifo_Full},
    {47052, TxFifo_NotEmpty},   // 'r'
    {47052+21, TxFifo_Full},
    {48391, TxFifo_NotEmpty},   // 'f'
    {48391+21, TxFifo_Full},
    {49723, TxFifo_NotEmpty},   // '.'
    {49723+21, TxFifo_Full},
    {51062, TxFifo_NotEmpty},   // ' '
    {51062+21, TxFifo_Full},
    {52401, TxFifo_NotEmpty},   // '\r'
    {52401+21, TxFifo_Full},
    {53983, TxFifo_NotEmpty},   // '\n'
    {53983+21, TxFifo_Full},
    {55617, TxFifo_NotEmpty},
    {55617+11, TxFifo_Full},    // 'D'
    {57139, TxFifo_NotEmpty},
    {57139+11, TxFifo_Full},    // 'h'
    {58669, TxFifo_NotEmpty},
    {58669+11, TxFifo_Full},    // 'r'
    {60191, TxFifo_NotEmpty},
    {60191+11, TxFifo_Full},    // 'y'
    {61713, TxFifo_NotEmpty},
    {61713+11, TxFifo_Full},    // 's'
    {63243, TxFifo_NotEmpty},
    {63243+11, TxFifo_Full},    // 't'
    {64765, TxFifo_NotEmpty},
    {64765+11, TxFifo_Full},    // 'o'
    {66295, TxFifo_NotEmpty},
    {66295+11, TxFifo_Full},    // 'n'
    {67817, TxFifo_NotEmpty},
    {67817+11, TxFifo_Full},    // 'e'
    {69339, TxFifo_NotEmpty},
    {69339+11, TxFifo_Full},    // ' '
    {70869, TxFifo_NotEmpty},
    {70869+11, TxFifo_Full},    // 'B'
    {72391, TxFifo_NotEmpty},
    {72391+11, TxFifo_Full},    // 'e'
    {73921, TxFifo_NotEmpty},
    {73921+11, TxFifo_Full},    // 'n'
    {75443, TxFifo_NotEmpty},
    {75443+11, TxFifo_Full},    // 'c'
    {76965, TxFifo_NotEmpty},
    {76965+11, TxFifo_Full},    // 'h'
    {78495, TxFifo_NotEmpty},
    {78495+11, TxFifo_Full},    // 'm'
    {80017, TxFifo_NotEmpty},
    {80017+11, TxFifo_Full},    // 'a'
    {81547, TxFifo_NotEmpty},
    {81547+11, TxFifo_Full},    // 'r'
    {83069, TxFifo_NotEmpty},
    {83069+11, TxFifo_Full},    // 'k'
    {84591, TxFifo_NotEmpty},
    {84591+11, TxFifo_Full},    // ','
    {86121, TxFifo_NotEmpty},
    {86121+11, TxFifo_Full},    // ' '
    {87643, TxFifo_NotEmpty},
    {87643+11, TxFifo_Full},    // 'V'
    {89173, TxFifo_NotEmpty},
    {89173+11, TxFifo_Full},    // 'e'
    {90695, TxFifo_NotEmpty},
    {90695+11, TxFifo_Full},    // 'r'
    {92217, TxFifo_NotEmpty},
    {92217+11, TxFifo_Full},    // 's'
    {93747, TxFifo_NotEmpty},
    {93747+11, TxFifo_Full},    // 'i'
    {95269, TxFifo_NotEmpty},
    {95269+11, TxFifo_Full},    // 'o'
    {96799, TxFifo_NotEmpty},
    {96799+11, TxFifo_Full},    // 'n'
    {98321, TxFifo_NotEmpty},
    {98321+11, TxFifo_Full},    // ' '
    {99843, TxFifo_NotEmpty},
    {99843+11, TxFifo_Full},    // '2'
    {101373, TxFifo_NotEmpty},
    {101373+11, TxFifo_Full},   // '.'
    {102895, TxFifo_NotEmpty},
    {102895+11, TxFifo_Full},   // '1'
    {104425, TxFifo_NotEmpty},
    {104425+11, TxFifo_Full},   // ' '
    {105947, TxFifo_NotEmpty},
    {105947+11, TxFifo_Full},   // '('
    {107469, TxFifo_NotEmpty},
    {107469+11, TxFifo_Full},   // 'L'
    {108999, TxFifo_NotEmpty},
    {108999+11, TxFifo_Full},   // 'a'
    {110521, TxFifo_NotEmpty},
    {110521+11, TxFifo_Full},   // 'n'
    {112051, TxFifo_NotEmpty},
    {112051+11, TxFifo_Full},   // 'g'
    {113573, TxFifo_NotEmpty},
    {113573+11, TxFifo_Full},   // 'u'
    {115095, TxFifo_NotEmpty},
    {115095+11, TxFifo_Full},   // 'a'
    {116625, TxFifo_NotEmpty},
    {116625+11, TxFifo_Full},   // 'g'
    {118147, TxFifo_NotEmpty},
    {118147+11, TxFifo_Full},   // 'e'
    {119677, TxFifo_NotEmpty},
    {119677+11, TxFifo_Full},   // ':'
    {121199, TxFifo_NotEmpty},
    {121199+11, TxFifo_Full},   // ' '
    {122721, TxFifo_NotEmpty},
    {122721+11, TxFifo_Full},   // 'C'
    {124251, TxFifo_NotEmpty},
    {124251+11, TxFifo_Full},   // ')'
    {125773, TxFifo_NotEmpty},
    {125773+11, TxFifo_Full},   // '\r'
    {127389, TxFifo_NotEmpty},
    {127389+11, TxFifo_Full},   // '\n'
    {129016, TxFifo_NotEmpty},
    {129016+11, TxFifo_Full},   // 'P'
    {130538, TxFifo_NotEmpty},
    {130538+11, TxFifo_Full},   // 'r'
    {132060, TxFifo_NotEmpty},
    {132060+11, TxFifo_Full},   // 'o'
    {133590, TxFifo_NotEmpty},
    {133590+11, TxFifo_Full},   // 'g'
    {135112, TxFifo_NotEmpty},
    {135112+11, TxFifo_Full},   // 'r'
    {136642, TxFifo_NotEmpty},
    {136642+11, TxFifo_Full},   // 'a'
    {138164, TxFifo_NotEmpty},
    {138164+11, TxFifo_Full},   // 'm'
    {139686, TxFifo_NotEmpty},
    {139686+11, TxFifo_Full},   // ' '
    {141216, TxFifo_NotEmpty},
    {141216+11, TxFifo_Full},   // 'c'
    {142738, TxFifo_NotEmpty},
    {142738+11, TxFifo_Full},   // 'o'
    {144268, TxFifo_NotEmpty},
    {144268+11, TxFifo_Full},   // 'm'
    {145790, TxFifo_NotEmpty},
    {145790+11, TxFifo_Full},   // 'p'
    {147312, TxFifo_NotEmpty},
    {147312+11, TxFifo_Full},   // 'i'
    {148842, TxFifo_NotEmpty},
    {148842+11, TxFifo_Full},   // 'l'
    {150364, TxFifo_NotEmpty},
    {150364+11, TxFifo_Full},   // 'e'
    {151894, TxFifo_NotEmpty},
    {151894+11, TxFifo_Full},   // 'd'
    {153416, TxFifo_NotEmpty},
    {153416+11, TxFifo_Full},   // ' '
    {154938, TxFifo_NotEmpty},
    {154938+11, TxFifo_Full},   // 'w'
    {156468, TxFifo_NotEmpty},
    {156468+11, TxFifo_Full},   // 'i'
    {157990, TxFifo_NotEmpty},
    {157990+11, TxFifo_Full},   // 't'
    {159520, TxFifo_NotEmpty},
    {159520+11, TxFifo_Full},   // 'h'
    {161042, TxFifo_NotEmpty},
    {161042+11, TxFifo_Full},   // 'o'
    {162564, TxFifo_NotEmpty},
    {162564+11, TxFifo_Full},   // 'u'
    {164094, TxFifo_NotEmpty},
    {164094+11, TxFifo_Full},   // 't'
    {165616, TxFifo_NotEmpty},
    {165616+11, TxFifo_Full},   // ' '
    {167146, TxFifo_NotEmpty},
    {167146+11, TxFifo_Full},   // '\''
    {168668, TxFifo_NotEmpty},
    {168668+11, TxFifo_Full},   // 'r'
    {170190, TxFifo_NotEmpty},
    {170190+11, TxFifo_Full},   // 'e'
    {171720, TxFifo_NotEmpty},
    {171720+11, TxFifo_Full},   // 'g'
    {173242, TxFifo_NotEmpty},
    {173242+11, TxFifo_Full},   // 'i'
    {174772, TxFifo_NotEmpty},
    {174772+11, TxFifo_Full},   // 's'
    {176294, TxFifo_NotEmpty},
    {176294+11, TxFifo_Full},   // 't'
    {177816, TxFifo_NotEmpty},
    {177816+11, TxFifo_Full},   // 'e'
    {179346, TxFifo_NotEmpty},
    {179346+11, TxFifo_Full},   // 'r'
    {180868, TxFifo_NotEmpty},
    {180868+11, TxFifo_Full},   // '\''
    {182398, TxFifo_NotEmpty},
    {182398+11, TxFifo_Full},   // ' '
    {183920, TxFifo_NotEmpty},
    {183920+11, TxFifo_Full},   // 'a'
    {185442, TxFifo_NotEmpty},
    {185442+11, TxFifo_Full},   // 't'
    {186972, TxFifo_NotEmpty},
    {186972+11, TxFifo_Full},   // 't'
    {188494, TxFifo_NotEmpty},
    {188494+11, TxFifo_Full},   // 'r'
    {190024, TxFifo_NotEmpty},
    {190024+11, TxFifo_Full},   // 'i'
    {191546, TxFifo_NotEmpty},
    {191546+11, TxFifo_Full},   // 'b'
    {193068, TxFifo_NotEmpty},
    {193068+11, TxFifo_Full},   // 'u'
    {194598, TxFifo_NotEmpty},
    {194598+11, TxFifo_Full},   // 't'
    {196120, TxFifo_NotEmpty},
    {196120+11, TxFifo_Full},   // 'e'
    {197650, TxFifo_NotEmpty},
    {197650+11, TxFifo_Full},   // '\r'
    {199258, TxFifo_NotEmpty},
    {199258+11, TxFifo_Full},   // '\n'
    {200845, TxFifo_NotEmpty},
    {200845+11, TxFifo_Full},   // 'E'
    {202375, TxFifo_NotEmpty},
    {202375+11, TxFifo_Full},   // 'x'
    {203897, TxFifo_NotEmpty},
    {203897+11, TxFifo_Full},   // 'e'
    {205427, TxFifo_NotEmpty},
    {205427+11, TxFifo_Full},   // 'c'
    {206949, TxFifo_NotEmpty},
    {206949+11, TxFifo_Full},   // 'u'
    {208471, TxFifo_NotEmpty},
    {208471+11, TxFifo_Full},   // 't'
    {210001, TxFifo_NotEmpty},
    {210001+11, TxFifo_Full},   // 'i'
    {211523, TxFifo_NotEmpty},
    {211523+11, TxFifo_Full},   // 'o'
    {213053, TxFifo_NotEmpty},
    {213053+11, TxFifo_Full},   // 'n'
    {214575, TxFifo_NotEmpty},
    {214575+11, TxFifo_Full},   // ' '
    {216097, TxFifo_NotEmpty},
    {216097+11, TxFifo_Full},   // 's'
    {217627, TxFifo_NotEmpty},
    {217627+11, TxFifo_Full},   // 't'
    {219149, TxFifo_NotEmpty},
    {219149+11, TxFifo_Full},   // 'a'
    {220679, TxFifo_NotEmpty},
    {220679+11, TxFifo_Full},   // 'r'
    {222201, TxFifo_NotEmpty},
    {222201+11, TxFifo_Full},   // 't'
    {223723, TxFifo_NotEmpty},
    {223723+11, TxFifo_Full},   // 's'
    {225253, TxFifo_NotEmpty},
    {225253+11, TxFifo_Full},   // ','
    {226775, TxFifo_NotEmpty},
    {226775+11, TxFifo_Full},   // ' '
    {228305, TxFifo_NotEmpty},
    {228305+11, TxFifo_Full},   // '1'
    {229827, TxFifo_NotEmpty},
    {229827+11, TxFifo_Full},   // '6'
    {231349, TxFifo_NotEmpty},
    {231349+11, TxFifo_Full},   // '3'
    {232879, TxFifo_NotEmpty},
    {232879+11, TxFifo_Full},   // '8'
    {234401, TxFifo_NotEmpty},
    {234401+11, TxFifo_Full},   // '4'
    {235931, TxFifo_NotEmpty},
    {235931+11, TxFifo_Full},   // ' '
    {237453, TxFifo_NotEmpty},
    {237453+11, TxFifo_Full},   // 'r'
    {238975, TxFifo_NotEmpty},
    {238975+11, TxFifo_Full},   // 'u'
    {240505, TxFifo_NotEmpty},
    {240505+11, TxFifo_Full},   // 'n'
    {242027, TxFifo_NotEmpty},
    {242027+11, TxFifo_Full},   // 's'
    {243557, TxFifo_NotEmpty},
    {243557+11, TxFifo_Full},   // ' '
    {245079, TxFifo_NotEmpty},
    {245079+11, TxFifo_Full},   // 't'
    {246601, TxFifo_NotEmpty},
    {246601+11, TxFifo_Full},   // 'h'
    {248131, TxFifo_NotEmpty},
    {248131+11, TxFifo_Full},   // 'r'
    {249653, TxFifo_NotEmpty},
    {249653+11, TxFifo_Full},   // 'o'
    {251183, TxFifo_NotEmpty},
    {251183+11, TxFifo_Full},   // 'u'
    {252705, TxFifo_NotEmpty},
    {252705+11, TxFifo_Full},   // 'g'
    {254227, TxFifo_NotEmpty},
    {254227+11, TxFifo_Full},   // 'h'
    {255757, TxFifo_NotEmpty},
    {255757+11, TxFifo_Full},   // ' '
    {257279, TxFifo_NotEmpty},
    {257279+11, TxFifo_Full},   // 'D'
    {258809, TxFifo_NotEmpty},
    {258809+11, TxFifo_Full},   // 'h'
    {260331, TxFifo_NotEmpty},
    {260331+11, TxFifo_Full},   // 'r'
    {261853, TxFifo_NotEmpty},
    {261853+11, TxFifo_Full},   // 'y'
    {263383, TxFifo_NotEmpty},
    {263383+11, TxFifo_Full},   // 's'
    {264905, TxFifo_NotEmpty},
    {264905+11, TxFifo_Full},   // 't'
    {266435, TxFifo_NotEmpty},
    {266435+11, TxFifo_Full},   // 'o'
    {267957, TxFifo_NotEmpty},
    {267957+11, TxFifo_Full},   // 'n'
    {269479, TxFifo_NotEmpty},
    {269479+11, TxFifo_Full},   // 'e'
    {271009, TxFifo_NotEmpty},
    {271009+11, TxFifo_Full},   // 'o'
    {272371, TxFifo_NotEmpty},
    {272371+11, TxFifo_Full},   // 'o'
    {273703, TxFifo_NotEmpty},
    {273703+11, TxFifo_Full},   // 'o'
    {275042, TxFifo_NotEmpty},
    {275042+11, TxFifo_Full},   // 'o'
    {276381, TxFifo_NotEmpty},
    {276381+11, TxFifo_Full},   // 'o'
    {277720, TxFifo_NotEmpty},
    {277720+11, TxFifo_Full},   // 'o'
    {279052, TxFifo_NotEmpty},
    {279052+11, TxFifo_Full},   // 'o'
    {280391, TxFifo_NotEmpty},
    {280391+11, TxFifo_Full},   // 'o'
    {281734, TxFifo_NotEmpty},
    {281734+11, TxFifo_Full},   // 'o'
    {283067, TxFifo_NotEmpty},
    {283067+11, TxFifo_Full},   // 'o'
    {284407, TxFifo_NotEmpty},
    {284407+11, TxFifo_Full},   // 'o'
    {285747, TxFifo_NotEmpty},
    {285747+11, TxFifo_Full},   // 'o'

};
static const int DBG_TEST_TOTAL =
    static_cast<int>(sizeof(dbg_steps)/sizeof(dbg_state_type));
int dbg_tst_cnt = 0;
ETxFifoState dbg_state = TxFifo_Empty;
#endif

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

#ifdef GENERATE_PRECISE_DHRYSTONE_RTL_TRACE
    uint64_t tm = p->getExecCounter();
    if (dbg_tst_cnt < DBG_TEST_TOTAL) {
        if (tm >= dbg_steps[dbg_tst_cnt].tm_to_switch) {
            dbg_state = dbg_steps[dbg_tst_cnt++].next_state;
        }
    } else {
        dbg_state = TxFifo_Full;
    }
    switch (dbg_state) {
    case TxFifo_Empty:
        t.b.tx_fifo_empty = 1;
        t.b.tx_fifo_full = 0;
        break;
    case TxFifo_NotEmpty:
        t.b.tx_fifo_empty = 0;
        t.b.tx_fifo_full = 0;
        break;
    case TxFifo_Full:
        t.b.tx_fifo_empty = 0;
        t.b.tx_fifo_full = 1;
        break;
    default:;
    }
#else
    if (p->getTxTotal() == 0) {
        t.b.tx_fifo_empty = 1;
        t.b.tx_fifo_full = 0;
    } else if (p->getTxTotal() == FIFOSZ) {
        t.b.tx_fifo_empty = 0;
        t.b.tx_fifo_full = 1;
    } else {
        t.b.tx_fifo_empty = 0;
        t.b.tx_fifo_full = 0;
    }
#endif
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

