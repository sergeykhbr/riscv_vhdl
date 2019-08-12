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

#include "memaccess.h"

namespace debugger {

MemAccess::MemAccess(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_e_valid("i_e_valid"),
    i_e_pc("i_e_pc"),
    i_e_instr("i_e_instr"),
    i_res_addr("i_res_addr"),
    i_res_data("i_res_data"),
    i_memop_sign_ext("i_memop_sign_ext"),
    i_memop_load("i_memop_load"),
    i_memop_store("i_memop_store"),
    i_memop_size("i_memop_size"),
    i_memop_addr("i_memop_addr"),
    o_wena("o_wena"),
    o_waddr("o_waddr"),
    o_wdata("o_wdata"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_valid("o_mem_valid"),
    o_mem_write("o_mem_write"),
    o_mem_sz("o_mem_sz"),
    o_mem_addr("o_mem_addr"),
    o_mem_data("o_mem_data"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    o_mem_resp_ready("o_mem_resp_ready"),
    o_hold("o_hold"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_instr("o_instr") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mem_req_ready;
    sensitive << i_e_valid;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_res_addr;
    sensitive << i_res_data;
    sensitive << i_memop_sign_ext;
    sensitive << i_memop_load;
    sensitive << i_memop_store;
    sensitive << i_memop_size;
    sensitive << i_memop_addr;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
#ifdef MEM_V2
    sensitive << r.valid;
    sensitive << r.memop_r;
    sensitive << r.memop_rw;
    sensitive << r.pc;
    sensitive << r.instr;
    sensitive << r.res_addr;
    sensitive << r.res_data;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.wena;
#else
    sensitive << r.valid;
    sensitive << r.wdata;
    sensitive << r.wait_req;
    sensitive << r.wait_resp;
#endif

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void MemAccess::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_memop_store, i_memop_store.name());
        sc_trace(o_vcd, i_memop_load, i_memop_load.name());
        sc_trace(o_vcd, o_mem_valid, o_mem_valid.name());
        sc_trace(o_vcd, o_mem_write, o_mem_write.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_sz, o_mem_sz.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, o_mem_data, o_mem_data.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());

        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_wena, o_wena.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_wdata, o_wdata.name());
        sc_trace(o_vcd, o_hold, o_hold.name());
#ifdef MEM_V2
#else
        sc_trace(o_vcd, r.wait_resp, "/top/proc0/mem0/r_wait_resp");
#endif
    }
}

void MemAccess::comb() {
#ifdef MEM_V2
    bool w_hold_req;
    bool w_hold_resp;
    bool w_hold;
    bool w_memop_r;
    bool w_memop_w;
    bool w_valid;
    sc_uint<RISCV_ARCH> wb_mem_data_signext;
    sc_uint<RISCV_ARCH> wb_res_data;

    v = r;

    w_memop_r = i_memop_load.read() && i_e_valid.read();
    w_memop_w = i_memop_store.read() && i_e_valid.read();

    w_valid = 0;
    w_hold_req = 0;
    w_hold_resp = 0;
    if (i_e_valid.read() == 1) {
        w_valid = 1;
        if (r.memop_rw.read() == 1 && i_mem_data_valid.read() == 0) {
            w_hold_resp = 1;
            w_valid = 0;
        } else if (w_memop_r || w_memop_w) {
            if (i_mem_req_ready.read() == 0) {
                w_hold_req = 1;
                w_valid = 0;
            }
        }
    }

    if (w_valid == 1) {
        v.valid = 1;
        v.memop_r = w_memop_r;
        v.memop_rw = w_memop_r || w_memop_w;
        v.pc = i_e_pc.read();
        v.instr = i_e_instr.read();
        v.res_addr = i_res_addr.read();
        v.res_data = i_res_data.read();
        v.memop_sign_ext = i_memop_sign_ext.read();
        v.memop_size = i_memop_size.read();
        if (i_res_addr.read() == 0) {
            v.wena = 0;
        } else {
            v.wena = 1;
        }
    } else if ((r.memop_rw.read() == 1 && i_mem_data_valid.read() == 1)
            || (r.memop_rw.read() == 0 && r.valid.read() == 1)) {
        v.valid = 0;
        v.memop_r = 0;
        v.memop_rw = 0;
        v.pc = 0;
        v.instr = 0;
        v.res_addr = 0;
        v.res_data = 0;
        v.memop_sign_ext = 0;
        v.memop_size = 0;
        v.wena = 0;
    }

    switch (r.memop_size.read()) {
    case MEMOP_1B:
        wb_mem_data_signext = i_mem_data;
        if (i_mem_data.read()[7]) {
            wb_mem_data_signext(63, 8) = ~0;
        }
        break;
    case MEMOP_2B:
        wb_mem_data_signext = i_mem_data;
        if (i_mem_data.read()[15]) {
            wb_mem_data_signext(63, 16) = ~0;
        }
        break;
    case MEMOP_4B:
        wb_mem_data_signext = i_mem_data;
        if (i_mem_data.read()[31]) {
            wb_mem_data_signext(63, 32) = ~0;
        }
        break;
    default:
        wb_mem_data_signext = i_mem_data;
    }

    if (r.memop_r.read() == 1) {
        if (r.memop_sign_ext.read() == 1) {
            wb_res_data = wb_mem_data_signext;
        } else {
            wb_res_data = i_mem_data;
        }
    } else {
        wb_res_data = r.res_data;
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    w_hold = w_hold_req || w_hold_resp;
    o_mem_resp_ready = 1;

    o_mem_valid = w_memop_r || w_memop_w;
    o_mem_write = w_memop_w;
    o_mem_sz = i_memop_size.read();
    o_mem_addr = i_memop_addr.read();
    o_mem_data = i_res_data.read();

    o_wena = r.wena;
    o_waddr = r.res_addr;
    o_wdata = wb_res_data;
    o_valid = r.valid.read() && !w_hold;
    o_pc = r.pc;
    o_instr = r.instr;
    o_hold = w_hold;

#else
    bool w_o_mem_valid;
    bool w_o_mem_write;
    sc_uint<2> wb_o_mem_sz;
    sc_uint<BUS_ADDR_WIDTH> wb_o_mem_addr;
    sc_uint<RISCV_ARCH> wb_o_mem_wdata;
    sc_uint<RISCV_ARCH> wb_res_wdata;
    bool w_memop;
    bool w_o_valid;
    bool w_o_wena;
    bool w_o_hold;
    bool w_mem_fire;

    v = r;

    w_o_mem_valid = 0;
    w_o_mem_write = 0;
    wb_o_mem_sz = 0;
    wb_o_mem_addr = 0;
    wb_o_mem_wdata = 0;
    v.valid = 0;
    w_o_hold = 0;

    w_memop = i_memop_load.read() || i_memop_store.read();

    if (r.wait_req.read()) {
        if (i_mem_req_ready.read()) {
            v.wait_req = 0;
            v.wait_resp = 1;
        }
        w_o_mem_valid = 1;
        w_o_mem_write = r.wait_req_write;
        wb_o_mem_sz = r.wait_req_sz;
        wb_o_mem_addr = r.wait_req_addr;
        wb_o_mem_wdata = r.wait_req_wdata;
    } else if (i_e_valid.read()) {
        v.valid = !w_memop;
        v.pc = i_e_pc;
        v.instr = i_e_instr;
        v.waddr = i_res_addr;
        v.wdata = i_res_data;
        if (i_res_addr.read() == 0) {
            v.wena = 0;
        } else {
            v.wena = 1;
        }

        if (w_memop) {
            w_o_mem_valid = 1;
            w_o_mem_write = i_memop_store;
            wb_o_mem_sz = i_memop_size;
            wb_o_mem_addr = i_memop_addr;
            wb_o_mem_wdata = i_res_data;
            v.sign_ext = i_memop_sign_ext;
            v.size = i_memop_size;

            v.wait_resp = i_mem_req_ready;
            v.wait_req = !i_mem_req_ready;
            v.wait_req_write = i_memop_store;
            v.wait_req_sz = i_memop_size;
            v.wait_req_addr = i_memop_addr;
            v.wait_req_wdata = i_res_data;
        } else {
            w_o_mem_valid = 0;
            w_o_mem_write = 0;
            wb_o_mem_sz = 0;
            wb_o_mem_addr = 0;
            wb_o_mem_wdata = 0;
            v.sign_ext = 0;
            v.size = 0;
            v.wait_req_addr = 0;
            v.wait_req = 0;
            v.wait_resp = 0;
        }
    } else if (i_mem_data_valid.read()) {
        v.wait_resp = 0;
    }

    w_o_hold = (i_e_valid.read() && w_memop) || r.wait_req.read() 
            || (r.wait_resp.read() && !i_mem_data_valid.read());

    w_mem_fire = i_mem_data_valid;
    if (w_mem_fire) {
        if (r.sign_ext.read()) {
            switch (r.size.read()) {
            case MEMOP_1B:
                wb_res_wdata = i_mem_data;
                if (i_mem_data.read()[7]) {
                    wb_res_wdata(63, 8) = ~0;
                }
                break;
            case MEMOP_2B:
                wb_res_wdata = i_mem_data;
                if (i_mem_data.read()[15]) {
                    wb_res_wdata(63, 16) = ~0;
                }
                break;
            case MEMOP_4B:
                wb_res_wdata = i_mem_data;
                if (i_mem_data.read()[31]) {
                    wb_res_wdata(63, 32) = ~0;
                }
                break;
            default:
                wb_res_wdata = i_mem_data;
            }
        } else {
            wb_res_wdata = i_mem_data;
        }
    } else {
        wb_res_wdata = r.wdata;
    }

    w_o_valid = r.valid.read() || w_mem_fire;
    w_o_wena = r.wena & w_o_valid;

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_mem_resp_ready = 1;

    o_mem_valid = w_o_mem_valid;
    o_mem_write = w_o_mem_write;
    o_mem_sz = wb_o_mem_sz;
    o_mem_addr = wb_o_mem_addr;
    o_mem_data = wb_o_mem_wdata;

    o_wena = w_o_wena;
    o_waddr = r.waddr;
    o_wdata = wb_res_wdata;
    o_valid = w_o_valid;
    o_pc = r.pc;
    o_instr = r.instr;
    o_hold = w_o_hold;
#endif
}

void MemAccess::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

