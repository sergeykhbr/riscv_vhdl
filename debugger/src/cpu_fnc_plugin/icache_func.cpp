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

#include <api_core.h>
#include "icache_func.h"

namespace debugger {

ICacheFunctional::ICacheFunctional(const char *name) : IService(name),
    ICommand(name, 0) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("SysBus", &sysBus_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("TotalKBytes", &totalKBytes_);
    registerAttribute("LineBytes", &lineBytes_);
    registerAttribute("Ways", &ways_);
}

ICacheFunctional::~ICacheFunctional() {
}

void ICacheFunctional::postinitService() {
    isysbus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(sysBus_.to_string(), IFACE_MEMORY_OPERATION));
    if (!isysbus_) {
        RISCV_error("System Bus interface '%s' not found",
                    sysBus_.to_string());
        return;
    }

    icmdexec_ = static_cast<ICmdExecutor *>(
        RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
    } else {
        icmdexec_->registerCommand(static_cast<ICommand *>(this));
    }
}

void ICacheFunctional::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(static_cast<ICommand *>(this));
    }
}

ETransStatus ICacheFunctional::b_transport(Axi4TransactionType *trans) {
    return TRANS_OK;
}

int ICacheFunctional::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 2 && (*args)[1].is_string()) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void ICacheFunctional::exec(AttributeType *args, AttributeType *res) {
    if ((*args)[1].is_equal("test")) {
        runTest();
    }
}

void ICacheFunctional::runTest() {
    Axi4TransactionType tr;
    tr.action = MemAction_Read;
    tr.source_idx = 0;
    tr.wstrb = 0;
    tr.xsize = 4;
    for (unsigned i = 0; i < 100; i++) {
        tr.addr = (3*i) << 1;   // 1-byte alignment (uint16_t)
        b_transport(&tr);
    }
}

}  // namespace debugger
