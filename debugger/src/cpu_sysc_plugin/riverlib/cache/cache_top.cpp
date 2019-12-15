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

#include "cache_top.h"

namespace debugger {

CacheTop::CacheTop(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_ctrl_valid("i_req_ctrl_valid"),
    i_req_ctrl_addr("i_req_ctrl_addr"),
    o_req_ctrl_ready("o_req_ctrl_ready"),
    o_resp_ctrl_valid("o_resp_ctrl_valid"),
    o_resp_ctrl_addr("o_resp_ctrl_addr"),
    o_resp_ctrl_data("o_resp_ctrl_data"),
    o_resp_ctrl_load_fault("o_resp_ctrl_load_fault"),
    o_resp_ctrl_executable("o_resp_ctrl_executable"),
    i_resp_ctrl_ready("i_resp_ctrl_ready"),
    i_req_data_valid("i_req_data_valid"),
    i_req_data_write("i_req_data_write"),
    i_req_data_addr("i_req_data_addr"),
    i_req_data_wdata("i_req_data_wdata"),
    i_req_data_wstrb("i_req_data_wstrb"),
    o_req_data_ready("o_req_data_ready"),
    o_resp_data_valid("o_resp_data_valid"),
    o_resp_data_addr("o_resp_data_addr"),
    o_resp_data_data("o_resp_data_data"),
    o_resp_data_store_fault_addr("o_resp_data_store_fault_addr"),
    o_resp_data_load_fault("o_resp_data_load_fault"),
    o_resp_data_store_fault("o_resp_data_store_fault"),
    o_resp_data_er_mpu_load("o_resp_data_er_mpu_load"),
    o_resp_data_er_mpu_store("o_resp_data_er_mpu_store"),
    i_resp_data_ready("i_resp_data_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    o_req_mem_len("o_req_mem_len"),
    o_req_mem_burst("o_req_mem_burst"),
    i_resp_mem_data_valid("i_resp_mem_data_valid"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    i_resp_mem_store_fault("i_resp_mem_store_fault"),
    //i_resp_mem_store_fault_addr("i_resp_mem_store_fault_addr"),
    i_mpu_region_we("i_mpu_region_we"),
    i_mpu_region_idx("i_mpu_region_idx"),
    i_mpu_region_addr("i_mpu_region_addr"),
    i_mpu_region_mask("i_mpu_region_mask"),
    i_mpu_region_flags("i_mpu_region_flags"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    i_data_flush_address("i_data_flush_address"),
    i_data_flush_valid("i_data_flush_valid"),
    o_istate("o_istate"),
    o_dstate("o_dstate"),
    o_cstate("o_cstate") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_ready;
    sensitive << i.req_mem_valid;
    sensitive << i.req_mem_write;
    sensitive << i.req_mem_addr;
    sensitive << i.req_mem_strob;
    sensitive << i.req_mem_wdata;
    sensitive << i.req_mem_len;
    sensitive << i.req_mem_burst;
    sensitive << i.req_mem_last;
    sensitive << d.req_mem_valid;
    sensitive << d.req_mem_write;
    sensitive << d.req_mem_addr;
    sensitive << d.req_mem_strob;
    sensitive << d.req_mem_wdata;
    sensitive << d.req_mem_len;
    sensitive << d.req_mem_burst;
    sensitive << d.req_mem_last;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    w_mpu_ireadable_unsued = 0;
    w_mpu_iwritable_unused = 0;
    w_mpu_dexecutable_unused = 0;

    i1 = new ICacheLru("i1", async_reset, CFG_IINDEX_WIDTH);
    i1->i_clk(i_clk);
    i1->i_nrst(i_nrst);
    i1->i_req_ctrl_valid(i_req_ctrl_valid);
    i1->i_req_ctrl_addr(i_req_ctrl_addr);
    i1->o_req_ctrl_ready(o_req_ctrl_ready);
    i1->o_resp_ctrl_valid(o_resp_ctrl_valid);
    i1->o_resp_ctrl_addr(o_resp_ctrl_addr);
    i1->o_resp_ctrl_data(o_resp_ctrl_data);
    i1->o_resp_ctrl_load_fault(o_resp_ctrl_load_fault);
    i1->o_resp_ctrl_executable(o_resp_ctrl_executable);
    i1->o_resp_ctrl_writable(w_resp_ctrl_writable_unused);
    i1->o_resp_ctrl_readable(w_resp_ctrl_readable_unused);
    i1->i_resp_ctrl_ready(i_resp_ctrl_ready);
    i1->i_req_mem_ready(w_ctrl_req_ready);
    i1->o_req_mem_valid(i.req_mem_valid);
    i1->o_req_mem_write(i.req_mem_write);
    i1->o_req_mem_addr(i.req_mem_addr);
    i1->o_req_mem_strob(i.req_mem_strob);
    i1->o_req_mem_data(i.req_mem_wdata);
    i1->o_req_mem_len(i.req_mem_len);
    i1->o_req_mem_burst(i.req_mem_burst);
    i1->o_req_mem_last(i.req_mem_last);
    i1->i_resp_mem_data_valid(w_ctrl_resp_mem_data_valid);
    i1->i_resp_mem_data(wb_ctrl_resp_mem_data);
    i1->i_resp_mem_load_fault(w_ctrl_resp_mem_load_fault);
    i1->o_mpu_addr(i.mpu_addr);
    i1->i_mpu_cachable(w_mpu_icachable);
    i1->i_mpu_executable(w_mpu_iexecutable);
    i1->i_mpu_writable(w_mpu_iwritable_unused);
    i1->i_mpu_readable(w_mpu_ireadable_unsued);
    i1->i_flush_address(i_flush_address);
    i1->i_flush_valid(i_flush_valid);
    i1->o_istate(o_istate);

    d0 = new DCacheLru("d0", async_reset);
    d0->i_clk(i_clk);
    d0->i_nrst(i_nrst);
    d0->i_req_valid(i_req_data_valid);
    d0->i_req_write(i_req_data_write);
    d0->i_req_addr(i_req_data_addr);
    d0->i_req_wdata(i_req_data_wdata);
    d0->i_req_wstrb(i_req_data_wstrb);
    d0->o_req_ready(o_req_data_ready);
    d0->o_resp_valid(o_resp_data_valid);
    d0->o_resp_addr(o_resp_data_addr);
    d0->o_resp_data(o_resp_data_data);
    d0->o_resp_er_addr(o_resp_data_store_fault_addr);
    d0->o_resp_er_load_fault(o_resp_data_load_fault);
    d0->o_resp_er_store_fault(o_resp_data_store_fault);
    d0->o_resp_er_mpu_load(o_resp_data_er_mpu_load);
    d0->o_resp_er_mpu_store(o_resp_data_er_mpu_store);
    d0->i_resp_ready(i_resp_data_ready);
    d0->i_req_mem_ready(w_data_req_ready);
    d0->o_req_mem_valid(d.req_mem_valid);
    d0->o_req_mem_write(d.req_mem_write);
    d0->o_req_mem_addr(d.req_mem_addr);
    d0->o_req_mem_strob(d.req_mem_strob);
    d0->o_req_mem_data(d.req_mem_wdata);
    d0->o_req_mem_len(d.req_mem_len);
    d0->o_req_mem_burst(d.req_mem_burst);
    d0->o_req_mem_last(d.req_mem_last);
    d0->i_mem_data_valid(w_data_resp_mem_data_valid);
    d0->i_mem_data(wb_data_resp_mem_data);
    d0->i_mem_load_fault(w_data_resp_mem_load_fault);
    d0->i_mem_store_fault(i_resp_mem_store_fault);
    //d0->i_resp_mem_store_fault_addr(i_resp_mem_store_fault_addr);
    d0->o_mpu_addr(d.mpu_addr);
    d0->i_mpu_cachable(w_mpu_dcachable);
    d0->i_mpu_readable(w_mpu_dreadable);
    d0->i_mpu_writable(w_mpu_dwritable);
    d0->i_flush_address(i_data_flush_address);
    d0->i_flush_valid(i_data_flush_valid);
    d0->o_state(o_dstate);

    mpu0 = new MPU("mpu0", async_reset);

    mpu0->i_clk(i_clk);
    mpu0->i_nrst(i_nrst);
    mpu0->i_iaddr(i.mpu_addr);
    mpu0->i_daddr(d.mpu_addr);
    mpu0->i_region_we(i_mpu_region_we);
    mpu0->i_region_idx(i_mpu_region_idx);
    mpu0->i_region_addr(i_mpu_region_addr);
    mpu0->i_region_mask(i_mpu_region_mask);
    mpu0->i_region_flags(i_mpu_region_flags);
    mpu0->o_icachable(w_mpu_icachable);
    mpu0->o_iexecutable(w_mpu_iexecutable);
    mpu0->o_dcachable(w_mpu_dcachable);
    mpu0->o_dreadable(w_mpu_dreadable);
    mpu0->o_dwritable(w_mpu_dwritable);

#ifdef DBG_ICACHE_TB
    i0_tb = new ICache_tb("ictb0");
#endif
};

void CacheTop::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_data_valid, i_req_data_valid.name());
        sc_trace(o_vcd, i_req_data_write, i_req_data_write.name());
        sc_trace(o_vcd, i_req_data_addr, i_req_data_addr.name());
        sc_trace(o_vcd, i_req_data_wdata, i_req_data_wdata.name());
        sc_trace(o_vcd, i_req_data_wstrb, i_req_data_wstrb.name());
        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_data_valid, i_resp_mem_data_valid.name());
        sc_trace(o_vcd, i_resp_mem_data, i_resp_mem_data.name());
        sc_trace(o_vcd, o_istate, o_istate.name());
        sc_trace(o_vcd, o_dstate, o_dstate.name());
        sc_trace(o_vcd, o_cstate, o_cstate.name());

        std::string pn(name());
        sc_trace(o_vcd, w_data_req_ready, pn + ".w_data_req_ready");
        sc_trace(o_vcd, w_data_resp_mem_data_valid,
                        pn + ".w_data_resp_mem_data_valid");
        sc_trace(o_vcd, wb_data_resp_mem_data, pn + ".wb_data_resp_mem_data");
        sc_trace(o_vcd, w_ctrl_resp_mem_data_valid,
                        pn + ".w_ctrl_resp_mem_data_valid");
        sc_trace(o_vcd, wb_ctrl_resp_mem_data, pn + ".wb_ctrl_resp_mem_data");
        sc_trace(o_vcd, w_ctrl_req_ready, pn + ".w_ctrl_req_ready");
        sc_trace(o_vcd, r.state, pn + ".r_state");
    }
    i1->generateVCD(i_vcd, o_vcd);
    d0->generateVCD(i_vcd, o_vcd);
    mpu0->generateVCD(i_vcd, o_vcd);
}

