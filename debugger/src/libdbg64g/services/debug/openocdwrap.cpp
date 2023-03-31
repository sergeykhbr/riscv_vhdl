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
    : IService(name), IHap(HAP_All) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Jtag", &jtag_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("PollingMs", &pollingMs_);

    RISCV_event_create(&config_done_, "openocdwrap_config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    isEnable_.make_boolean(true);
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

void OpenOcdWrapper::hapTriggered(EHapType type, 
                                  uint64_t param,
                                  const char *descr) {
    if (type == HAP_ConfigDone) {
        RISCV_event_set(&config_done_);
    }
}

void OpenOcdWrapper::busyLoop() {
    RISCV_event_wait(&config_done_);

    pid_ = RISCV_get_pid();
    const char *argv[] = {"C:/Projects/riscv_vhdl/openocd_gdb_cfg/openocd.exe"
                          "-f",
                          "C:/Projects/riscv_vhdl/openocd_gdb_cfg/bitbang_gdb.cfg"};

    //int retcode = execve(argv[0],
    //                      argv,
    //                      NULL);
    //int retcode = system("C:/Projects/riscv_vhdl/openocd_gdb_cfg/openocd.exe -f C:/Projects/riscv_vhdl/openocd_gdb_cfg/bitbang_gdb.cfg");

    while (isEnabled()) {
        RISCV_sleep_ms(pollingMs_.to_int());
    }
}

void OpenOcdWrapper::stop() {
    IThread::stop();
    CloseHandle(threadInit_.Handle);
    
    //signal(pid_, SIGBREAK);
}

}  // namespace debugger
