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

#include "api_core.h"
#include "rtl_wrapper.h"
#include "iservice.h"
#include "ihap.h"
#include <riscv-isa.h>
#include "coreservices/iserial.h"

namespace debugger {

RtlWrapper::RtlWrapper(IFace *parent, sc_module_name name) : sc_module(name),
    o_clk("clk", 10, SC_NS) {
    iparent_ = parent;
    generate_ref_ = false;
    clockCycles_ = 1000000; // 1 MHz when default resolution = 1 ps

    async_nrst = 1;//0;
    w_nrst = 1;
    v.nrst = 1;//0;
    v.req_addr = 0;
    v.req_len = 0;
    v.req_burst = 0;
    v.req_write = 0;
    v.store_fault = 0;
    v.store_addr = 0;
    v.interrupt = false;
    async_interrupt = 0;
    w_interrupt = 0;
    v.halted = false;
    v.state = State_Idle;
    r.state = State_Idle;
    RISCV_event_create(&dport_.valid, "dport_valid");
    dport_.trans_idx_up = 0;
    dport_.trans_idx_down = 0;
    trans.source_idx = 0;//CFG_NASTI_MASTER_CACHED;

    SC_METHOD(comb);
    sensitive << w_nrst;
    sensitive << w_interrupt;
    sensitive << i_halted;
    sensitive << i_req_mem_valid;
    sensitive << i_req_mem_write;
    sensitive << i_req_mem_addr;
    sensitive << i_req_mem_strob;
    sensitive << i_req_mem_data;
    sensitive << i_req_mem_len;
    sensitive << i_req_mem_burst;
    sensitive << i_halted;
    sensitive << r.req_addr;
    sensitive << r.req_len;
    sensitive << r.req_burst;
    sensitive << r.req_write;
    sensitive << r.store_fault;
    sensitive << r.store_addr;
    sensitive << r.nrst;
    sensitive << r.interrupt;
    sensitive << r.state;
    sensitive << r.halted;
    sensitive << w_resp_valid;
    sensitive << wb_resp_data;
    sensitive << w_resp_store_fault;
    sensitive << w_resp_load_fault;
    sensitive << w_dport_valid;
    sensitive << w_dport_write;
    sensitive << wb_dport_region;
    sensitive << wb_dport_addr;
    sensitive << wb_dport_wdata;

    SC_METHOD(registers);
    sensitive << o_clk.posedge_event();

    SC_METHOD(sys_bus_proc);
    sensitive << bus_event_;
}

RtlWrapper::~RtlWrapper() {
    RISCV_event_close(&dport_.valid);
}

void RtlWrapper::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, w_nrst, "wrapper0/w_nrst");
        sc_trace(o_vcd, r.nrst, "wrapper0/r_nrst");
        sc_trace(o_vcd, r.state, "wrapper0/r_state");
        sc_trace(o_vcd, r.req_addr, "wrapper0/r_req_addr");
        sc_trace(o_vcd, r.req_write, "wrapper0/r_req_write");
        sc_trace(o_vcd, r.req_len, "wrapper0/r_req_len");
        sc_trace(o_vcd, r.req_burst, "wrapper0/r_req_burst");
        sc_trace(o_vcd, i_req_mem_strob, "wrapper0/i_req_mem_strob");
        sc_trace(o_vcd, i_req_mem_data, "wrapper0/i_req_mem_data");
        sc_trace(o_vcd, o_resp_mem_data_valid, "wrapper0/o_resp_mem_data_valid");
        sc_trace(o_vcd, o_resp_mem_data, "wrapper0/o_resp_mem_data");
    }
}

void RtlWrapper::clk_gen() {
    // todo: instead sc_clock
}

void RtlWrapper::comb() {
    bool w_req_mem_ready;
    sc_uint<BUS_ADDR_WIDTH> vb_req_addr;

    w_req_mem_ready = 0;

    v.interrupt = w_interrupt;
    v.halted = i_halted.read();
    v.store_fault = w_resp_store_fault;
    v.store_addr = wb_resp_store_fault_addr;

    switch (r.state.read()) {
    case State_Idle:
        w_req_mem_ready = 1;
        if (i_req_mem_valid.read()) {
            v.req_addr = i_req_mem_addr.read();
            v.req_write = i_req_mem_write.read();
            v.req_burst = i_req_mem_burst.read();
            v.req_len = i_req_mem_len.read();
            v.state = State_Busy;
        }
        break;
    case State_Busy:
        if (r.req_len.read() == 0) {
            w_req_mem_ready = 1;
            if (i_req_mem_valid.read()) {
                v.req_addr = i_req_mem_addr.read();
                v.req_write = i_req_mem_write.read();
                v.req_burst = i_req_mem_burst.read();
                v.req_len = i_req_mem_len.read();
                v.state = State_Busy;
            } else {
                v.state = State_Idle;
            }
        } else {
            v.req_len = r.req_len.read() - 1;
            if (r.req_burst.read() == 0) {
                vb_req_addr = r.req_addr.read();
            } else if (r.req_burst.read() == 1) {
                vb_req_addr = r.req_addr.read() + 8;
            } else if (r.req_burst.read() == 2) {
                vb_req_addr(4, 0) = r.req_addr.read()(4, 0) + 8;
                vb_req_addr(31, 5) = r.req_addr.read()(31, 5);
            }
            v.req_addr = vb_req_addr;
        }
        break;
    }

    v.nrst = (r.nrst.read() << 1) | w_nrst;
    o_nrst = r.nrst.read()[1].to_bool();

    w_req_mem_ready = w_req_mem_ready;
    o_req_mem_ready = w_req_mem_ready;
    o_resp_mem_data_valid = w_resp_valid;
    o_resp_mem_data = wb_resp_data;
    o_resp_mem_load_fault = w_resp_load_fault;
    o_resp_mem_store_fault = r.store_fault.read();
    o_resp_mem_store_fault_addr = r.store_addr.read();
    o_interrupt = r.interrupt;

    o_dport_valid = w_dport_valid;
    o_dport_write = w_dport_write;
    o_dport_region = wb_dport_region;
    o_dport_addr = wb_dport_addr;
    o_dport_wdata = wb_dport_wdata;

    if (!r.nrst.read()[1]) {
    }
}

