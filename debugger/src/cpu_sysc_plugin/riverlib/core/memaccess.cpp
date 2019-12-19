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
    o_memop_ready("o_memop_ready"),
    o_waddr("o_waddr"),
    o_wdata("o_wdata"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_valid("o_mem_valid"),
    o_mem_write("o_mem_write"),
    o_mem_addr("o_mem_addr"),
    o_mem_wdata("o_mem_wdata"),
    o_mem_wstrb("o_mem_wstrb"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    o_mem_resp_ready("o_mem_resp_ready"),
    o_hold("o_hold"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_instr("o_instr"),
    o_wb_memop("o_wb_memop") {
    async_reset_ = async_reset;

    SC_METHOD(main);
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

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_e_valid;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_res_addr;
    sensitive << i_res_data;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
    sensitive << queue_data_o;
    sensitive << queue_nempty;
    sensitive << queue_full;
    sensitive << r.state;
    sensitive << r.memop_r;
    sensitive << r.memop_addr;
    sensitive << r.memop_wdata;
    sensitive << r.memop_wstrb;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.memop_res_pc;
    sensitive << r.memop_res_instr;
    sensitive << r.memop_res_addr;
    sensitive << r.memop_res_data;
    sensitive << r.memop_res_wena;
    sensitive << r.reg_wb_valid;
    sensitive << r.reg_res_pc;
    sensitive << r.reg_res_instr;
    sensitive << r.reg_res_addr;
    sensitive << r.reg_res_data;
    sensitive << r.reg_res_wena;

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
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_res_addr, i_res_addr.name());
        sc_trace(o_vcd, i_res_data, i_res_data.name());
        sc_trace(o_vcd, i_memop_addr, i_memop_addr.name());
        sc_trace(o_vcd, i_memop_store, i_memop_store.name());
        sc_trace(o_vcd, i_memop_load, i_memop_load.name());
        sc_trace(o_vcd, i_memop_size, i_memop_size.name());
        sc_trace(o_vcd, o_mem_valid, o_mem_valid.name());
        sc_trace(o_vcd, o_mem_write, o_mem_write.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, o_mem_wdata, o_mem_wdata.name());
        sc_trace(o_vcd, o_mem_wstrb, o_mem_wstrb.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());

        sc_trace(o_vcd, o_hold, o_hold.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_wena, o_wena.name());
        sc_trace(o_vcd, o_memop_ready, o_memop_ready.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_wdata, o_wdata.name());
        sc_trace(o_vcd, o_wb_memop, o_wb_memop.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".state");
        sc_trace(o_vcd, r.memop_res_pc, pn + ".memop_res_pc");
        sc_trace(o_vcd, r.reg_wb_valid, pn + ".reg_wb_valid");
        sc_trace(o_vcd, queue_data_i, pn + ".queue_data_i");
        sc_trace(o_vcd, queue_data_o, pn + ".queue_data_o");
        sc_trace(o_vcd, queue_nempty, pn + ".queue_nempty");
        sc_trace(o_vcd, queue_full, pn + ".queue_full");
        sc_trace(o_vcd, queue_we, pn + ".queue_we");
        sc_trace(o_vcd, queue_re, pn + ".queue_re");
    }
    queue0->generateVCD(i_vcd, o_vcd);
}

void MemAccess::main() {
    sc_uint<BUS_DATA_WIDTH> vb_memop_wdata;
    sc_uint<BUS_DATA_BYTES> vb_memop_wstrb;

    switch (i_memop_size.read()) {
    case 0:
        vb_memop_wdata = (i_res_data.read()(7, 0),
            i_res_data.read()(7, 0), i_res_data.read()(7, 0),
            i_res_data.read()(7, 0), i_res_data.read()(7, 0),
            i_res_data.read()(7, 0), i_res_data.read()(7, 0),
            i_res_data.read()(7, 0));
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
        vb_memop_wdata = (i_res_data.read()(15, 0),
            i_res_data.read()(15, 0), i_res_data.read()(15, 0),
            i_res_data.read()(15, 0));
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
        vb_memop_wdata = (i_res_data.read()(31, 0),
                    i_res_data.read()(31, 0));
        if (i_memop_addr.read()[2]) {
            vb_memop_wstrb = 0xF0;
        } else {
            vb_memop_wstrb = 0x0F;
        }
        break;
    case 3:
        vb_memop_wdata = i_res_data;
        vb_memop_wstrb = 0xFF;
        break;
    default:;
    }


    queue_data_i = (vb_memop_wdata, vb_memop_wstrb,
                    i_res_data, i_res_addr, i_e_instr, i_e_pc,
                    i_memop_size, i_memop_sign_ext, i_memop_store,
                    i_memop_addr);
    queue_we = i_e_valid & (i_memop_load | i_memop_store);
}

void MemAccess::comb() {
    bool w_mem_valid;
    bool w_mem_write;
    bool w_mem_sign_ext;
    sc_uint<2> wb_mem_sz;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<BUS_DATA_WIDTH> vb_mem_rdata;
    bool w_hold;
    bool w_wb_memop;
    bool w_queue_re;
    sc_uint<BUS_DATA_WIDTH> vb_mem_wdata;
    sc_uint<BUS_DATA_BYTES> vb_mem_wstrb;
    sc_uint<BUS_DATA_WIDTH> vb_mem_resp_shifted;
    sc_uint<BUS_DATA_WIDTH> vb_mem_data_unsigned;
    sc_uint<BUS_DATA_WIDTH> vb_mem_data_signed;
    sc_uint<RISCV_ARCH> vb_res_data;
    sc_uint<6> wb_res_addr;
    sc_uint<BUS_ADDR_WIDTH> wb_e_pc;
    sc_uint<32> wb_e_instr;

    v = r;

    w_mem_valid = 0;
    w_hold = 0;
    w_wb_memop = 0;
    w_queue_re = 0;

    vb_mem_wdata = queue_data_o.read()(2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+BUS_DATA_WIDTH+42-1,
                                      2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+42);
    vb_mem_wstrb = queue_data_o.read()(2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+42-1,
                                      2*BUS_ADDR_WIDTH+RISCV_ARCH+42);
    vb_res_data = queue_data_o.read()(2*BUS_ADDR_WIDTH+RISCV_ARCH+42-1,
                                      2*BUS_ADDR_WIDTH+42);
    wb_res_addr = queue_data_o.read()(2*BUS_ADDR_WIDTH+42-1,
                                      2*BUS_ADDR_WIDTH+36);
    wb_e_instr = queue_data_o.read()(2*BUS_ADDR_WIDTH+36-1,
                                     2*BUS_ADDR_WIDTH+4);
    wb_e_pc = queue_data_o.read()(2*BUS_ADDR_WIDTH+4-1, BUS_ADDR_WIDTH+4);
    wb_mem_sz = queue_data_o.read()(BUS_ADDR_WIDTH+3, BUS_ADDR_WIDTH+2);
    w_mem_sign_ext = queue_data_o.read()[BUS_ADDR_WIDTH+1];
    w_mem_write = queue_data_o.read()[BUS_ADDR_WIDTH];
    wb_mem_addr = queue_data_o.read()(BUS_ADDR_WIDTH-1, 0);

    switch (r.state.read()) {
    case State_Idle:
        w_queue_re = 1;
        if (queue_nempty.read() == 1) {
            w_mem_valid = 1;
            v.memop_res_pc = wb_e_pc;
            v.memop_res_instr = wb_e_instr;
            v.memop_res_addr = wb_res_addr;
            v.memop_res_data = vb_res_data;
            if (wb_res_addr == 0) {
                v.memop_res_wena = 0;
            } else {
                v.memop_res_wena = 1;
            }
            v.memop_addr = wb_mem_addr;
            v.memop_wdata = vb_mem_wdata;
            v.memop_wstrb = vb_mem_wstrb;
            v.memop_r = !w_mem_write;
            v.memop_sign_ext = w_mem_sign_ext;
            v.memop_size = wb_mem_sz;

            if (i_mem_req_ready.read() == 1) {
                v.state = State_WaitResponse;
            } else {
                v.state = State_WaitReqAccept;
            }
        }
        break;
    case State_WaitReqAccept:
        w_mem_valid = 1;
        w_mem_write = !r.memop_r.read();
        wb_mem_sz = r.memop_size.read();
        wb_mem_addr = r.memop_addr.read();
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
            w_wb_memop = 1;
            w_queue_re = 1;
            if (queue_nempty.read() == 1) {
                w_mem_valid = 1;
                v.memop_res_pc = wb_e_pc;
                v.memop_res_instr = wb_e_instr;
                v.memop_res_addr = wb_res_addr;
                v.memop_res_data = vb_res_data;
                if (wb_res_addr == 0) {
                    v.memop_res_wena = 0;
                } else {
                    v.memop_res_wena = 1;
                }
                v.memop_addr = wb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_r = !w_mem_write;
                v.memop_sign_ext = w_mem_sign_ext;
                v.memop_size = wb_mem_sz;

                if (i_mem_req_ready.read() == 1) {
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

    vb_mem_resp_shifted = 0;
    vb_mem_data_unsigned = 0;
    vb_mem_data_signed = 0;
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
        vb_mem_data_unsigned = vb_mem_resp_shifted(7, 0);
        vb_mem_data_signed = vb_mem_resp_shifted(7, 0);
        if (vb_mem_resp_shifted[7]) {
            vb_mem_data_signed(63, 8) = ~0;
        }
        break;
    case MEMOP_2B:
        vb_mem_data_unsigned = vb_mem_resp_shifted(15, 0);
        vb_mem_data_signed = vb_mem_resp_shifted(15, 0);
        if (vb_mem_resp_shifted[15]) {
            vb_mem_data_signed(63, 16) = ~0;
        }
        break;
    case MEMOP_4B:
        vb_mem_data_unsigned = vb_mem_resp_shifted(31, 0);
        vb_mem_data_signed = vb_mem_resp_shifted(31, 0);
        if (i_mem_data.read()[31]) {
            vb_mem_data_signed(63, 32) = ~0;
        }
        break;
    default:
        vb_mem_data_unsigned = vb_mem_resp_shifted;
        vb_mem_data_signed = vb_mem_resp_shifted;
    }

    if (r.memop_r.read() == 1) {
        if (r.memop_sign_ext.read() == 1) {
            vb_mem_rdata = vb_mem_data_signed;
        } else {
            vb_mem_rdata = vb_mem_data_unsigned;
        }
    } else {
        vb_mem_rdata = r.memop_res_data;
    }

    if ((i_e_valid.read() == 1 && (i_memop_load | i_memop_store) == 0)) {
        v.reg_wb_valid = 1;
        v.reg_res_pc = i_e_pc;
        v.reg_res_instr = i_e_instr;
        v.reg_res_addr = i_res_addr;
        v.reg_res_data = i_res_data;
        if (i_res_addr.read() == 0) {
            v.reg_res_wena = 0;
        } else {
            v.reg_res_wena = 1;
        }
    } else if (w_wb_memop == 0) {
        v.reg_wb_valid = 0;
        v.reg_res_pc = 0;
        v.reg_res_instr = 0;
        v.reg_res_addr = 0;
        v.reg_res_data = 0;
        v.reg_res_wena = 0;
    }


    // Select memop or register writeback channel. If both inject hold state
    if (r.reg_wb_valid.read() == 1 && w_wb_memop == 1) {
        w_hold = 1;
    }

    bool w_memop_ready;
    bool v_o_wena;
    sc_uint<6> vb_o_waddr;
    sc_uint<RISCV_ARCH> vb_o_wdata;
    sc_uint<BUS_ADDR_WIDTH> vb_o_pc;
    sc_uint<32> vb_o_instr;

    w_memop_ready = 1;
    if (queue_full.read() == 1) {
        w_memop_ready = 0;
    }

    if (w_wb_memop == 1) {
        v_o_wena = r.memop_res_wena.read();
        vb_o_waddr = r.memop_res_addr.read();
        vb_o_wdata = vb_mem_rdata;
        vb_o_pc = r.memop_res_pc.read();
        vb_o_instr = r.memop_res_instr.read();
    } else {
        v_o_wena = r.reg_res_wena.read();
        vb_o_waddr = r.reg_res_addr.read();
        vb_o_wdata = r.reg_res_data.read();
        vb_o_pc = r.reg_res_pc.read();
        vb_o_instr = r.reg_res_instr.read();
    }




    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    queue_re = w_queue_re;

    o_mem_resp_ready = 1;

    o_mem_valid = w_mem_valid;
    o_mem_write = w_mem_write;
    o_mem_addr = wb_mem_addr & ~LOG2_DATA_BYTES_MASK;
    o_mem_wdata = vb_mem_wdata;
    o_mem_wstrb = vb_mem_wstrb;

    o_hold = w_hold;
    o_memop_ready = w_memop_ready;
    o_wena = v_o_wena;
    o_waddr = vb_o_waddr;
    o_wdata = vb_o_wdata;
    /** the following signal used to executation instruction count and debug */
    o_valid = w_wb_memop || r.reg_wb_valid.read();
    o_pc = vb_o_pc;
    o_instr = vb_o_instr;
    o_wb_memop = w_wb_memop;
}

void MemAccess::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

