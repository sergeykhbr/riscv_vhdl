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
#include "coreservices/iserial.h"

namespace debugger {

extern bool dbg_e_valid;

RtlWrapper::RtlWrapper(IFace *parent, sc_module_name name) : sc_module(name),
    o_clk("clk", 10, SC_NS),
    o_sys_nrst("o_sys_nrst"),
    o_dmi_nrst("o_dmi_nrst"),
    o_msti("o_msti"),
    i_msto("i_msto"),
    o_msip("o_msip"),
    o_mtip("o_mtip"),
    o_meip("o_meip"),
    o_seip("o_seip"),
    i_ndmreset("i_ndmreset") {
    iparent_ = parent;
    clockCycles_ = 1000000; // 1 MHz when default resolution = 1 ps

    v.nrst = 1;//0;
    v.req_addr = 0;
    v.req_len = 0;
    v.req_burst = 0;
    v.b_valid = 0;
    v.b_resp = 0;
    async_interrupt = 0;
    request_reset = false;
    w_interrupt = 0;
    v.state = State_Idle;
    r.state = State_Idle;
    v.clk_cnt = 0;
    r.clk_cnt = 0;
    trans.source_idx = 0;//CFG_NASTI_MASTER_CACHED;
    w_msip = 0;
    w_mtip = 0;
    w_meip = 0;
    w_seip = 0;
    iirqloc_ = 0;
    iirqext_ = 0;

    SC_METHOD(comb);
    sensitive << w_interrupt;
    sensitive << i_msto;
    sensitive << i_ndmreset;
    sensitive << r.clk_cnt;
    sensitive << r.req_addr;
    sensitive << r.req_len;
    sensitive << r.req_burst;
    sensitive << r.b_valid;
    sensitive << r.b_resp;
    sensitive << r.nrst;
    sensitive << r.state;
    sensitive << r.r_error;
    sensitive << r.w_error;
    sensitive << w_resp_valid;
    sensitive << wb_resp_data;
    sensitive << w_r_error;
    sensitive << w_w_error;

    SC_METHOD(registers);
    sensitive << o_clk.posedge_event();

    SC_METHOD(sys_bus_proc);
    sensitive << bus_req_event_;
}

RtlWrapper::~RtlWrapper() {
}

void RtlWrapper::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, i_msto, i_msto.name());
        sc_trace(o_vcd, o_msti, o_msti.name());

        std::string pn(name());
        sc_trace(o_vcd, r.nrst, pn + ".r_nrst");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.req_burst, pn + ".r_req_burst");
        sc_trace(o_vcd, r.b_resp, pn + ".r_b_resp");
        sc_trace(o_vcd, r.b_valid, pn + ".r_b_valid");
        sc_trace(o_vcd, wb_wdata, pn + ".wb_wdata");
        sc_trace(o_vcd, wb_wstrb, pn + ".wb_wstrb");
    }
}

void RtlWrapper::clk_gen() {
    // todo: instead sc_clock
}

void RtlWrapper::comb() {
    bool w_req_mem_ready;
    sc_uint<CFG_SYSBUS_ADDR_BITS> vb_req_addr;
    sc_uint<4> vb_r_resp;
    bool v_r_valid;
    bool v_r_last;
    bool v_w_ready;
    sc_uint<CFG_SYSBUS_DATA_BITS> vb_wdata;
    sc_uint<CFG_SYSBUS_DATA_BYTES> vb_wstrb;
    axi4_master_in_type vmsti;
    sc_uint<CFG_CPU_MAX> vb_msip;
    sc_uint<CFG_CPU_MAX> vb_mtip;
    sc_uint<CFG_CPU_MAX> vb_meip;
    sc_uint<CFG_CPU_MAX> vb_seip;

    w_req_mem_ready = 0;
    vb_r_resp = 0; // OKAY
    v_r_valid = 0;
    v_r_last = 0;
    v_w_ready = 0;
    v.b_valid = 0;
    v.b_resp = 0;

    vb_wdata = 0;
    vb_wstrb = 0;
    vb_msip = 0;
    vb_mtip = 0;
    vb_meip = 0;
    vb_seip = 0;

    v.clk_cnt = r.clk_cnt.read() + 1;

    v.nrst = (r.nrst.read() << 1) | 1;
    switch (r.state.read()) {
    case State_Idle:
        v.r_error = 0;
        v.w_error = 0;
        if (request_reset) {
            v.state = State_Reset;
        } else {
            w_req_mem_ready = 1;
        }
        break;
    case State_Read:
        v_r_valid = 1;
        if (r.req_len.read() == 0) {
            w_req_mem_ready = 1;
            v_r_last = 1;
        } else {
            v.r_error = r.r_error.read() || w_r_error;
            v.req_len = r.req_len.read() - 1;
            if (r.req_burst.read() == 1) {
                vb_req_addr = r.req_addr.read() + 8;
            }
            v.req_addr = vb_req_addr;
        }
        break;
    case State_Write:
        vb_wdata = i_msto.read().w_data;
        vb_wstrb = i_msto.read().w_strb;
        v_w_ready = 1;
        if (r.req_len.read() == 0) {
            v.b_valid = 1;
            v.b_resp = r.w_error.read() || w_w_error ? 0x2 : 0x0;
            w_req_mem_ready = 1;
        } else {
            v.w_error = r.w_error.read() || w_w_error;
            v.req_len = r.req_len.read() - 1;
            if (r.req_burst.read() == 1) {
                vb_req_addr = r.req_addr.read() + 8;
            }
            v.req_addr = vb_req_addr;
        }
        break;
    case State_Reset:
        request_reset = false;
        v.nrst = (r.nrst.read() << 1);
        v.state = State_Idle;
        break;
    default:;
    }

    if (w_req_mem_ready == 1) {
        v.r_error = 0;
        v.w_error = 0;
        if (i_msto.read().ar_valid) {
            v.state = State_Read;
            v.req_addr = i_msto.read().ar_bits.addr;
            v.req_burst = i_msto.read().ar_bits.burst;
            v.req_len = i_msto.read().ar_bits.len;
        } else if (i_msto.read().aw_valid) {
            v.state = State_Write;
            v.req_addr = i_msto.read().aw_bits.addr;
            v.req_burst = i_msto.read().aw_bits.burst;
            v.req_len = i_msto.read().aw_bits.len;
        } else {
            v.state = State_Idle;
        }
    }

    if (r.r_error.read() || w_r_error.read()) {
        vb_r_resp = 0x2;    // SLVERR
    }

    wb_wdata = vb_wdata;
    wb_wstrb = vb_wstrb;

    o_dmi_nrst = r.nrst.read()[1].to_bool();
    o_sys_nrst = r.nrst.read()[1].to_bool() 
            && !i_ndmreset.read();

    vmsti.aw_ready = w_req_mem_ready;
    vmsti.w_ready = v_w_ready;
    vmsti.b_valid = r.b_valid;
    vmsti.b_resp = r.b_resp;
    vmsti.b_id = 0;
    vmsti.b_user = 0;

    vmsti.ar_ready = w_req_mem_ready;
    vmsti.r_valid = v_r_valid;
    vmsti.r_resp = vb_r_resp;                    // 0=OKAY;1=EXOKAY;2=SLVERR;3=DECER
    vmsti.r_data = wb_resp_data;
    vmsti.r_last = v_r_last;
    vmsti.r_id = 0;
    vmsti.r_user = 0;
    o_msti = vmsti;     // to trigger event;

    vb_msip[0] = w_msip;
    vb_mtip[0] = w_mtip;
    vb_meip[0] = w_meip;
    vb_seip[0] = w_seip;

    o_msip = vb_msip;
    o_mtip = vb_mtip;
    o_meip = vb_meip;
    o_seip = vb_seip;
    o_mtimer = r.clk_cnt;

    if (!r.nrst.read()[1]) {
    }
}