void RtlWrapper::registers() {
    bus_event_.notify(1, SC_NS);
    r = v;
}

void RtlWrapper::sys_bus_proc() {
    /** Simulation events queue */
    IFace *cb;

    step_queue_.initProc();
    step_queue_.pushPreQueued();
    uint64_t step_cnt = i_time.read();
    while ((cb = step_queue_.getNext(step_cnt)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

    if (i_halted.read() && !r.halted.read()) {
        IService *iserv = static_cast<IService *>(iparent_);
        RISCV_trigger_hap(iserv, HAP_Halt, "Descr");
    }

    w_nrst = async_nrst;
    w_interrupt = async_interrupt;

    w_resp_valid = 0;
    wb_resp_data = 0;
    w_resp_store_fault = 0;
    w_resp_load_fault = 0;
    if (r.state.read() == State_Busy) {
        trans.addr = r.req_addr.read();
        if (r.req_write.read() == 1) {
            trans.action = MemAction_Write;
            uint8_t strob = i_req_mem_strob.read();
            uint64_t offset = mask2offset(strob);
            trans.addr += offset;
            trans.xsize = mask2size(strob >> offset);
            trans.wstrb = (1 << trans.xsize) - 1;
            trans.wpayload.b64[0] = i_req_mem_data.read();
            trans.rpayload.b64[0] = 0;
        } else {
            trans.action = MemAction_Read;
            trans.xsize = 8;
            trans.wstrb = 0;
            trans.wpayload.b64[0] = 0;
            trans.rpayload.b64[0] = 0;
        }
        resp = ibus_->b_transport(&trans);

        w_resp_valid = 1;
        wb_resp_data = trans.rpayload.b64[0];
        if (resp == TRANS_ERROR) {
            if (r.req_write.read() == 1) {
                w_resp_store_fault = 1;
                wb_resp_store_fault_addr =
                    static_cast<uint32_t>(trans.addr);
            } else {
                w_resp_load_fault = 1;
            }
        }
    }

    // Debug port handling:
    w_dport_valid = 0;
    if (RISCV_event_is_set(&dport_.valid)) {
        RISCV_event_clear(&dport_.valid);
        w_dport_valid = 1;
        w_dport_write = dport_.trans->write;
        wb_dport_region = dport_.trans->region;
        wb_dport_addr = dport_.trans->addr >> 3;
        wb_dport_wdata = dport_.trans->wdata;
    }
    dport_.idx_missmatch = 0;
    if (i_dport_ready.read()) {
        dport_.trans->rdata = i_dport_rdata.read().to_uint64();
        dport_.trans_idx_down++;
        if (dport_.trans_idx_down != dport_.trans_idx_up) {
            dport_.idx_missmatch = 1;
            RISCV_error("error: sync. is lost: up=%d, down=%d",
                         dport_.trans_idx_up, dport_.trans_idx_down);
            dport_.trans_idx_down = dport_.trans_idx_up;
        }
        dport_.cb->nb_response_debug_port(dport_.trans);
    }

}

uint64_t RtlWrapper::mask2offset(uint8_t mask) {
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (mask & 0x1) {
            return static_cast<uint64_t>(i);
        }
        mask >>= 1;
    }
    return 0;
}

uint32_t RtlWrapper::mask2size(uint8_t mask) {
    uint32_t bytes = 0;
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (!(mask & 0x1)) {
            break;
        }
        bytes++;
        mask >>= 1;
    }
    return bytes;
}

void RtlWrapper::setClockHz(double hz) {
    sc_time dt = sc_get_time_resolution();
    clockCycles_ = static_cast<int>((1.0 / hz) / dt.to_seconds() + 0.5);
}
    
void RtlWrapper::registerStepCallback(IClockListener *cb, uint64_t t) {
    if (!async_nrst) {
        if (i_time.read() == t) {
            cb->stepCallback(t);
        }
    } else {
        step_queue_.put(t, cb);
    }
}

void RtlWrapper::raiseSignal(int idx) {
    switch (idx) {
    case SIGNAL_HardReset:
        async_nrst = 0;
        break;
    case INTERRUPT_MExternal:
        async_interrupt = true;
        break;
    default:;
    }
}

void RtlWrapper::lowerSignal(int idx) {
    switch (idx) {
    case SIGNAL_HardReset:
        async_nrst = 1;
        break;
    case INTERRUPT_MExternal:
        async_interrupt = false;
        break;
    default:;
    }
}

void RtlWrapper::nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb) {
    dport_.trans = trans;
    dport_.cb = cb;
    dport_.trans_idx_up++;
    RISCV_event_set(&dport_.valid);
}

}  // namespace debugger

