/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <api_types.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "openocdwrap.h"

namespace debugger {

OpenOcdWrapper::OpenOcdWrapper(const char *name) 
    : TcpClient(0, name),
    cmdInit_(this, static_cast<IJtag *>(this)),
    cmdReset_(this, static_cast<IJtag *>(this)),
    cmdResume_(this, static_cast<IJtag *>(this)),
    cmdHalt_(this, static_cast<IJtag *>(this)),
    cmdStatus_(this, static_cast<IJtag *>(this)),
    cmdReg_(this, static_cast<IJtag *>(this)),
    cmdRead_(this, static_cast<IJtag *>(this)),
    cmdWrite_(this, static_cast<IJtag *>(this)),
    cmdExit_(this, static_cast<IJtag *>(this)),
    cmdLog_(this, static_cast<IJtag *>(this)) {
    registerInterface(static_cast<IJtag *>(this));
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("PollingMs", &pollingMs_);
    registerAttribute("OpenOcdPath", &openOcdPath_);
    registerAttribute("OpenOcdScript", &openOcdScript_);
    registerAttribute("GdbMode", &gdbMode_);
    openocd_ = 0;
    emsgstate_ = MsgIdle;
    msgcnt_ = 0;
    pcmdGdb_ = 0;
    bbstate_ = IJtag::IDLE;
    memset(&scanreq_, 0, sizeof(scanreq_));

    RISCV_event_create(&eventJtagScanEnd_, "openocdwrap_jtagscan");
}

OpenOcdWrapper::~OpenOcdWrapper() {
    RISCV_event_close(&eventJtagScanEnd_);
    RISCV_event_close(&config_done_);
    if (openocd_) {
        delete openocd_;
    }
}

void OpenOcdWrapper::postinitService() {
    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
    } else {
        icmdexec_->registerCommand(&cmdInit_);
        icmdexec_->registerCommand(&cmdReset_);
        icmdexec_->registerCommand(&cmdResume_);
        icmdexec_->registerCommand(&cmdHalt_);
        icmdexec_->registerCommand(&cmdStatus_);
        icmdexec_->registerCommand(&cmdReg_);
        icmdexec_->registerCommand(&cmdRead_);
        icmdexec_->registerCommand(&cmdWrite_);
        icmdexec_->registerCommand(&cmdExit_);
    }

    // Run openocd as an external process using execv
    if (gdbMode_.to_bool()) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "%s.ext", getObjName());
        openocd_ = new ExternalProcessThread(this,
                                             tstr,
                                             openOcdPath_.to_string(),
                                             openOcdScript_.to_string());
        openocd_->run();
    }
    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }
}

void OpenOcdWrapper::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(&cmdInit_);
        icmdexec_->unregisterCommand(&cmdReset_);
        icmdexec_->unregisterCommand(&cmdResume_);
        icmdexec_->unregisterCommand(&cmdHalt_);
        icmdexec_->unregisterCommand(&cmdStatus_);
        icmdexec_->unregisterCommand(&cmdReg_);
        icmdexec_->unregisterCommand(&cmdRead_);
        icmdexec_->unregisterCommand(&cmdWrite_);
        icmdexec_->unregisterCommand(&cmdExit_);
    }
}

void OpenOcdWrapper::afterThreadStarted() {
    if (openocd_ && openocd_->isEnabled()) {
        openocd_->waitToStart();
        RISCV_sleep_ms(1000);

        // trying to connect to external openocd:4444 or 3333
        while (openocd_->isEnabled() && connectToServer() != 0) {
            RISCV_sleep_ms(1000);
        }
    } else {
        gdbMode_.make_boolean(false);
        targetPort_.make_int64(9824);       // connect ot BitBang server
        while (connectToServer() != 0) {
            RISCV_sleep_ms(1000);
        }
    }
}


void OpenOcdWrapper::ExternalProcessThread::busyLoop() {
    char tstr[4096];
    RISCV_sprintf(tstr, sizeof(tstr), "%s/openocd -f %s",
                path_.to_string(),
                script_.to_string());

    RISCV_event_set(&eventLoopStarted_);
    retcode_ = RISCV_system(tstr);
    RISCV_info("External OpenOCD was closed with code %d", retcode_);
    stop();
}

