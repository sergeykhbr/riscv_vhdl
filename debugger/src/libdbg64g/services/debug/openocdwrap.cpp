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
    : TcpClient(0, name, "127.0.0.1", 4444),
    openocd_(this) {
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Jtag", &jtag_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("PollingMs", &pollingMs_);

}

OpenOcdWrapper::~OpenOcdWrapper() {
    RISCV_event_close(&config_done_);
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

    openocd_.run();
    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void OpenOcdWrapper::predeleteService() {
    getenv("OPENOCD_PATH");
    if (icmdexec_) {
    }
}


void OpenOcdWrapper::busyLoop() {
    // trying to connect to openocd:4444
    while (openocd_.isOpened() && connectToServer() != 0) {
        RISCV_sleep_ms(1000);
    }

    while (isEnabled() && openocd_.isOpened()) {
        // Just wait exit to send 'shutdown'
        RISCV_sleep_ms(pollingMs_.to_int());
    }

    char tstr[64];
    int tsz = RISCV_sprintf(tstr, sizeof(tstr), "%s", "shutdown");
    writeTxBuffer(tstr, tsz);
    sendData();
}

void OpenOcdWrapper::ExternalProcessThread::busyLoop() {
#if 0
    const char *argv[] = {"C:/Projects/riscv_vhdl/openocd_gdb_cfg/openocd.exe", 
                          "-f",
                          "C:/Projects/riscv_vhdl/openocd_gdb_cfg/bitbang_gdb.cfg",
                          NULL};
    int retcode = execv(argv[0], argv);
#endif
    int retcode = system("C:/Projects/riscv_vhdl/openocd_gdb_cfg/openocd.exe"
                " -f C:/Projects/riscv_vhdl/openocd_gdb_cfg/bitbang_gdb.cfg");
    opened_ = false;
}

}  // namespace debugger