void RtlWrapper::registers() {
    bus_req_event_.notify(1, SC_NS);

    w_msip = 0;
    w_mtip = 0;
    w_meip = 0;
    w_seip = 0;

    if (iirqloc_) {
        int hartid = 0;
        w_msip = iirqloc_->getPendingRequest(2*hartid);
        w_mtip = iirqloc_->getPendingRequest(2*hartid + 1);
    }
    if (iirqext_) {
        int ctx = 0;
        int irqidx = iirqext_->getPendingRequest(ctx);
        if (irqidx != IRQ_REQUEST_NONE) {
            w_meip = 1;
        }
        irqidx = iirqext_->getPendingRequest(ctx + 1);
        if (irqidx != IRQ_REQUEST_NONE) {
            w_seip = 1;
        }
    }

    r = v;
}

void RtlWrapper::sys_bus_proc() {
    /** Simulation events queue */
    IFace *cb;
    uint8_t strob;
    uint64_t offset;

    step_queue_.initProc();
    step_queue_.pushPreQueued();
    uint64_t step_cnt = r.clk_cnt.read();
    while ((cb = step_queue_.getNext(step_cnt)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

    w_interrupt = async_interrupt;

    w_resp_valid = 0;
    wb_resp_data = 0;
    w_r_error = 0;
    w_w_error = 0;

    uint64_t toff;
    switch (r.state.read()) {
    case State_Read:
        trans.action = MemAction_Read;
        trans.addr = r.req_addr.read();
        trans.xsize = 8;
        trans.wstrb = 0;
        trans.wpayload.b64[0] = 0;
        resp = ibus_->b_transport(&trans);

        w_resp_valid = 1;
        toff = r.req_addr.read()(CFG_LOG2_SYSBUS_DATA_BYTES - 1, 0);
        wb_resp_data = (trans.rpayload.b64[0]) << (8*toff);
        if (resp == TRANS_ERROR) {
            w_r_error = 1;
        }
        break;
    case State_Write:
        trans.action = MemAction_Write;
        strob = static_cast<uint8_t>(wb_wstrb.read());
        offset = mask2offset(strob);
        trans.addr = r.req_addr.read() & ~((1 << CFG_LOG2_SYSBUS_DATA_BYTES) - 1);
        if (offset) {
            trans.addr += offset;
        }
        trans.xsize = mask2size(strob >> offset);
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.wpayload.b64[0] = wb_wdata.read() >> (8*offset);
        resp = ibus_->b_transport(&trans);

        w_resp_valid = 1;
        if (resp == TRANS_ERROR) {
            w_w_error = 1;
        }
        break;
    default:;
    }
}

uint64_t RtlWrapper::mask2offset(uint8_t mask) {
    for (int i = 0; i < CFG_SYSBUS_DATA_BYTES; i++) {
        if (mask & 0x1) {
            return static_cast<uint64_t>(i);
        }
        mask >>= 1;
    }
    return 0;
}

uint32_t RtlWrapper::mask2size(uint8_t mask) {
    uint32_t bytes = 0;
    for (int i = 0; i < CFG_SYSBUS_DATA_BYTES; i++) {
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
    if (request_reset) {
        if (r.clk_cnt.read() == t) {
            cb->stepCallback(t);
        }
    } else {
        step_queue_.put(t, cb);
    }
}

bool RtlWrapper::isHalt() {
    return 0;
}

}  // namespace debugger