int OpenOcdWrapper::processRxBuffer(const char *buf, int sz) {
    RISCV_debug("%s", buf);

    if (isJtagEnabled()) {
        uint64_t tdi;
        for (int i = 0; i < sz; i++) {
            tdi = 0;
            if (buf[i] == '1') {
                tdi = 1;
            } else if (buf[i] != '0') {
                bool st = true;
            }
            dr_ >>= 1;
            dr_ |= tdi << (scanreq_.drlen - 1);
            if (++msgcnt_ >= scanreq_.drlen) {
                RISCV_event_set(&eventJtagScanEnd_);
                break;
            }
        }
    } else {
        for (int i = 0; i < sz; i++) {
            if (pcmdGdb_ == 0) {
                break;
            }

            msgbuf_[msgcnt_++] = buf[i];
            msgbuf_[msgcnt_] = '\0';

            switch (emsgstate_) {
            case MsgData:
                if (buf[i] == '#') {
                    emsgstate_ = MsgCrcHigh;
                }
                break;
            case MsgCrcHigh:
                emsgstate_ = MsgCrcLow;
                break;
            case MsgCrcLow:
                pcmdGdb_->handleResponse(msgbuf_, msgcnt_);
                emsgstate_ = MsgIdle;
                msgcnt_ = 0;
                break;
            default:
                if (buf[i] == '+') {
                    msgcnt_ = 0;
                    pcmdGdb_->setAck();
                } else if (buf[i] == '-') {
                    msgcnt_ = 0;
                    RISCV_error("command: %s was NAKed", pcmdGdb_->to_string());
                    writeTxBuffer(pcmdGdb_->to_string(),
                                  pcmdGdb_->getStringSize());
                } else if (buf[i] == '$') {
                    emsgstate_ = MsgData;
                } else {
                    msgcnt_ = 0;
                }
            }
        }

        if (pcmdGdb_ == 0
            || (pcmdGdb_->isAcked() && pcmdGdb_->isResponded())) {
            RISCV_event_set(&eventJtagScanEnd_);
        }
    }
    return sz;
}

void OpenOcdWrapper::resetAsync() {
    msgcnt_ = 0;
    scanreq_.drlen = 0;

    RISCV_event_clear(&eventJtagScanEnd_);
    writeTxBuffer("ur15R", 5);  // TRST=1,SRTS=1; TRST=0,SRST=0; RESET->IDLE; get TDO
    RISCV_event_wait(&eventJtagScanEnd_);
}

void OpenOcdWrapper::resetSync() {
    msgcnt_ = 0;
    scanreq_.drlen = 0;

    RISCV_event_clear(&eventJtagScanEnd_);
    writeTxBuffer("373737373737373737373715R", 25); // tms=1,tdo=1; RESET->IDLE; get TDO
    RISCV_event_wait(&eventJtagScanEnd_);
}

uint64_t OpenOcdWrapper::scan(uint32_t ir, uint64_t dr, int drlen) {
    char tms;
    char tdo;
    bool read_tdi;

    scanreq_.ir = ir;
    scanreq_.dr = dr;
    scanreq_.drlen = drlen;
    scanreq_.valid = true;
    char scanbuf[1024];
    int scanbuf_cnt = 0;

    while (scanreq_.valid == true || bbstate_ != IJtag::IDLE) {
        read_tdi = false;
        switch (bbstate_) {
        case IJtag::IDLE:
            tms = 0;
            tdo = 1;
            if (scanreq_.valid) {
                scanreq_.valid = false;
                tms = 1;
                tdo = 1;
                bbstate_ = IJtag::DRSCAN;
            }
            break;
        case IJtag::DRSCAN:
            if (ir_ != scanreq_.ir) {
                tms = 1;
                tdo = 1;
                bbstate_ = IJtag::IRSCAN;
            } else {
                tms = 0;
                tdo = 1;
                bbstate_ = IJtag::DRCAPTURE;
            }
            break;
        case IJtag::IRSCAN:
            tms = 0;
            tdo = 1;
            bbstate_ = IJtag::IRCAPTURE;
            break;
        case IJtag::IRCAPTURE:
            tms = 0;
            tdo = 1;
            ir_ = scanreq_.ir;
            ircnt_ = 0;
            bbstate_ = IJtag::IRSHIFT;
            break;
        case IJtag::IRSHIFT:
            tms = 0;
            tdo = (ir_ >> ircnt_) & 0x1;
            if (++ircnt_ == IRLEN) {
                tms = 1;
                bbstate_ = IJtag::IREXIT1;
            }
            break;
        case IJtag::IREXIT1:
            tms = 1;
            tdo = 1;
            bbstate_ = IJtag::IRUPDATE;
            break;
        case IJtag::IRUPDATE:
            tms = 1;
            tdo = 1;
            bbstate_ = IJtag::DRSCAN;
            break;

        case IJtag::DRCAPTURE:
            tms = 0;
            tdo = 1;
            dr_ = scanreq_.dr;
            drcnt_ = 0;
            bbstate_ = IJtag::DRSHIFT;
            break;
        case IJtag::DRSHIFT:
            tms = 0;
            tdo = dr_ & 0x1;
            dr_ >>= 1;
            read_tdi = true;

            if (++drcnt_ == scanreq_.drlen) {
                tms = 1;
                bbstate_ = IJtag::DREXIT1;
            }
            break;
        case IJtag::DREXIT1:
            tms = 1;
            tdo = 1;
            dr_ >>= 1;
            bbstate_ = IJtag::DRUPDATE;
            break;
        case IJtag::DRUPDATE:
            tms = 0;
            tdo = 1;
            bbstate_ = IJtag::IDLE;
            break;
        default:
            tms = 0;
            tdo = 1;
            bbstate_ = IJtag::IDLE;
        }
        scanbuf[scanbuf_cnt++] = '0' + ((tms << 1) | tdo);
        if (read_tdi) {
            scanbuf[scanbuf_cnt++] = 'R';                       // response
        }
        scanbuf[scanbuf_cnt++] = '4' + ((tms << 1) | tdo);      // tck posedge
    }

    msgcnt_ = 0;
    scanbuf[scanbuf_cnt] = 0;

    RISCV_event_clear(&eventJtagScanEnd_);
    writeTxBuffer(scanbuf, scanbuf_cnt);
    RISCV_event_wait(&eventJtagScanEnd_);
    return 0;
}

