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
#include "dsumap.h"

namespace debugger {

DSU::DSU(const char *name) :
    RegMemBankGeneric(name),
    DsuRegisters(static_cast<IService *>(this)) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IDsuGeneric *>(this));
    registerAttribute("CPU", &cpu_);
    icpulist_.make_list(0);
    RISCV_event_create(&nb_event_, "DSU_event_nb");
}

DSU::~DSU() {
    RISCV_event_close(&nb_event_);
}

void DSU::postinitService() {
    RegMemBankGeneric::postinitService();

    ICpuGeneric *icpu;
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
    }

    // Set default context
    setCpuContext(0);
}

void DSU::nb_response_debug_port(DebugPortTransactionType *trans) {
    RISCV_event_set(&nb_event_);
}

void DSU::nb_debug_write(unsigned hartid, uint16_t addr, uint64_t wdata) {
    if (hartid >= getCpuTotal()) {
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
    RISCV_event_wait(&nb_event_);
}

void DSU::incrementRdAccess(int mst_id) {
    bus_util_.getp()[2*mst_id + 1].val++;
}

void DSU::incrementWrAccess(int mst_id) {
    bus_util_.getp()[2*mst_id].val++;
}

void DSU::softReset(bool val) {
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

void DSU::setCpuContext(unsigned n) {
    if (n >= icpulist_.size()) {
        hartsel_ = icpulist_.size();
        RISCV_error("Context index out of range %d", n);
        return;
    }
    ICpuGeneric *pcpu = static_cast<ICpuGeneric *>(icpulist_[n].to_iface());
    hartsel_ = n;
    dport_region_.setCpu(pcpu);
}

bool DSU::isCpuHalted(unsigned idx) {
    if (idx >= icpulist_.size()) {
        return false;
    }
    ICpuGeneric *pcpu = static_cast<ICpuGeneric *>(icpulist_[idx].to_iface());
    if (!pcpu) {
        return false;
    }
    return pcpu->isHalt();
}

}  // namespace debugger

