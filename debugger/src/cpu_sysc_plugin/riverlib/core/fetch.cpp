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

#include "fetch.h"

namespace debugger {

InstrFetch::InstrFetch(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_pipeline_hold("i_pipeline_hold"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_addr_valid("o_mem_addr_valid"),
    o_mem_addr("o_mem_addr"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    i_mem_load_fault("i_mem_load_fault"),
    i_mem_executable("i_mem_executable"),
    o_mem_resp_ready("o_mem_resp_ready"),
    i_flush_pipeline("i_flush_pipeline"),
    i_progbuf_ena("i_progbuf_ena"),
    i_progbuf_pc("i_progbuf_pc"),
    i_progbuf_instr("i_progbuf_instr"),
    i_predict_npc("i_predict_npc"),
    o_mem_req_fire("o_mem_req_fire"),
    o_instr_load_fault("o_instr_load_fault"),
    o_instr_executable("o_instr_executable"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_instr("o_instr"),
    o_hold("o_hold") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_pipeline_hold;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_mem_load_fault;
    sensitive << i_mem_executable;
    sensitive << i_flush_pipeline;
    sensitive << i_progbuf_ena;
    sensitive << i_progbuf_pc;
    sensitive << i_progbuf_instr;
    sensitive << i_predict_npc;
    sensitive << r.wait_resp;
    sensitive << r.resp_address;
    sensitive << r.resp_data;
    sensitive << r.resp_valid;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_executable;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void InstrFetch::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_load_fault, i_mem_load_fault.name());
        sc_trace(o_vcd, i_mem_executable, i_mem_executable.name());
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, i_progbuf_ena, i_progbuf_ena.name());
        sc_trace(o_vcd, i_progbuf_pc, i_progbuf_pc.name());
        sc_trace(o_vcd, i_progbuf_instr, i_progbuf_instr.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());
        sc_trace(o_vcd, i_predict_npc, i_predict_npc.name());
        sc_trace(o_vcd, i_pipeline_hold, i_pipeline_hold.name());
        sc_trace(o_vcd, o_mem_addr_valid, o_mem_addr_valid.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_hold, o_hold.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
        sc_trace(o_vcd, o_instr_executable, o_instr_executable.name());

        std::string pn(name());
        sc_trace(o_vcd, r.wait_resp, pn + ".r_wait_resp");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
    }
}

void InstrFetch::comb() {
    bool w_o_req_valid;
    bool w_o_req_fire;
    bool w_o_hold;
    bool w_o_valid;
    sc_uint<CFG_CPU_ADDR_BITS> wb_o_pc;
    sc_uint<32> wb_o_instr;

    v = r;

    w_o_req_valid = !i_pipeline_hold.read()
            & !(r.wait_resp.read() & !i_mem_data_valid.read())
            & !i_progbuf_ena.read();
    w_o_req_fire =  i_mem_req_ready.read() && w_o_req_valid;

    w_o_hold = !(r.wait_resp.read() && i_mem_data_valid.read())
             & !i_progbuf_ena.read();

    // Debug last fetched instructions buffer:
    if (w_o_req_fire) {
        v.wait_resp = 1;
    } else if (i_mem_data_valid.read() == 1 && i_pipeline_hold.read() == 0) {
        v.wait_resp = 0;
    }

    if (i_mem_data_valid.read() && r.wait_resp.read() && !i_pipeline_hold.read()) {
        v.resp_valid = 1;
        v.resp_address = i_mem_data_addr.read();
        v.resp_data = i_mem_data.read();
        v.instr_load_fault = i_mem_load_fault.read();
        v.instr_executable = i_mem_executable.read();
    }
    if (i_flush_pipeline.read() == 1) {
        // Clear pipeline stage
        v.resp_address = ~0ull;
    }
    wb_o_pc = r.resp_address.read();

    if (i_progbuf_ena.read() == 1) {
        wb_o_instr = i_progbuf_instr.read();
        wb_o_pc    = i_progbuf_pc.read();
    } else {
        wb_o_instr = r.resp_data;
        wb_o_instr = r.resp_data.read();
    }
    w_o_valid = (r.resp_valid.read() || i_progbuf_ena.read())
            && !(i_pipeline_hold.read() || w_o_hold);


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_mem_addr_valid = w_o_req_valid;
    o_mem_addr = i_predict_npc.read();
    o_mem_req_fire = w_o_req_fire;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_executable = r.instr_executable;
    o_valid = w_o_valid;
    o_pc = wb_o_pc;
    o_instr = wb_o_instr;
    o_mem_resp_ready = r.wait_resp.read() && !i_pipeline_hold.read();
    o_hold = w_o_hold;
}

void InstrFetch::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