uint32_t OpenOcdWrapper::scanIdCode() {
    uint32_t ret = 0;

    scan(IJtag::IR_IDCODE, 0xFFFFFFFFFFFFFFFFull, 32);
    ret = static_cast<uint32_t>(dr_);
    RISCV_debug("TAP id = %08x", ret);
    return ret;
}


IJtag::DtmcsType OpenOcdWrapper::scanDtmcs() {
    IJtag::DtmcsType ret = {0};
    scan(IJtag::IR_DTMCS, 0, 32);
    ret.u32 = static_cast<uint32_t>(dr_);
    RISCV_debug("DTMCS = %08x: ver:%d, abits:%d, stat:%d",
            ret.u32, ret.bits.version, ret.bits.abits, ret.bits.dmistat);
    return ret;
}

uint32_t OpenOcdWrapper::scanDmi(uint32_t addr, uint32_t data, IJtag::EDmiOperation op) {
    IJtag::DmiType ret;
    uint64_t dr = addr;
    dr = (dr << 32) | data;
    dr = (dr << 2) | op;
    scan(IJtag::IR_DMI, dr, 34 + ABITS);

    // Do the same but with rena=0 and wena=0
    dr = static_cast<uint64_t>(addr) << 34;
    scan(IJtag::IR_DMI, dr, 34 + ABITS);
    ret.u64 = dr_;

    RISCV_debug("DMI [%02x] %08x, stat:%d",
            static_cast<uint32_t>(ret.bits.addr),
            static_cast<uint32_t>(ret.bits.data),
            static_cast<uint32_t>(ret.bits.status));
    return static_cast<uint32_t>(ret.bits.data);
}


void OpenOcdWrapper::exec(GdbCommandGeneric *cmd) {
    pcmdGdb_ = cmd;

    RISCV_event_clear(&eventJtagScanEnd_);
    writeTxBuffer(pcmdGdb_->to_string(),
                  pcmdGdb_->getStringSize());

    RISCV_event_wait(&eventJtagScanEnd_);
    pcmdGdb_ = 0;
}

/*void OpenOcdWrapper::resume() {
    if (openocd_ && openocd_->isEnabled()) {
        gdbReq_ = GdbCommand_Continue();
    } else {
        resetAsync();
        IJtag::DtmcsType dtmcs;
        dtmcs = scanDtmcs();
        IJtag::dmi_dmstatus_type dmstatus;
        dmstatus.u32 = read_dmi(IJtag::DMI_DMSTATUS);
        bool st = true;
    }
}

void OpenOcdWrapper::halt() {
    if (openocd_->isEnabled()) {
        gdbReq_ = GdbCommand_Halt();
    }
}

void OpenOcdWrapper::step() {
    if (openocd_->isEnabled()) {
        gdbReq_ = GdbCommand_Step();
    }
}*/

}  // namespace debugger
