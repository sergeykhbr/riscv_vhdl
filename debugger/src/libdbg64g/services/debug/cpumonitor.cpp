/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "cpumonitor.h"
#include "coreservices/isrccode.h"

namespace debugger {

CpuMonitor::CpuMonitor(const char *name) 
    : IService(name), IHap(HAP_All) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("PollingMs", &pollingMs_);

    RISCV_event_create(&config_done_, "cpumonitor_config_done");
    RISCV_mutex_init(&mutex_resume_);
    RISCV_register_hap(static_cast<IHap *>(this));
    hartsel_ = 0;
}

CpuMonitor::~CpuMonitor() {
    RISCV_event_close(&config_done_);
    RISCV_mutex_destroy(&mutex_resume_);
}

void CpuMonitor::postinitService() {
    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
        return;
    }

    haltsum_ = getStatus();;

    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }
}

void CpuMonitor::hapTriggered(EHapType type, 
                              uint64_t param,
                              const char *descr) {
    if (type == HAP_ConfigDone) {
        RISCV_event_set(&config_done_);
    } else if (type == HAP_CpuContextChanged) {
        hartsel_ = param;
    } else if (type == HAP_Resume) {
        writeBreakpoints();

        // CPU can halt faster than than we poll bits
        RISCV_mutex_lock(&mutex_resume_);
        haltsum_ &= ~(1ull << (hartsel_ & 0x3f));
        RISCV_mutex_unlock(&mutex_resume_);
    }
}

void CpuMonitor::busyLoop() {
    uint64_t status;
    uint64_t mask;
    uint64_t t1;
    RISCV_event_wait(&config_done_);

    while (isEnabled()) {
        RISCV_sleep_ms(pollingMs_.to_int());

        status = getStatus();

        RISCV_mutex_lock(&mutex_resume_);
        mask = 1ull << (hartsel_ & 0x3f);
        t1 = haltsum_;
        haltsum_ = status;
        RISCV_mutex_unlock(&mutex_resume_);

        if (((t1 ^ status) & mask) != 0) {
            if (status & mask) {
                removeBreakpoints();
                RISCV_trigger_hap(HAP_Halt,
                                  hartsel_,
                                  "Selected core is halted");
            }
        }
    }
}

uint64_t CpuMonitor::getStatus() {
    icmdexec_->exec("status", &statusResponse_, true);
    return statusResponse_.to_uint64();
}

void CpuMonitor::executeProgBuffer(uint64_t addr,
                                  uint32_t opcode,
                                  uint32_t oplen) {
    RISCV_error("TODO progbuf exec: 0x%08" RV_PRI64 "x", addr);
}

void CpuMonitor::writeBreakpoints() {
    uint64_t npc;
    uint64_t br_addr;
    uint64_t br_flags;
    uint32_t br_opcode;   // ebreak opcode
    uint32_t br_oplen;
    char tstr[128];

    icmdexec_->exec("reg npc", &npcResponse_, true);
    icmdexec_->exec("br", &brList_, true);

    npc = npcResponse_.to_uint64();
    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        br_addr = br[BrkList_address].to_uint64();
        br_flags = br[BrkList_flags].to_uint64();
        br_opcode = br[BrkList_opcode].to_uint32();
        br_oplen = br[BrkList_oplen].to_uint32();

        if (npc == br_addr) {
            uint32_t br_instr = br[BrkList_instr].to_uint32();
            executeProgBuffer(br_addr, br_instr, br_oplen);
        }

        if (br_flags & BreakFlag_HW) {
            // TODO: triggers
        } else {
            RISCV_sprintf(tstr, sizeof(tstr),
                    "write 0x%08" RV_PRI64 "x %d 0x%x",
                    br_addr, br_oplen, br_opcode);
            icmdexec_->exec(tstr, &writeMemResp_, true);

            RISCV_error("Write breakpoint 0x%08" RV_PRI64 "x", br_addr);
        }
    }
}

void CpuMonitor::removeBreakpoints() {
    uint64_t br_addr;
    uint32_t br_instr;      // original instruction opcode
    uint64_t br_flags;
    uint32_t br_oplen;
    char tstr[128];

    icmdexec_->exec("br", &brList_, true);

    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        br_addr = br[BrkList_address].to_uint64();
        br_instr = br[BrkList_instr].to_uint32();
        br_flags = br[BrkList_flags].to_uint64();
        br_oplen = br[BrkList_oplen].to_uint32();

        if (br_flags & BreakFlag_HW) {
            // TODO: triggers
        } else {
            RISCV_sprintf(tstr, sizeof(tstr),
                    "write 0x%08" RV_PRI64 "x %d 0x%x",
                    br_addr, br_oplen, br_instr);
            icmdexec_->exec(tstr, &writeMemResp_, true);

            RISCV_error("Remove breakpoint 0x%08" RV_PRI64 "x", br_addr);
        }
    }
}

}  // namespace debugger
