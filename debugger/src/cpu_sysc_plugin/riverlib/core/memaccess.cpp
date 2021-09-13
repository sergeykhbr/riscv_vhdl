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
    i_e_pc("i_e_pc"),
    i_e_instr("i_e_instr"),
    i_e_wtag("i_e_wtag"),
    i_e_flushd("i_e_flushd"),
    o_flushd("o_flushd"),
    i_memop_valid("i_memop_valid"),
    i_memop_waddr("i_memop_waddr"),
    i_memop_wdata("i_memop_wdata"),
    i_memop_sign_ext("i_memop_sign_ext"),
    i_memop_type("i_memop_type"),
    i_memop_size("i_memop_size"),
    i_memop_addr("i_memop_addr"),
    o_memop_ready("o_memop_ready"),
    o_wb_wena("o_wb_wena"),
    o_wb_waddr("o_wb_waddr"),
    o_wb_wdata("o_wb_wdata"),
    o_wb_wtag("o_wb_wtag"),
    i_wb_ready("i_wb_ready"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_valid("o_mem_valid"),
    o_mem_write("o_mem_write"),
    o_mem_addr("o_mem_addr"),
    o_mem_wdata("o_mem_wdata"),
    o_mem_wstrb("o_mem_wstrb"),
    o_mem_size("o_mem_size"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    o_mem_resp_ready("o_mem_resp_ready") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_e_wtag;
    sensitive << i_e_flushd;
    sensitive << i_memop_valid;
    sensitive << i_memop_sign_ext;
    sensitive << i_memop_type;
    sensitive << i_memop_size;
    sensitive << i_memop_addr;
    sensitive << i_memop_waddr;
    sensitive << i_memop_wdata;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
    sensitive << i_wb_ready;
    sensitive << queue_data_o;
    sensitive << queue_nempty;
    sensitive << queue_full;
    sensitive << r.state;
    sensitive << r.memop_w;
    sensitive << r.memop_addr;
    sensitive << r.memop_wdata;
    sensitive << r.memop_wstrb;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.memop_res_pc;
    sensitive << r.memop_res_instr;
    sensitive << r.memop_res_addr;
    sensitive << r.memop_res_wtag;
    sensitive << r.memop_res_data;
    sensitive << r.memop_res_wena;
    sensitive << r.hold_rdata;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    queue0 = new Queue<2, QUEUE_WIDTH>("queue0", async_reset);
    queue0->i_clk(i_clk);
    queue0->i_nrst(i_nrst);
    queue0->i_re(queue_re);
    queue0->i_we(queue_we);
    queue0->i_wdata(queue_data_i);
    queue0->o_rdata(queue_data_o);
    queue0->o_full(queue_full);
    queue0->o_nempty(queue_nempty);
};

MemAccess::~MemAccess() {
    delete queue0;
}

void MemAccess::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_e_wtag, i_e_wtag.name());
        sc_trace(o_vcd, i_e_flushd, i_e_flushd.name());
        sc_trace(o_vcd, i_memop_valid, i_memop_valid.name());
        sc_trace(o_vcd, i_memop_waddr, i_memop_waddr.name());
        sc_trace(o_vcd, i_memop_wdata, i_memop_wdata.name());
        sc_trace(o_vcd, i_memop_addr, i_memop_addr.name());
        sc_trace(o_vcd, i_memop_type, i_memop_type.name());
        sc_trace(o_vcd, i_memop_size, i_memop_size.name());
        sc_trace(o_vcd, o_mem_valid, o_mem_valid.name());
        sc_trace(o_vcd, o_mem_write, o_mem_write.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, o_mem_wdata, o_mem_wdata.name());
        sc_trace(o_vcd, o_mem_wstrb, o_mem_wstrb.name());
        sc_trace(o_vcd, o_mem_size, o_mem_size.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());

        sc_trace(o_vcd, o_memop_ready, o_memop_ready.name());
        sc_trace(o_vcd, o_wb_wena, o_wb_wena.name());
        sc_trace(o_vcd, o_wb_waddr, o_wb_waddr.name());
        sc_trace(o_vcd, o_wb_wdata, o_wb_wdata.name());
        sc_trace(o_vcd, o_wb_wtag, o_wb_wtag.name());
        sc_trace(o_vcd, i_wb_ready, i_wb_ready.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".state");
        sc_trace(o_vcd, r.memop_addr, pn + ".r_memop_addr");
        sc_trace(o_vcd, r.memop_res_pc, pn + ".r_memop_res_pc");
        sc_trace(o_vcd, queue_data_i, pn + ".queue_data_i");
        sc_trace(o_vcd, queue_data_o, pn + ".queue_data_o");
        sc_trace(o_vcd, queue_nempty, pn + ".queue_nempty");
        sc_trace(o_vcd, queue_full, pn + ".queue_full");
        sc_trace(o_vcd, queue_we, pn + ".queue_we");
        sc_trace(o_vcd, queue_re, pn + ".queue_re");
    }
    queue0->generateVCD(i_vcd, o_vcd);
}

