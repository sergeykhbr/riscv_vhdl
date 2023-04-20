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

OcdCmdResume::OcdCmdResume(OpenOcdWrapper *parent, IJtag *ijtag)
    : ICommand(parent, "test") {

    briefDescr_.make_string("Run simulation for a specify number of steps"
                            "or a specific addres\n");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run simulation for a specified number of steps.\n"
        "Usage:\n"
        "    c <N steps>\n"
        "Example:\n"
        "    c\n"
        "    c <addr>\n"
        "    c 1\n");
}


int OcdCmdResume::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (!cmdName_.is_equal(name.to_string())
        && !name.is_equal("c")) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void OcdCmdResume::exec(AttributeType *args, AttributeType *res) {
    OpenOcdWrapper *p = static_cast<OpenOcdWrapper *>(cmdParent_);
    res->attr_free();
    res->make_nil();

    p->resume();
}

OpenOcdWrapper::OpenOcdWrapper(const char *name) 
    : TcpClient(0, name) {
    registerAttribute("Jtag", &jtag_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("PollingMs", &pollingMs_);
    registerAttribute("OpenOcdPath", &openOcdPath_);
    registerAttribute("OpenOcdScript", &openOcdScript_);
    openocd_ = 0;
    estate_ = State_Connecting;
}

OpenOcdWrapper::~OpenOcdWrapper() {
    RISCV_event_close(&config_done_);
    if (openocd_) {
        delete openocd_;
    }
}

void OpenOcdWrapper::postinitService() {
    ijtag_ = static_cast<IJtag *>
            (RISCV_get_service_iface(jtag_.to_string(), IFACE_JTAG));

    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
    } else {
        pcmdResume_ = new OcdCmdResume(this, 0);
        icmdexec_->registerCommand(pcmdResume_);
    }

    // Run openocd as an external process using execv
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s.ext", getObjName());
    openocd_ = new ExternalProcessThread(this,
                                         tstr,
                                         openOcdPath_.to_string(),
                                         openOcdScript_.to_string());
    openocd_->run();
    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }
}

void OpenOcdWrapper::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(pcmdResume_);
        delete pcmdResume_;
    }
}

void OpenOcdWrapper::afterThreadStarted() {
    openocd_->waitToStart();
    RISCV_sleep_ms(1000);

    // trying to connect to external openocd:4444 or 3333
    while (openocd_->isEnabled() && connectToServer() != 0) {
        RISCV_sleep_ms(1000);
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
    switch (estate_) {
    case State_Connecting:
        if (sz == 1 && buf[0] == '+') {
            RISCV_info("%s", "Disabling ACK mode");
            writeTxBuffer("+", 1);

            gdbCmd_ = GdbCommand_QStartNoAckMode();
            writeTxBuffer(gdbCmd_.to_string(),
                          gdbCmd_.getStringSize());
            estate_ = State_Idle;
        }
        break;
    case State_Idle:
        break;
    default:;
    }

    return 0;
}

void OpenOcdWrapper::resume() {
    if (openocd_->isEnabled()) {
        gdbCmd_ = GdbCommand_Continue();
        writeTxBuffer(gdbCmd_.to_string(), gdbCmd_.getStringSize());
    }
}
void OpenOcdWrapper::halt() {
    if (openocd_->isEnabled()) {
        gdbCmd_ = GdbCommand_vCtrlC();
        writeTxBuffer(gdbCmd_.to_string(), gdbCmd_.getStringSize());
    }
}


}  // namespace debugger
