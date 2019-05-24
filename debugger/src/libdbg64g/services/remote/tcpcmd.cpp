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

#include "tcpcmd.h"

namespace debugger {

TcpCommands::TcpCommands(IService *parent) : IHap(HAP_All) {
    parent_ = parent;
    rxcnt_ = 0;

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

TcpCommands::~TcpCommands() {
    RISCV_event_close(&eventHalt_);
    RISCV_event_close(&eventDelayMs_);
    RISCV_event_close(&eventPowerChanged_);
}

void TcpCommands::setPlatformConfig(AttributeType *cfg) {
    platformConfig_ = *cfg;
}

void TcpCommands::hapTriggered(IFace *isrc, EHapType type,
                                const char *descr) {
    if (type == HAP_Halt) {
        RISCV_event_set(&eventHalt_);
    } else if (type == HAP_CpuTurnON || type == HAP_CpuTurnOFF) {
        RISCV_event_set(&eventPowerChanged_);
    }
}

void TcpCommands::stepCallback(uint64_t t) {
    RISCV_event_set(&eventDelayMs_);
}

void TcpCommands::updateData(const char *buf, int buflen) {
    for (int i = 0; i < buflen; i++) {
        rxbuf_[rxcnt_++] = buf[i];
        if (buf[i] == 0) {
            processCommand();
            rxcnt_ = 0;
        }
    }
}

void TcpCommands::processCommand() {
    AttributeType cmd;
    cmd.from_config(rxbuf_);
    if (!cmd.is_list() || cmd.size() < 3) {
        return;
    }

    uint64_t idx = cmd[0u].to_uint64();
    resp_.make_list(2);
    resp_[0u].make_uint64(idx);
    resp_[1].make_string("OK");

    AttributeType &requestType = cmd[1];
    AttributeType &requestAction = cmd[2];
    AttributeType *resp = &resp_[1];

    if (requestType.is_equal("Configuration")) {
        resp->clone(&platformConfig_);
    } else if (requestType.is_equal("Command")) {
        /** Redirect command to console directly */
        iexec_->exec(requestAction.to_string(), resp, false);
        if (igui_) {
            igui_->externalCommand(&requestAction);
        }
    } else if (requestType.is_equal("Breakpoint")) {
        /** Breakpoints action */
        if (requestAction[0u].is_equal("Add")) {
            br_add(requestAction[1], resp);
        } else if (requestAction[0u].is_equal("Remove")) {
            br_rm(requestAction[1], resp);
        } else {
            resp->make_string("Wrong breakpoint command");
        }
    } else if (requestType.is_equal("Control")) {
        /** Run Control action */
        if (requestAction[0u].is_equal("GoUntil")) {
            go_until(requestAction[1], resp);
        } else if (requestAction[0u].is_equal("GoMsec")) {
            go_msec(requestAction[1], resp);
        } else if (requestAction[0u].is_equal("Step")) {
            step(requestAction[1].to_int(), resp);
        } else if (requestAction[0u].is_equal("PowerOn")) {
            power_on(resp);
            RISCV_debug("[%" RV_PRI64 "d] Command Power-On", idx);
        } else if (requestAction[0u].is_equal("PowerOff")) {
            power_off(resp);
            RISCV_debug("[%" RV_PRI64 "d] Command Power-Off", idx);
        } else {
            resp->make_string("Wrong control command");
        }
    } else if (requestType.is_equal("Status")) {
        /** Pump status */
        if (requestAction.is_equal("IsON")) {
            resp->make_boolean(true);
        } else if (requestAction.is_equal("IsHalt")) {
            //resp->make_boolean(iriscv_->isHalt());
        } else if (requestAction.is_equal("Steps")) {
            resp->make_uint64(iclk_->getStepCounter());
        } else if (requestAction.is_equal("TimeSec")) {
            double t1 = iclk_->getStepCounter() / iclk_->getFreqHz();
            resp->make_floating(t1);
        } else {
            resp->make_string("Wrong status command");
        }
    } else if (requestType.is_equal("Symbol")) {
        /** Symbols table conversion */
        if (requestAction[0u].is_equal("ToAddr")) {
            symb2addr(requestAction[1].to_string(), resp);
        } else if (requestAction[0u].is_equal("FromAddr")) {
            // todo:
        } else {
            resp->make_string("Wrong symbol command");
        }
    } else if (requestType.is_equal("Attribute")) {
        IService *isrv = static_cast<IService *>(
                        RISCV_get_service(requestAction[0u].to_string()));
        if (isrv) {
            AttributeType *iatr = static_cast<AttributeType *>(
                        isrv->getAttribute(requestAction[1].to_string()));
            if (iatr) {
                resp->clone(iatr);
            } else {
                resp->make_string("Attribute not found");
            }
        } else {
            resp->make_string("Service not found");
        }
    } else {
        resp->make_list(2);
        (*resp)[0u].make_string("ERROR");
        (*resp)[1].make_string("Wrong command format");
    }
    resp_.to_config();
}

AttributeType *TcpCommands::response() {
    return &resp_;
}

void TcpCommands::br_add(const AttributeType &symb, AttributeType *res) {
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

void TcpCommands::br_rm(const AttributeType &symb, AttributeType *res) {
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

void TcpCommands::step(int cnt, AttributeType *res) {
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

void TcpCommands::go_until(const AttributeType &symb, AttributeType *res) {
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

void TcpCommands::symb2addr(const char *symbol, AttributeType *res) {
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

void TcpCommands::power_on(AttributeType *res) {
}

void TcpCommands::power_off(AttributeType *res) {
}

void TcpCommands::go_msec(const AttributeType &msec, AttributeType *res) {
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
