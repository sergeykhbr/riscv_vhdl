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

#include "tcpcmd_gen.h"

namespace debugger {

TcpCommandsGen::TcpCommandsGen(IService *parent) : IHap(HAP_All) {
    parent_ = parent;
    rxcnt_ = 0;
    estate_ = State_Idle;

    resptotal_ = 1 << 18;   // should re-allocated if need in childs
    respcnt_ = 0;
    respbuf_ = new char[resptotal_];

    cpu_.make_string("core0");
    executor_.make_string("cmdexec0");
    source_.make_string("src0");
    gui_.make_string("gui0");

    iexec_ = static_cast<ICmdExecutor *>(
        RISCV_get_service_iface(executor_.to_string(), IFACE_CMD_EXECUTOR));

    igui_ = static_cast<IGui *>(
        RISCV_get_service_iface(gui_.to_string(), IFACE_GUI_PLUGIN));

    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CLOCK));
    IService *iservcpu = static_cast<IService *>(
                        RISCV_get_service(cpu_.to_string()));
    cpuLogLevel_ = static_cast<AttributeType *>(
                        iservcpu->getAttribute("LogLevel"));

    icpugen_ = static_cast<ICpuGeneric *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CPU_GENERIC));

    icpufunc_ = static_cast<ICpuFunctional *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CPU_FUNCTIONAL));

    isrc_ = static_cast<ISourceCode *>(
        RISCV_get_service_iface(source_.to_string(), IFACE_SOURCE_CODE));

    char tstr[128];
    RISCV_sprintf(tstr, sizeof(tstr), "%s_halt", parent_->getObjName());
    RISCV_event_create(&eventHalt_, tstr);
    RISCV_sprintf(tstr, sizeof(tstr), "%s_delay_ms", parent_->getObjName());
    RISCV_event_create(&eventDelayMs_, tstr);
    RISCV_sprintf(tstr, sizeof(tstr), "%s_pwr", parent_->getObjName());
    RISCV_event_create(&eventPowerChanged_, tstr);

    RISCV_register_hap(static_cast<IHap *>(this));
}

TcpCommandsGen::~TcpCommandsGen() {
    RISCV_event_close(&eventHalt_);
    RISCV_event_close(&eventDelayMs_);
    RISCV_event_close(&eventPowerChanged_);
    respcnt_ = 0;
    resptotal_ = 0;
    delete [] respbuf_;
}

void TcpCommandsGen::setPlatformConfig(AttributeType *cfg) {
    platformConfig_ = *cfg;
}

void TcpCommandsGen::hapTriggered(IFace *isrc, EHapType type,
                                const char *descr) {
    if (type == HAP_Halt) {
        RISCV_event_set(&eventHalt_);
    } else if (type == HAP_CpuTurnON || type == HAP_CpuTurnOFF) {
        RISCV_event_set(&eventPowerChanged_);
    }
}

void TcpCommandsGen::stepCallback(uint64_t t) {
    RISCV_event_set(&eventDelayMs_);
}

int TcpCommandsGen::updateData(const char *buf, int buflen) {
    int ret = 0;
    for (int i = 0; i < buflen; i++) {
        switch (estate_) {
        case State_Idle:
            if (isStartMarker(buf[i])) {
                rxbuf_[rxcnt_++] = buf[i];
                estate_ = State_Started;
            }
            break;
        case State_Started:
            if (rxcnt_ < static_cast<int>(sizeof(rxbuf_))) {
                rxbuf_[rxcnt_++] = buf[i];
                rxbuf_[rxcnt_] = '\0';
            } else {
                RISCV_error("rxbuf_ overflow %d", rxcnt_);
            }
            if (isEndMarker(rxbuf_, rxcnt_)) {
                estate_ = State_Ready;
            }
            break;
        default:;
        }

        if (estate_ == State_Ready) {
            processCommand(rxbuf_, rxcnt_);
            rxcnt_ = 0;
            estate_ = State_Idle;
            ret = i + 1;  // take into account the last symbol
        }
    }
    return ret;
}

void TcpCommandsGen::br_add(const AttributeType &symb, AttributeType *res) {
    uint64_t addr;
    if (symb.is_string()) {
        AttributeType t1;
        symb2addr(symb.to_string(), &t1);
        if (t1.is_nil()) {
            res->make_string("br_add: Symbol not found");
            return;
        }
        addr = t1.to_uint64();
    } else if (symb.is_integer()) {
        addr = symb.to_uint64();
    } else {
        res->make_string("br_add: Wrong format");
        return;
    }
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "br add 0x%x", addr);
    iexec_->exec(tstr, res, false);
}

