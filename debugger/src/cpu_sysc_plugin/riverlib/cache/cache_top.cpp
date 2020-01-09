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
    o_req_mem_path("o_req_mem_path"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    o_req_mem_len("o_req_mem_len"),
    o_req_mem_burst("o_req_mem_burst"),
    o_req_mem_last("o_req_mem_last"),
    i_resp_mem_valid("i_resp_mem_valid"),
    i_resp_mem_path("i_resp_mem_path"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    i_resp_mem_store_fault("i_resp_mem_store_fault"),
    i_resp_mem_store_fault_addr("i_resp_mem_store_fault_addr"),
    i_mpu_region_we("i_mpu_region_we"),
    i_mpu_region_idx("i_mpu_region_idx"),
    i_mpu_region_addr("i_mpu_region_addr"),
    i_mpu_region_mask("i_mpu_region_mask"),
    i_mpu_region_flags("i_mpu_region_flags"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    i_data_flush_address("i_data_flush_address"),
    i_data_flush_valid("i_data_flush_valid"),
    o_data_flush_end("o_data_flush_end"),
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
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_path;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << queue_rdata_o;
    sensitive << queue_full_o;
    sensitive << queue_nempty_o;

    i1 = new ICacheLru("i1", async_reset);
    i1->i_clk(i_clk);
    i1->i_nrst(i_nrst);
    i1->i_req_valid(i_req_ctrl_valid);
    i1->i_req_addr(i_req_ctrl_addr);
    i1->o_req_ready(o_req_ctrl_ready);
    i1->o_resp_valid(o_resp_ctrl_valid);
    i1->o_resp_addr(o_resp_ctrl_addr);
    i1->o_resp_data(o_resp_ctrl_data);
    i1->o_resp_load_fault(o_resp_ctrl_load_fault);
    i1->o_resp_executable(o_resp_ctrl_executable);
    i1->o_resp_writable(w_resp_ctrl_writable_unused);
    i1->o_resp_readable(w_resp_ctrl_readable_unused);
    i1->i_resp_ready(i_resp_ctrl_ready);
    i1->i_req_mem_ready(w_ctrl_req_ready);
    i1->o_req_mem_valid(i.req_mem_valid);
    i1->o_req_mem_write(i.req_mem_write);
    i1->o_req_mem_addr(i.req_mem_addr);
    i1->o_req_mem_strob(i.req_mem_strob);
    i1->o_req_mem_data(i.req_mem_wdata);
    i1->o_req_mem_len(i.req_mem_len);
    i1->o_req_mem_burst(i.req_mem_burst);
    i1->o_req_mem_last(i.req_mem_last);
    i1->i_mem_data_valid(w_ctrl_resp_mem_data_valid);
    i1->i_mem_data(wb_ctrl_resp_mem_data);
    i1->i_mem_load_fault(w_ctrl_resp_mem_load_fault);
    i1->o_mpu_addr(i.mpu_addr);
    i1->i_mpu_flags(wb_mpu_iflags);
    i1->i_flush_address(i_flush_address);
    i1->i_flush_valid(i_flush_valid);
    i1->o_state(o_istate);

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
    d0->i_mpu_flags(wb_mpu_dflags);
    d0->i_flush_address(i_data_flush_address);
    d0->i_flush_valid(i_data_flush_valid);
    d0->o_flush_end(o_data_flush_end);
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
    mpu0->o_iflags(wb_mpu_iflags);
    mpu0->o_dflags(wb_mpu_dflags);

    queue0 = new Queue<2, QUEUE_WIDTH>("queue0", async_reset);

    queue0->i_clk(i_clk);
    queue0->i_nrst(i_nrst);
    queue0->i_re(queue_re_i);
    queue0->i_we(queue_we_i);
    queue0->i_wdata(queue_wdata_i);
    queue0->o_rdata(queue_rdata_o);
    queue0->o_full(queue_full_o);
    queue0->o_nempty(queue_nempty_o);


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
        sc_trace(o_vcd, o_req_mem_path, o_req_mem_path.name());
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_valid, i_resp_mem_valid.name());
        sc_trace(o_vcd, i_resp_mem_path, i_resp_mem_path.name());
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
    }
    i1->generateVCD(i_vcd, o_vcd);
    d0->generateVCD(i_vcd, o_vcd);
    mpu0->generateVCD(i_vcd, o_vcd);
    queue0->generateVCD(i_vcd, o_vcd);
}

CacheTop::~CacheTop() {
    delete i1;
    delete d0;
    delete mpu0;
    delete queue0;
}


void CacheTop::comb() {
    sc_biguint<QUEUE_WIDTH> vb_ctrl_bus;
    sc_biguint<QUEUE_WIDTH> vb_data_bus;
    sc_biguint<QUEUE_WIDTH> vb_queue_bus;
    sc_biguint<QUEUE_WIDTH> to;
    sc_uint<1> ctrl_path_id;
    sc_uint<1> data_path_id;
    bool v_queue_we;
    bool v_queue_re;
    bool v_req_mem_last;

    v_queue_re = i_req_mem_ready;
    v_queue_we = i.req_mem_valid || d.req_mem_valid;

    ctrl_path_id = CTRL_PATH;
    vb_ctrl_bus = (ctrl_path_id,
                i.req_mem_write,
                i.req_mem_burst,
                i.req_mem_len,
                i.req_mem_addr);

    data_path_id = DATA_PATH;
    vb_data_bus = (data_path_id,
                d.req_mem_write,
                d.req_mem_burst,
                d.req_mem_len,
                d.req_mem_addr);

    if (d.req_mem_valid.read() == 1) {
        vb_queue_bus = vb_data_bus;
    } else {
        vb_queue_bus = vb_ctrl_bus;
    }

    queue_wdata_i = vb_queue_bus;
    queue_we_i = v_queue_we;
    queue_re_i = v_queue_re;

    w_data_req_ready = 1;
    w_ctrl_req_ready = !d.req_mem_valid;
    if (i_resp_mem_path.read() == CTRL_PATH) {
        v_req_mem_last = i.req_mem_last;
        w_ctrl_resp_mem_data_valid = i_resp_mem_valid.read();
        w_ctrl_resp_mem_load_fault = i_resp_mem_load_fault.read();
        w_data_resp_mem_data_valid = 0;
        w_data_resp_mem_load_fault = 0;
    } else {
        v_req_mem_last = d.req_mem_last;
        w_ctrl_resp_mem_data_valid = 0;
        w_ctrl_resp_mem_load_fault = 0;
        w_data_resp_mem_data_valid = i_resp_mem_valid.read();
        w_data_resp_mem_load_fault = i_resp_mem_load_fault.read();
    }

    wb_ctrl_resp_mem_data = i_resp_mem_data.read();
    wb_data_resp_mem_data = i_resp_mem_data.read();


    to = queue_rdata_o.read();
    o_req_mem_valid = queue_nempty_o;
    o_req_mem_path = to[BUS_ADDR_WIDTH + 11];
    o_req_mem_write = to[BUS_ADDR_WIDTH + 10];
    o_req_mem_burst = to(BUS_ADDR_WIDTH + 9, BUS_ADDR_WIDTH + 8).to_uint64();
    o_req_mem_len = to(BUS_ADDR_WIDTH + 7, BUS_ADDR_WIDTH).to_uint64();
    o_req_mem_addr = to(BUS_ADDR_WIDTH-1, 0).to_uint64();
    o_req_mem_strob = d.req_mem_strob;
    o_req_mem_data = d.req_mem_wdata;
    o_req_mem_last = v_req_mem_last;
    o_cstate = 0;
}

}  // namespace debugger

