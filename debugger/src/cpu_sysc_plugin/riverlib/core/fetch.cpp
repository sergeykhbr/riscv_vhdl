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
    i_bp_valid("i_bp_valid"),
    i_bp_pc("i_bp_pc"),
    o_requested_pc("o_requested_pc"),
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
    o_instr_load_fault("o_instr_load_fault"),
    o_instr_executable("o_instr_executable"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_instr("o_instr") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_bp_valid;
    sensitive << i_bp_pc;
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
    sensitive << r.state;
    sensitive << r.req_valid;
    sensitive << r.resp_ready;
    sensitive << r.buf_valid;
    sensitive << r.req_addr;
    sensitive << r.requested_pc;
    sensitive << r.pc;
    sensitive << r.instr;
    sensitive << r.valid;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_executable;
    sensitive << r.progbuf_ena;

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
        sc_trace(o_vcd, i_bp_pc, i_bp_pc.name());
        sc_trace(o_vcd, i_bp_valid, i_bp_valid.name());
        sc_trace(o_vcd, o_mem_addr_valid, o_mem_addr_valid.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_requested_pc, o_requested_pc.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
        sc_trace(o_vcd, o_instr_executable, o_instr_executable.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.buf_valid, pn + ".r_buf_valid");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
    }
}

void InstrFetch::comb() {
    bool v_now_req_valid;

    v = r;

    v_now_req_valid = 0;
    v.valid = 0;

    switch (r.state.read()) {
    case Idle:
        v.req_valid = 0;
        v.resp_ready = 0;
        v.buf_valid = 0;
        v.progbuf_ena = 0;
        v.requested_pc = ~0ull;
        if (i_progbuf_ena.read()) {
            // Execution from buffer
            v.valid = 1;
            v.progbuf_ena = 1;
            v.pc = i_progbuf_pc.read();
            v.instr = i_progbuf_instr;
            v.instr_load_fault = 0;
            v.instr_executable = 0;
        } else if (i_bp_valid) {
            v.state = WaitReqAccept;
            v.req_valid = 1;
            v.req_addr = i_bp_pc;
            v.requested_pc = i_bp_pc;
        }
        break;
    case WaitReqAccept:
        if (i_mem_req_ready) {
            v.state = WaitResp;
            v.req_valid = 0;
            v.resp_ready = 1;
            if (i_bp_valid && !i_progbuf_ena) {
                v.buf_valid = 1;
                v.req_addr = i_bp_pc;
            }
        }
        break;
    case WaitResp:
        if (i_bp_valid && !i_progbuf_ena) {
            v.buf_valid = 1;
            v.req_addr = i_bp_pc;
        }
        if (i_mem_data_valid) {
            v.resp_ready = 0;
            v.buf_valid = 0;
            v.requested_pc = ~0ull;
            v.valid = 1;
            v.pc = i_mem_data_addr;
            v.instr = i_mem_data;
            v.instr_load_fault = i_mem_load_fault.read();
            v.instr_executable = i_mem_executable.read();
            if (r.buf_valid) {
                v_now_req_valid = 1;
                v.requested_pc = r.req_addr;
                if (!i_mem_req_ready) {
                    v.state = WaitReqAccept;
                    v.req_valid = 1;
                } else {
                    v.resp_ready = 1;
                }
            } else if (i_bp_valid && !i_progbuf_ena) {
                v.state = WaitReqAccept;
                v.req_valid = 1;
                v.req_addr = i_bp_pc;
                v.requested_pc = i_bp_pc;
            } else {
                v.state = Idle;
            }
        }
        break;
    default:;
    }

    if (i_flush_pipeline.read() == 1) {
        // Clear pipeline stage
        v.pc = ~0ull;
        v.instr = 0;
        v.instr_load_fault = 0;
        v.instr_executable = 0;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_mem_addr_valid = r.req_valid || v_now_req_valid;
    o_mem_addr = r.req_addr;
    o_mem_resp_ready = r.resp_ready;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_executable = r.instr_executable;
    o_valid = r.valid;
    o_requested_pc = r.requested_pc;
    o_pc = r.pc;
    o_instr = r.instr;
}

void InstrFetch::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

