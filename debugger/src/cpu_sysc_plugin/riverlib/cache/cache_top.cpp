// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 

#include "cache_top.h"
#include "api_core.h"

namespace debugger {

CacheTop::CacheTop(sc_module_name name,
                   bool async_reset,
                   bool coherence_ena)
    : sc_module(name),
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
    i_req_data_type("i_req_data_type"),
    i_req_data_addr("i_req_data_addr"),
    i_req_data_wdata("i_req_data_wdata"),
    i_req_data_wstrb("i_req_data_wstrb"),
    i_req_data_size("i_req_data_size"),
    o_req_data_ready("o_req_data_ready"),
    o_resp_data_valid("o_resp_data_valid"),
    o_resp_data_addr("o_resp_data_addr"),
    o_resp_data_data("o_resp_data_data"),
    o_resp_data_fault_addr("o_resp_data_fault_addr"),
    o_resp_data_load_fault("o_resp_data_load_fault"),
    o_resp_data_store_fault("o_resp_data_store_fault"),
    o_resp_data_er_mpu_load("o_resp_data_er_mpu_load"),
    o_resp_data_er_mpu_store("o_resp_data_er_mpu_store"),
    i_resp_data_ready("i_resp_data_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_path("o_req_mem_path"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_type("o_req_mem_type"),
    o_req_mem_size("o_req_mem_size"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    i_resp_mem_valid("i_resp_mem_valid"),
    i_resp_mem_path("i_resp_mem_path"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    i_resp_mem_store_fault("i_resp_mem_store_fault"),
    i_mpu_region_we("i_mpu_region_we"),
    i_mpu_region_idx("i_mpu_region_idx"),
    i_mpu_region_addr("i_mpu_region_addr"),
    i_mpu_region_mask("i_mpu_region_mask"),
    i_mpu_region_flags("i_mpu_region_flags"),
    i_req_snoop_valid("i_req_snoop_valid"),
    i_req_snoop_type("i_req_snoop_type"),
    o_req_snoop_ready("o_req_snoop_ready"),
    i_req_snoop_addr("i_req_snoop_addr"),
    i_resp_snoop_ready("i_resp_snoop_ready"),
    o_resp_snoop_valid("o_resp_snoop_valid"),
    o_resp_snoop_data("o_resp_snoop_data"),
    o_resp_snoop_flags("o_resp_snoop_flags"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    i_data_flush_address("i_data_flush_address"),
    i_data_flush_valid("i_data_flush_valid"),
    o_data_flush_end("o_data_flush_end") {

    async_reset_ = async_reset;
    coherence_ena_ = coherence_ena;
    i1 = 0;
    d0 = 0;
    mpu0 = 0;
    queue0 = 0;

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
    i1->o_req_mem_type(i.req_mem_type);
    i1->o_req_mem_size(i.req_mem_size);
    i1->o_req_mem_addr(i.req_mem_addr);
    i1->o_req_mem_strob(i.req_mem_strob);
    i1->o_req_mem_data(i.req_mem_wdata);
    i1->i_mem_data_valid(w_ctrl_resp_mem_data_valid);
    i1->i_mem_data(wb_ctrl_resp_mem_data);
    i1->i_mem_load_fault(w_ctrl_resp_mem_load_fault);
    i1->o_mpu_addr(i.mpu_addr);
    i1->i_mpu_flags(wb_mpu_iflags);
    i1->i_flush_address(i_flush_address);
    i1->i_flush_valid(i_flush_valid);


    d0 = new DCacheLru("d0", async_reset, coherence_ena);
    d0->i_clk(i_clk);
    d0->i_nrst(i_nrst);
    d0->i_req_valid(i_req_data_valid);
    d0->i_req_type(i_req_data_type);
    d0->i_req_addr(i_req_data_addr);
    d0->i_req_wdata(i_req_data_wdata);
    d0->i_req_wstrb(i_req_data_wstrb);
    d0->i_req_size(i_req_data_size);
    d0->o_req_ready(o_req_data_ready);
    d0->o_resp_valid(o_resp_data_valid);
    d0->o_resp_addr(o_resp_data_addr);
    d0->o_resp_data(o_resp_data_data);
    d0->o_resp_er_addr(o_resp_data_fault_addr);
    d0->o_resp_er_load_fault(o_resp_data_load_fault);
    d0->o_resp_er_store_fault(o_resp_data_store_fault);
    d0->o_resp_er_mpu_load(o_resp_data_er_mpu_load);
    d0->o_resp_er_mpu_store(o_resp_data_er_mpu_store);
    d0->i_resp_ready(i_resp_data_ready);
    d0->i_req_mem_ready(w_data_req_ready);
    d0->o_req_mem_valid(d.req_mem_valid);
    d0->o_req_mem_type(d.req_mem_type);
    d0->o_req_mem_size(d.req_mem_size);
    d0->o_req_mem_addr(d.req_mem_addr);
    d0->o_req_mem_strob(d.req_mem_strob);
    d0->o_req_mem_data(d.req_mem_wdata);
    d0->i_mem_data_valid(w_data_resp_mem_data_valid);
    d0->i_mem_data(wb_data_resp_mem_data);
    d0->i_mem_load_fault(w_data_resp_mem_load_fault);
    d0->i_mem_store_fault(i_resp_mem_store_fault);
    d0->o_mpu_addr(d.mpu_addr);
    d0->i_mpu_flags(wb_mpu_dflags);
    d0->i_req_snoop_valid(i_req_snoop_valid);
    d0->i_req_snoop_type(i_req_snoop_type);
    d0->o_req_snoop_ready(o_req_snoop_ready);
    d0->i_req_snoop_addr(i_req_snoop_addr);
    d0->i_resp_snoop_ready(i_resp_snoop_ready);
    d0->o_resp_snoop_valid(o_resp_snoop_valid);
    d0->o_resp_snoop_data(o_resp_snoop_data);
    d0->o_resp_snoop_flags(o_resp_snoop_flags);
    d0->i_flush_address(i_data_flush_address);
    d0->i_flush_valid(i_data_flush_valid);
    d0->o_flush_end(o_data_flush_end);


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


    queue0 = new Queue<2,
                       QUEUE_WIDTH>("queue0", async_reset);
    queue0->i_clk(i_clk);
    queue0->i_nrst(i_nrst);
    queue0->i_re(queue_re_i);
    queue0->i_we(queue_we_i);
    queue0->i_wdata(queue_wdata_i);
    queue0->o_rdata(queue_rdata_o);
    queue0->o_full(queue_full_o);
    queue0->o_nempty(queue_nempty_o);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_valid;
    sensitive << i_req_ctrl_addr;
    sensitive << i_resp_ctrl_ready;
    sensitive << i_req_data_valid;
    sensitive << i_req_data_type;
    sensitive << i_req_data_addr;
    sensitive << i_req_data_wdata;
    sensitive << i_req_data_wstrb;
    sensitive << i_req_data_size;
    sensitive << i_resp_data_ready;
    sensitive << i_req_mem_ready;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_path;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << i_resp_mem_store_fault;
    sensitive << i_mpu_region_we;
    sensitive << i_mpu_region_idx;
    sensitive << i_mpu_region_addr;
    sensitive << i_mpu_region_mask;
    sensitive << i_mpu_region_flags;
    sensitive << i_req_snoop_valid;
    sensitive << i_req_snoop_type;
    sensitive << i_req_snoop_addr;
    sensitive << i_resp_snoop_ready;
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << i_data_flush_address;
    sensitive << i_data_flush_valid;
    sensitive << i.req_mem_valid;
    sensitive << i.req_mem_type;
    sensitive << i.req_mem_size;
    sensitive << i.req_mem_addr;
    sensitive << i.req_mem_strob;
    sensitive << i.req_mem_wdata;
    sensitive << i.mpu_addr;
    sensitive << d.req_mem_valid;
    sensitive << d.req_mem_type;
    sensitive << d.req_mem_size;
    sensitive << d.req_mem_addr;
    sensitive << d.req_mem_strob;
    sensitive << d.req_mem_wdata;
    sensitive << d.mpu_addr;
    sensitive << w_ctrl_resp_mem_data_valid;
    sensitive << wb_ctrl_resp_mem_data;
    sensitive << w_ctrl_resp_mem_load_fault;
    sensitive << w_resp_ctrl_writable_unused;
    sensitive << w_resp_ctrl_readable_unused;
    sensitive << w_ctrl_req_ready;
    sensitive << w_data_resp_mem_data_valid;
    sensitive << wb_data_resp_mem_data;
    sensitive << w_data_resp_mem_load_fault;
    sensitive << w_data_req_ready;
    sensitive << wb_mpu_iflags;
    sensitive << wb_mpu_dflags;
    sensitive << queue_re_i;
    sensitive << queue_we_i;
    sensitive << queue_wdata_i;
    sensitive << queue_rdata_o;
    sensitive << queue_full_o;
    sensitive << queue_nempty_o;
}

CacheTop::~CacheTop() {
    if (i1) {
        delete i1;
    }
    if (d0) {
        delete d0;
    }
    if (mpu0) {
        delete mpu0;
    }
    if (queue0) {
        delete queue0;
    }
}

void CacheTop::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_ctrl_valid, i_req_ctrl_valid.name());
        sc_trace(o_vcd, i_req_ctrl_addr, i_req_ctrl_addr.name());
        sc_trace(o_vcd, o_req_ctrl_ready, o_req_ctrl_ready.name());
        sc_trace(o_vcd, o_resp_ctrl_valid, o_resp_ctrl_valid.name());
        sc_trace(o_vcd, o_resp_ctrl_addr, o_resp_ctrl_addr.name());
        sc_trace(o_vcd, o_resp_ctrl_data, o_resp_ctrl_data.name());
        sc_trace(o_vcd, o_resp_ctrl_load_fault, o_resp_ctrl_load_fault.name());
        sc_trace(o_vcd, o_resp_ctrl_executable, o_resp_ctrl_executable.name());
        sc_trace(o_vcd, i_resp_ctrl_ready, i_resp_ctrl_ready.name());
        sc_trace(o_vcd, i_req_data_valid, i_req_data_valid.name());
        sc_trace(o_vcd, i_req_data_type, i_req_data_type.name());
        sc_trace(o_vcd, i_req_data_addr, i_req_data_addr.name());
        sc_trace(o_vcd, i_req_data_wdata, i_req_data_wdata.name());
        sc_trace(o_vcd, i_req_data_wstrb, i_req_data_wstrb.name());
        sc_trace(o_vcd, i_req_data_size, i_req_data_size.name());
        sc_trace(o_vcd, o_req_data_ready, o_req_data_ready.name());
        sc_trace(o_vcd, o_resp_data_valid, o_resp_data_valid.name());
        sc_trace(o_vcd, o_resp_data_addr, o_resp_data_addr.name());
        sc_trace(o_vcd, o_resp_data_data, o_resp_data_data.name());
        sc_trace(o_vcd, o_resp_data_fault_addr, o_resp_data_fault_addr.name());
        sc_trace(o_vcd, o_resp_data_load_fault, o_resp_data_load_fault.name());
        sc_trace(o_vcd, o_resp_data_store_fault, o_resp_data_store_fault.name());
        sc_trace(o_vcd, o_resp_data_er_mpu_load, o_resp_data_er_mpu_load.name());
        sc_trace(o_vcd, o_resp_data_er_mpu_store, o_resp_data_er_mpu_store.name());
        sc_trace(o_vcd, i_resp_data_ready, i_resp_data_ready.name());
        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_path, o_req_mem_path.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_type, o_req_mem_type.name());
        sc_trace(o_vcd, o_req_mem_size, o_req_mem_size.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_valid, i_resp_mem_valid.name());
        sc_trace(o_vcd, i_resp_mem_path, i_resp_mem_path.name());
        sc_trace(o_vcd, i_resp_mem_data, i_resp_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_load_fault, i_resp_mem_load_fault.name());
        sc_trace(o_vcd, i_resp_mem_store_fault, i_resp_mem_store_fault.name());
        sc_trace(o_vcd, i_mpu_region_we, i_mpu_region_we.name());
        sc_trace(o_vcd, i_mpu_region_idx, i_mpu_region_idx.name());
        sc_trace(o_vcd, i_mpu_region_addr, i_mpu_region_addr.name());
        sc_trace(o_vcd, i_mpu_region_mask, i_mpu_region_mask.name());
        sc_trace(o_vcd, i_mpu_region_flags, i_mpu_region_flags.name());
        sc_trace(o_vcd, i_req_snoop_valid, i_req_snoop_valid.name());
        sc_trace(o_vcd, i_req_snoop_type, i_req_snoop_type.name());
        sc_trace(o_vcd, o_req_snoop_ready, o_req_snoop_ready.name());
        sc_trace(o_vcd, i_req_snoop_addr, i_req_snoop_addr.name());
        sc_trace(o_vcd, i_resp_snoop_ready, i_resp_snoop_ready.name());
        sc_trace(o_vcd, o_resp_snoop_valid, o_resp_snoop_valid.name());
        sc_trace(o_vcd, o_resp_snoop_data, o_resp_snoop_data.name());
        sc_trace(o_vcd, o_resp_snoop_flags, o_resp_snoop_flags.name());
        sc_trace(o_vcd, i_flush_address, i_flush_address.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, i_data_flush_address, i_data_flush_address.name());
        sc_trace(o_vcd, i_data_flush_valid, i_data_flush_valid.name());
        sc_trace(o_vcd, o_data_flush_end, o_data_flush_end.name());
    }

    if (i1) {
        i1->generateVCD(i_vcd, o_vcd);
    }
    if (d0) {
        d0->generateVCD(i_vcd, o_vcd);
    }
    if (mpu0) {
        mpu0->generateVCD(i_vcd, o_vcd);
    }
}

void CacheTop::comb() {
    sc_biguint<QUEUE_WIDTH> vb_ctrl_bus;
    sc_biguint<QUEUE_WIDTH> vb_data_bus;
    sc_biguint<QUEUE_WIDTH> vb_queue_bus;
    sc_uint<1> ctrl_path_id;
    sc_uint<1> data_path_id;
    bool v_queue_we;
    bool v_queue_re;
    bool v_req_mem_path_o;
    sc_uint<REQ_MEM_TYPE_BITS> vb_req_mem_type_o;
    sc_uint<3> vb_req_mem_size_o;
    sc_uint<CFG_CPU_ADDR_BITS> vb_req_mem_addr_o;

    vb_ctrl_bus = 0;
    vb_data_bus = 0;
    vb_queue_bus = 0;
    ctrl_path_id = 0;
    data_path_id = 0;
    v_queue_we = 0;
    v_queue_re = 0;
    v_req_mem_path_o = 0;
    vb_req_mem_type_o = 0;
    vb_req_mem_size_o = 0;
    vb_req_mem_addr_o = 0;

    v_queue_re = i_req_mem_ready;
    v_queue_we = (i.req_mem_valid || d.req_mem_valid);

    ctrl_path_id = CTRL_PATH;
    vb_ctrl_bus = (ctrl_path_id,
            i.req_mem_type,
            i.req_mem_size,
            i.req_mem_addr);

    data_path_id = DATA_PATH;
    vb_data_bus = (data_path_id,
            d.req_mem_type,
            d.req_mem_size,
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
    w_ctrl_req_ready = (!d.req_mem_valid);
    if (i_resp_mem_path.read() == CTRL_PATH) {
        w_ctrl_resp_mem_data_valid = i_resp_mem_valid;
        w_ctrl_resp_mem_load_fault = i_resp_mem_load_fault;
        w_data_resp_mem_data_valid = 0;
        w_data_resp_mem_load_fault = 0;
    } else {
        w_ctrl_resp_mem_data_valid = 0;
        w_ctrl_resp_mem_load_fault = 0;
        w_data_resp_mem_data_valid = i_resp_mem_valid;
        w_data_resp_mem_load_fault = i_resp_mem_load_fault;
    }

    wb_ctrl_resp_mem_data = i_resp_mem_data;
    wb_data_resp_mem_data = i_resp_mem_data;
    v_req_mem_path_o = queue_rdata_o.read()[70];
    vb_req_mem_type_o = queue_rdata_o.read()(69, 67);
    vb_req_mem_size_o = queue_rdata_o.read()(66, 64);
    vb_req_mem_addr_o = queue_rdata_o.read()(63, 0);

    o_req_mem_valid = queue_nempty_o;
    o_req_mem_path = v_req_mem_path_o;
    o_req_mem_type = vb_req_mem_type_o;
    o_req_mem_size = vb_req_mem_size_o;
    o_req_mem_addr = vb_req_mem_addr_o;
    o_req_mem_strob = d.req_mem_strob;
    o_req_mem_data = d.req_mem_wdata;
}

}  // namespace debugger

