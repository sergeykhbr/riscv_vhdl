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
    : TcpClient(0, name) {
    registerAttribute("Jtag", &jtag_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("PollingMs", &pollingMs_);
    registerAttribute("OpenOcdPath", &openOcdPath_);
    registerAttribute("OpenOcdScript", &openOcdScript_);
    openocd_ = 0;
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

void OpenOcdWrapper::beforeThreadClosing() {
    // Gracefully close external openocd:
    if (openocd_->getRetCode() == 0) {
        char tstr[64];
        //int tsz = RISCV_sprintf(tstr, sizeof(tstr), "%s\n", "shutdown");
        int tsz = RISCV_sprintf(tstr, sizeof(tstr), "%s", "$D#00qQexitclose");
        writeTxBuffer(tstr, tsz);
        sendData();
    }
}

void OpenOcdWrapper::ExternalProcessThread::busyLoop() {
    char tstr[4096];
    RISCV_sprintf(tstr, sizeof(tstr), "%s/openocd -f %s",
                path_.to_string(),
                script_.to_string());

    RISCV_event_set(&eventLoopStarted_);
    retcode_ = RISCV_system(tstr);
    RISCV_info("External OpenOCD was closed with the return code %d", retcode_);
    stop();
}

int OpenOcdWrapper::processRxBuffer(const char *buf, int sz) {
    bool st = true;
    if (sz == 1 && buf[0] == '+') {
        writeTxBuffer("+", 1);
    }
    return 0;
}

}  // namespace debugger