void MemAccess::comb() {
    sc_uint<64> vb_memop_wdata;
    sc_uint<8> vb_memop_wstrb;
    bool v_mem_valid;
    bool v_mem_write;
    bool v_mem_sign_ext;
    sc_uint<2> vb_mem_sz;
    sc_uint<CFG_CPU_ADDR_BITS> vb_mem_addr;
    sc_uint<64> vb_mem_rdata;
    bool v_queue_re;
    bool v_flushd;
    sc_uint<2> vb_res_wtag;
    sc_uint<64> vb_mem_wdata;
    sc_uint<8> vb_mem_wstrb;
    sc_uint<64> vb_mem_resp_shifted;
    sc_uint<64> vb_mem_data_unsigned;
    sc_uint<64> vb_mem_data_signed;
    sc_uint<RISCV_ARCH> vb_res_data;
    sc_uint<6> vb_res_addr;
    sc_uint<CFG_CPU_ADDR_BITS> vb_e_pc;
    sc_uint<32> vb_e_instr;
    bool v_memop_ready;
    bool v_o_wena;
    sc_uint<6> vb_o_waddr;
    sc_uint<RISCV_ARCH> vb_o_wdata;
    sc_uint<2> vb_o_wtag;

    v = r;

    v_mem_valid = 0;
    v_queue_re = 0;

    vb_mem_resp_shifted = 0;
    vb_mem_data_unsigned = 0;
    vb_mem_data_signed = 0;

    vb_memop_wdata = 0;
    vb_memop_wstrb = 0;

    v_o_wena = 0;
    vb_o_waddr = 0;
    vb_o_wdata = 0;
    vb_o_wtag = 0;

    switch (i_memop_size.read()) {
    case 0:
        vb_memop_wdata = (i_memop_wdata.read()(7, 0),
            i_memop_wdata.read()(7, 0), i_memop_wdata.read()(7, 0),
            i_memop_wdata.read()(7, 0), i_memop_wdata.read()(7, 0),
            i_memop_wdata.read()(7, 0), i_memop_wdata.read()(7, 0),
            i_memop_wdata.read()(7, 0));
        if (i_memop_addr.read()(2, 0) == 0x0) {
            vb_memop_wstrb = 0x01;
        } else if (i_memop_addr.read()(2, 0) == 0x1) {
            vb_memop_wstrb = 0x02;
        } else if (i_memop_addr.read()(2, 0) == 0x2) {
            vb_memop_wstrb = 0x04;
        } else if (i_memop_addr.read()(2, 0) == 0x3) {
            vb_memop_wstrb = 0x08;
        } else if (i_memop_addr.read()(2, 0) == 0x4) {
            vb_memop_wstrb = 0x10;
        } else if (i_memop_addr.read()(2, 0) == 0x5) {
            vb_memop_wstrb = 0x20;
        } else if (i_memop_addr.read()(2, 0) == 0x6) {
            vb_memop_wstrb = 0x40;
        } else if (i_memop_addr.read()(2, 0) == 0x7) {
            vb_memop_wstrb = 0x80;
        }
        break;
    case 1:
        vb_memop_wdata = (i_memop_wdata.read()(15, 0),
            i_memop_wdata.read()(15, 0), i_memop_wdata.read()(15, 0),
            i_memop_wdata.read()(15, 0));
        if (i_memop_addr.read()(2, 1) == 0) {
            vb_memop_wstrb = 0x03;
        } else if (i_memop_addr.read()(2, 1) == 1) {
            vb_memop_wstrb = 0x0C;
        } else if (i_memop_addr.read()(2, 1) == 2) {
            vb_memop_wstrb = 0x30;
        } else {
            vb_memop_wstrb = 0xC0;
        }
        break;
    case 2:
        vb_memop_wdata = (i_memop_wdata.read()(31, 0),
                    i_memop_wdata.read()(31, 0));
        if (i_memop_addr.read()[2]) {
            vb_memop_wstrb = 0xF0;
        } else {
            vb_memop_wstrb = 0x0F;
        }
        break;
    case 3:
        vb_memop_wdata = i_memop_wdata;
        vb_memop_wstrb = 0xFF;
        break;
    default:;
    }

    // Form Queue inputs:
    queue_data_i = (i_e_flushd, i_e_wtag, vb_memop_wdata, vb_memop_wstrb,
                    i_memop_wdata, i_memop_waddr, i_e_instr, i_e_pc,
                    i_memop_size, i_memop_sign_ext, i_memop_type,
                    i_memop_addr);
    queue_we = i_memop_valid | i_e_flushd;

    // Split Queue outputs:
    v_flushd = queue_data_o.read()[2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+44];
    vb_res_wtag = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+43,
                                      2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+42);
    vb_mem_wdata = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+42-1,
                                      2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+42);
    vb_mem_wstrb = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+42-1,
                                      2*CFG_CPU_ADDR_BITS+RISCV_ARCH+42);
    vb_res_data = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+42-1,
                                      2*CFG_CPU_ADDR_BITS+42);
    vb_res_addr = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+42-1,
                                      2*CFG_CPU_ADDR_BITS+36);
    vb_e_instr = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+36-1,
                                     2*CFG_CPU_ADDR_BITS+4);
    vb_e_pc = queue_data_o.read()(2*CFG_CPU_ADDR_BITS+4-1, CFG_CPU_ADDR_BITS+4);
    vb_mem_sz = queue_data_o.read()(CFG_CPU_ADDR_BITS+3, CFG_CPU_ADDR_BITS+2);
    v_mem_sign_ext = queue_data_o.read()[CFG_CPU_ADDR_BITS+1];
    v_mem_write = queue_data_o.read()[CFG_CPU_ADDR_BITS];
    vb_mem_addr = queue_data_o.read()(CFG_CPU_ADDR_BITS-1, 0);

    switch (r.memop_addr.read()(2, 0)) {
    case 1:
        vb_mem_resp_shifted = i_mem_data.read()(63, 8);
        break;
    case 2:
        vb_mem_resp_shifted = i_mem_data.read()(63, 16);
        break;
    case 3:
        vb_mem_resp_shifted = i_mem_data.read()(63, 24);
        break;
    case 4:
        vb_mem_resp_shifted = i_mem_data.read()(63, 32);
        break;
    case 5:
        vb_mem_resp_shifted = i_mem_data.read()(63, 40);
        break;
    case 6:
        vb_mem_resp_shifted = i_mem_data.read()(63, 48);
        break;
    case 7:
        vb_mem_resp_shifted = i_mem_data.read()(63, 56);
        break;
    default:
        vb_mem_resp_shifted = i_mem_data.read();
    } 

    switch (r.memop_size.read()) {
    case MEMOP_1B:
        vb_mem_data_unsigned(7, 0) = vb_mem_resp_shifted(7, 0);
        vb_mem_data_signed(7, 0) = vb_mem_resp_shifted(7, 0);
        if (vb_mem_resp_shifted[7]) {
            vb_mem_data_signed(63, 8) = ~0;
        }
        break;
    case MEMOP_2B:
        vb_mem_data_unsigned(15, 0) = vb_mem_resp_shifted(15, 0);
        vb_mem_data_signed(15, 0) = vb_mem_resp_shifted(15, 0);
        if (vb_mem_resp_shifted[15]) {
            vb_mem_data_signed(63, 16) = ~0;
        }
        break;
    case MEMOP_4B:
        vb_mem_data_unsigned(31, 0) = vb_mem_resp_shifted(31, 0);
        vb_mem_data_signed(31, 0) = vb_mem_resp_shifted(31, 0);
        if (vb_mem_resp_shifted[31]) {
            vb_mem_data_signed(63, 32) = ~0;
        }
        break;
    default:
        vb_mem_data_unsigned = vb_mem_resp_shifted;
        vb_mem_data_signed = vb_mem_resp_shifted;
    }

    if (r.memop_w.read() == 0) {
        if (r.memop_sign_ext.read() == 1) {
            vb_mem_rdata = vb_mem_data_signed;
        } else {
            vb_mem_rdata = vb_mem_data_unsigned;
        }
    } else {
        vb_mem_rdata = r.memop_res_data;
    }

    switch (r.state.read()) {
    case State_Idle:
        v_queue_re = 1;
        if (queue_nempty.read() == 1) {
            v_mem_valid = !v_flushd;
            v.memop_res_pc = vb_e_pc;
            v.memop_res_instr = vb_e_instr;
            v.memop_res_addr = vb_res_addr;
            v.memop_res_wtag = vb_res_wtag;
            v.memop_res_data = vb_res_data;
            v.memop_res_wena = vb_res_addr.or_reduce();
            v.memop_addr = vb_mem_addr;
            v.memop_wdata = vb_mem_wdata;
            v.memop_wstrb = vb_mem_wstrb;
            v.memop_w = v_mem_write;
            v.memop_sign_ext = v_mem_sign_ext;
            v.memop_size = vb_mem_sz;

            if (v_flushd == 1) {
                // do nothing
            } else if (i_mem_req_ready.read() == 1) {
                v.state = State_WaitResponse;
            } else {
                v.state = State_WaitReqAccept;
            }
        }
        break;
    case State_WaitReqAccept:
        v_mem_valid = 1;
        v_mem_write = r.memop_w.read();
        vb_mem_sz = r.memop_size.read();
        vb_mem_addr = r.memop_addr.read();
        vb_mem_wdata = r.memop_wdata.read();
        vb_mem_wstrb = r.memop_wstrb.read();
        vb_res_data = r.memop_res_data.read();
        if (i_mem_req_ready.read() == 1) {
            v.state = State_WaitResponse;
        }
        break;
    case State_WaitResponse:
        if (i_mem_data_valid.read() == 0) {
            // Do nothing
        } else {
            v_o_wena = r.memop_res_wena.read();
            vb_o_waddr = r.memop_res_addr.read();
            vb_o_wdata = vb_mem_rdata;
            vb_o_wtag = r.memop_res_wtag.read();

            v_queue_re = 1;
            if (r.memop_res_wena.read() == 1 && i_wb_ready.read() == 0) {
                // Inject only one clock hold-on and wait a couple of clocks
                v_queue_re = 0;
                v.state = State_Hold;
                v.hold_rdata = vb_mem_rdata;
            } else if (queue_nempty.read() == 1) {
                v_mem_valid = !v_flushd;
                v.memop_res_pc = vb_e_pc;
                v.memop_res_instr = vb_e_instr;
                v.memop_res_addr = vb_res_addr;
                v.memop_res_wtag = vb_res_wtag;
                v.memop_res_data = vb_res_data;
                v.memop_res_wena = vb_res_addr.or_reduce();
                v.memop_addr = vb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_w = v_mem_write;
                v.memop_sign_ext = v_mem_sign_ext;
                v.memop_size = vb_mem_sz;

                if (v_flushd == 1) {
                    v.state = State_Idle;
                } else if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_Idle;
            }
        }
        break;
    case State_Hold:
        v_o_wena = r.memop_res_wena.read();
        vb_o_waddr = r.memop_res_addr.read();
        vb_o_wdata = r.hold_rdata.read();
        vb_o_wtag = r.memop_res_wtag.read();
        if (i_wb_ready.read() == 1) {
            v_queue_re = 1;
            if (queue_nempty.read() == 1) {
                v_mem_valid = !v_flushd;
                v.memop_res_pc = vb_e_pc;
                v.memop_res_instr = vb_e_instr;
                v.memop_res_addr = vb_res_addr;
                v.memop_res_wtag = vb_res_wtag;
                v.memop_res_data = vb_res_data;
                v.memop_res_wena = vb_res_addr.or_reduce();
                v.memop_addr = vb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_w = v_mem_write;
                v.memop_sign_ext = v_mem_sign_ext;
                v.memop_size = vb_mem_sz;

                if (v_flushd == 1) {
                    v.state = State_Idle;
                } else if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_Idle;
            }
        }
        break;
    default:;
    }


    v_memop_ready = 1;
    if (queue_full.read() == 1) {
        v_memop_ready = 0;
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    queue_re = v_queue_re;

    o_flushd = queue_nempty.read() && v_flushd && v_queue_re;
    o_mem_resp_ready = 1;

    o_mem_valid = v_mem_valid;
    o_mem_write = v_mem_write;
    o_mem_addr = vb_mem_addr;
    o_mem_wdata = vb_mem_wdata;
    o_mem_wstrb = vb_mem_wstrb;
    o_mem_size = vb_mem_sz;

    o_memop_ready = v_memop_ready;
    o_wb_wena = v_o_wena;
    o_wb_waddr = vb_o_waddr;
    o_wb_wdata = vb_o_wdata;
    o_wb_wtag = vb_o_wtag;
}

void MemAccess::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