void TcpCommandsGen::br_rm(const AttributeType &symb, AttributeType *res) {
    uint64_t addr;
    if (symb.is_string()) {
        AttributeType t1;
        symb2addr(symb.to_string(), &t1);
        if (t1.is_nil()) {
            res->make_string("br_rm: Symbol not found");
            return;
        }
        addr = t1.to_uint64();
    } else if (symb.is_integer()) {
        addr = symb.to_uint64();
    } else {
        res->make_string("br_rm: Wrong format");
        return;
    }
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "br rm 0x%x", addr);
    iexec_->exec(tstr, res, false);
}

void TcpCommandsGen::step(int cnt, AttributeType *res) {
    char tstr[128];
    RISCV_sprintf(tstr, sizeof(tstr), "c %d", cnt);

    int log_level_old = cpuLogLevel_->to_int();
    if (cnt < 10) {
        cpuLogLevel_->make_int64(4);
    }

    RISCV_event_clear(&eventHalt_);
    iexec_->exec(tstr, res, false);
    RISCV_event_wait(&eventHalt_);
    cpuLogLevel_->make_int64(log_level_old);
}

void TcpCommandsGen::go_until(const AttributeType &symb, AttributeType *res) {
    uint64_t addr;
    if (symb.is_string()) {
        AttributeType t1;
        symb2addr(symb.to_string(), &t1);
        if (t1.is_nil()) {
            res->make_string("br_rm: Symbol not found");
            return;
        }
        addr = t1.to_uint64();
    } else if (symb.is_integer()) {
        addr = symb.to_uint64();
    } else {
        res->make_string("br_rm: Wrong format");
        return;
    }
    // Add breakpoint
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "br add 0x%x", addr);
    iexec_->exec(tstr, res, false);

    // Set CPU LogLevel=1 to hide all debugging messages
    int log_level_old = cpuLogLevel_->to_int();
    cpuLogLevel_->make_int64(1);

    // Run simulation
    RISCV_event_clear(&eventHalt_);
    RISCV_sprintf(tstr, sizeof(tstr), "c", 0);
    iexec_->exec(tstr, res, false);
    RISCV_event_wait(&eventHalt_);
    cpuLogLevel_->make_int64(log_level_old);

    // Remove breakpoint:
    RISCV_sprintf(tstr, sizeof(tstr), "br rm 0x%x", addr);
    iexec_->exec(tstr, res, false);
}

void TcpCommandsGen::symb2addr(const char *symbol, AttributeType *res) {
    res->make_nil();
    if (!isrc_) {
        return;
    }
    // Letters capitalization:
    char capital[256];
    int i = 0;
    while (symbol[i]) {
        capital[i] = symbol[i];
        if (symbol[i] >= 'a' && symbol[i] <= 'z') {
            capital[i] += ('A' - 'a');
        }
        i++;
        capital[i] = '\0';
    }
    uint64_t addr;
    if (isrc_->symbol2Address(capital, &addr) == 0) {
        res->make_uint64(addr);
        return;
    }
}

void TcpCommandsGen::power_on(const char *btn_name, AttributeType *res) {
    if (icpufunc_->isOn()) {
        res->make_string("Already ON");
        return;
    }
    char tstr[256];
    AttributeType t1;
    RISCV_event_clear(&eventPowerChanged_);
    RISCV_sprintf(tstr, sizeof(tstr), "%s press", btn_name);
    iexec_->exec(tstr, &t1, false);
    RISCV_event_wait(&eventPowerChanged_);
    RISCV_sprintf(tstr, sizeof(tstr), "%s release", btn_name);
    iexec_->exec(tstr, &t1, false);
    res->make_string("OK");
}

void TcpCommandsGen::power_off(const char *btn_name, AttributeType *res) {
    if (!icpufunc_->isOn()) {
        res->make_string("Already OFF");
        return;
    }
    char tstr[256];
    AttributeType t1;
    RISCV_event_clear(&eventPowerChanged_);
    RISCV_sprintf(tstr, sizeof(tstr), "%s press", btn_name);
    iexec_->exec(tstr, &t1, false);
    iexec_->exec("c", &t1, false);
    RISCV_event_wait(&eventPowerChanged_);
    RISCV_sprintf(tstr, sizeof(tstr), "%s release", btn_name);
    iexec_->exec(tstr, &t1, false);
    res->make_string("OK");
}

void TcpCommandsGen::go_msec(const AttributeType &msec, AttributeType *res) {
    char tstr[256];
    double delta = 0.001 * iclk_->getFreqHz() * msec.to_float();
    if (delta == 0) {
        delta = 1;
    }
    RISCV_sprintf(tstr, sizeof(tstr),
        "c %" RV_PRI64 "d", static_cast<uint64_t>(delta));

    RISCV_event_clear(&eventHalt_);
    iexec_->exec(tstr, res, false);
    RISCV_event_wait(&eventHalt_);
}

}  // namespace debugger
