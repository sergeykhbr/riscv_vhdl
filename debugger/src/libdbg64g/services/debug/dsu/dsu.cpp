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
#include "dsu.h"
#include "cmd_dsu_busutil.h"
#include "cmd_dsu_isrunning.h"
#include "cmd_dsu_run.h"
#include "cmd_dsu_halt.h"
#include "cmd_dsu_status.h"

namespace debugger {

DSU::DSU(const char *name) :
    RegMemBankGeneric(name),
    DsuRegisters(static_cast<IService *>(this)) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IDsuGeneric *>(this));
    registerAttribute("CPU", &cpu_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("Tap", &tap_);
    icpulist_.make_list(0);
    icmdlist_.make_list(0);;
    RISCV_event_create(&nb_event_, "DSU_event_nb");
}

DSU::~DSU() {
    RISCV_event_close(&nb_event_);
}

void DSU::postinitService() {
    RegMemBankGeneric::postinitService();

    itap_ = static_cast<ITap *>
            (RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    if (!itap_) {
        RISCV_error("Can't find ITap interface %s", tap_.to_string());
    }

    iexec_ = static_cast<ICmdExecutor *>(
        RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't find ICmdExecutor interface %s", cmdexec_.to_string());
    } else {
        icmdlist_.new_list_item().make_iface(new CmdDsuStatus(DSU_OFFSET, itap_));
        icmdlist_.new_list_item().make_iface(new CmdDsuBusUtil(DSU_OFFSET, itap_));
        icmdlist_.new_list_item().make_iface(new CmdDsuIsRunning(DSU_OFFSET, itap_));
        icmdlist_.new_list_item().make_iface(new CmdDsuStatus(DSU_OFFSET, itap_));
        icmdlist_.new_list_item().make_iface(new CmdDsuRun(DSU_OFFSET, itap_));
        for (unsigned i = 0; i < icmdlist_.size(); i++) {
            iexec_->registerCommand(static_cast<ICommand *>(icmdlist_[i].to_iface()));
        }
    }

    /*ICpuGeneric *icpu;
    for (unsigned i = 0; i < cpu_.size(); i++) {
        icpu = static_cast<ICpuGeneric *>(
            RISCV_get_service_iface(cpu_[i].to_string(), IFACE_CPU_GENERIC));
        if (!icpu) {
            RISCV_error("Can't find ICpuGeneric interface %s",
            cpu_[i].to_string());
        } else {
            AttributeType item;
            item.make_iface(icpu);
            icpulist_.add_to_list(&item);
        }
    }*/

    // Set default context
    //hartSelect(0);
}

void DSU::predeleteService() {
    for (unsigned i = 0; i < icmdlist_.size(); i++) {
        iexec_->unregisterCommand(static_cast<ICommand *>(icmdlist_[i].to_iface()));
    }
}

void DSU::nb_debug_write(unsigned hartid, uint16_t addr, uint64_t wdata) {
    /*if (static_cast<int>(hartid) >= hartTotal()) {
        RISCV_error("Debug Access index out of range %d", hartid);
        return;
    }
    ICpuGeneric *icpu = static_cast<ICpuGeneric *>(icpulist_[hartid].to_iface());
    nb_trans_.addr = addr;
    nb_trans_.wdata = wdata;
    nb_trans_.write = 1;
    nb_trans_.bytes = 8;

    RISCV_event_clear(&nb_event_);
    icpu->nb_transport_debug_port(&nb_trans_, static_cast<IDbgNbResponse *>(this));
    RISCV_event_wait(&nb_event_);*/
}

void DSU::incrementRdAccess(int mst_id) {
    bus_util_.getp()[2*mst_id + 1].val++;
}

void DSU::incrementWrAccess(int mst_id) {
    bus_util_.getp()[2*mst_id].val++;
}

/*void DSU::setResetPin(bool val) {
    IResetListener *irst;
    for (unsigned i = 0; i < cpu_.size(); i++) {
        irst = static_cast<IResetListener *>(
            RISCV_get_service_iface(cpu_[i].to_string(),
                                    IFACE_RESET_LISTENER));
        if (!irst) {
            RISCV_error("Can't find IResetListener interface %s",
                        cpu_[i].to_string());
        } else {
            irst->reset(static_cast<IService *>(this));
        }
    }
}

void DSU::hartSelect(int hartidx) {
    if (hartidx >= static_cast<int>(icpulist_.size())) {
        hartsel_ = icpulist_.size();
        RISCV_error("Context index out of range %d", hartidx);
        return;
    }
    ICpuGeneric *pcpu = static_cast<ICpuGeneric *>(icpulist_[hartidx].to_iface());
    hartsel_ = hartidx;
    dport_region_.setCpu(pcpu);
}

bool DSU::isHalted(int hartidx) {
    if (hartidx >= static_cast<int>(icpulist_.size())) {
        return false;
    }
    ICpuGeneric *pcpu = static_cast<ICpuGeneric *>(icpulist_[hartidx].to_iface());
    if (!pcpu) {
        return false;
    }
    return pcpu->isHalt();
}

void DSU::reqResume(int hartidx) {
    DMCONTROL_TYPE::ValueType t;
    t.val = dmcontrol_.getValue().val;
    t.bits.hartsello = hartidx;
    t.bits.hartselhi = hartidx >> 10;
    t.bits.resumereq = 1;

    nb_debug_write(static_cast<uint32_t>(hartidx),
                    -1,//CSR_runcontrol,
                    t.val);
}

void DSU::reqHalt(int hartidx) {
    DMCONTROL_TYPE::ValueType t;
    t.val = dmcontrol_.getValue().val;
    t.bits.hartsello = hartidx;
    t.bits.hartselhi = hartidx >> 10;
    t.bits.haltreq = 1;

   nb_debug_write(static_cast<uint32_t>(hartidx),
                  -1,//CSR_runcontrol,
                  t.val);
}
*/
}  // namespace debugger