CacheTop::~CacheTop() {
    delete i1;
    delete d0;
    delete mpu0;
}


void CacheTop::comb() {
    bool w_req_mem_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<8> wb_mem_len;
    sc_uint<2> wb_mem_burst;
    bool w_mem_write;
    bool v_data_req_ready;
    bool v_data_resp_mem_data_valid;
    bool v_ctrl_req_ready;
    bool v_ctrl_resp_mem_data_valid;

    v = r;

    // default is data path
    w_req_mem_valid = 0;
    wb_mem_addr = d.req_mem_addr;
    w_mem_write = d.req_mem_write;
    wb_mem_len = d.req_mem_len;
    wb_mem_burst = d.req_mem_burst;
    v_data_req_ready = 0;
    v_data_resp_mem_data_valid = 0;
    v_ctrl_req_ready = 0;
    v_ctrl_resp_mem_data_valid = 0;

    switch (r.state.read()) {
    case State_Idle:
        w_req_mem_valid = i.req_mem_valid | d.req_mem_valid;
        if (d.req_mem_valid.read()) {
            v_data_req_ready = i_req_mem_ready.read();
            if (i_req_mem_ready.read() == 1) {
                v.state = State_DMem;
            }
        } else if (i.req_mem_valid.read()) {
            v_ctrl_req_ready = i_req_mem_ready.read();
            wb_mem_addr = i.req_mem_addr;
            wb_mem_len = i.req_mem_len;
            wb_mem_burst = i.req_mem_burst;
            w_mem_write = i.req_mem_write;
            if (i_req_mem_ready.read() == 1) {
                v.state = State_IMem;
            }
        }
        break;

    case State_DMem:
        if (d.req_mem_last.read() == 1) {
            w_req_mem_valid = 1;
            if (d.req_mem_valid.read() == 1) {
                wb_mem_addr = d.req_mem_addr;
            } else if (i.req_mem_valid.read() == 1) {
                wb_mem_addr = i.req_mem_addr;
            }
        }
        if (i_resp_mem_data_valid.read() && d.req_mem_last.read()) {
            if (d.req_mem_valid.read()) {
                v_data_req_ready = i_req_mem_ready.read();
                if (i_req_mem_ready.read() == 1) {
                    v.state = State_DMem;
                } else {
                    v.state = State_Idle;
                }
            } else if (i.req_mem_valid.read() == 1) {
                v_ctrl_req_ready = i_req_mem_ready.read();
                wb_mem_addr = i.req_mem_addr;
                wb_mem_len = i.req_mem_len;
                wb_mem_burst = i.req_mem_burst;
                w_mem_write = i.req_mem_write;
                if (i_req_mem_ready.read() == 1) {
                    v.state = State_IMem;
                } else {
                    v.state = State_Idle;
                }
            } else {
                v.state = State_Idle;
            }
        }
        v_data_resp_mem_data_valid = i_resp_mem_data_valid;
        break;

    case State_IMem:
        if (i.req_mem_last.read() == 1) {
            w_req_mem_valid = 1;
            if (d.req_mem_valid.read() == 1) {
                wb_mem_addr = d.req_mem_addr;
            } else if (i.req_mem_valid.read() == 1) {
                wb_mem_addr = i.req_mem_addr;
            }
        }
        if (i_resp_mem_data_valid.read() && i.req_mem_last.read()) {
            if (d.req_mem_valid.read()) {
                v_data_req_ready = i_req_mem_ready.read();
                if (i_req_mem_ready.read() == 1) {
                    v.state = State_DMem;
                } else {
                    v.state = State_Idle;
                }
            } else if (i.req_mem_valid.read() == 1) {
                v_ctrl_req_ready = i_req_mem_ready.read();
                wb_mem_addr = i.req_mem_addr;
                wb_mem_len = i.req_mem_len;
                wb_mem_burst = i.req_mem_burst;
                w_mem_write = i.req_mem_write;
                if (i_req_mem_ready.read() == 1) {
                    v.state = State_IMem;
                } else {
                    v.state = State_Idle;
                }
            } else {
                v.state = State_Idle;
            }
        }
        v_ctrl_resp_mem_data_valid = i_resp_mem_data_valid;
        break;
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        v.state = State_Idle;
    }

    w_data_req_ready = v_data_req_ready;
    w_data_resp_mem_data_valid = v_data_resp_mem_data_valid;
    wb_data_resp_mem_data = i_resp_mem_data.read();
    w_data_resp_mem_load_fault = i_resp_mem_load_fault.read();

    w_ctrl_req_ready = v_ctrl_req_ready;
    w_ctrl_resp_mem_data_valid = v_ctrl_resp_mem_data_valid;
    wb_ctrl_resp_mem_data = i_resp_mem_data.read();
    w_ctrl_resp_mem_load_fault = i_resp_mem_load_fault.read();

    o_req_mem_valid = w_req_mem_valid;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_len = wb_mem_len;
    o_req_mem_burst = wb_mem_burst;
    o_req_mem_write = w_mem_write;
    o_req_mem_strob = d.req_mem_strob;
    o_req_mem_data = d.req_mem_wdata;
    o_cstate = r.state;
}

void CacheTop::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.state = State_Idle;
    } else {
        r = v;
    }
}

}  // namespace debugger

