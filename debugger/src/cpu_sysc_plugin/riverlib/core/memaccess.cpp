/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Memory Access stage.
 */

#include "memaccess.h"

namespace debugger {

MemAccess::MemAccess(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_e_valid;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_res_addr;
    sensitive << i_res_data;
    sensitive << i_memop_load;
    sensitive << i_memop_store;
    sensitive << i_memop_size;
    sensitive << i_memop_addr;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
    sensitive << r.valid;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_e_valid, "/top/proc0/mem0/i_e_valid");
        sc_trace(vcd, i_e_pc, "/top/proc0/mem0/i_e_pc");
        sc_trace(vcd, i_e_instr, "/top/proc0/mem0/i_e_instr");
        sc_trace(vcd, o_mem_valid, "/top/proc0/mem0/o_mem_valid");
        sc_trace(vcd, o_mem_write, "/top/proc0/mem0/o_mem_write");
        sc_trace(vcd, o_mem_sz, "/top/proc0/mem0/o_mem_sz");
        sc_trace(vcd, o_mem_addr, "/top/proc0/mem0/o_mem_addr");
        sc_trace(vcd, o_mem_data, "/top/proc0/mem0/o_mem_data");
        sc_trace(vcd, i_mem_data_valid, "/top/proc0/mem0/i_mem_data_valid");
        sc_trace(vcd, i_mem_data_addr, "/top/proc0/mem0/i_mem_data_addr");
        sc_trace(vcd, i_mem_data, "/top/proc0/mem0/i_mem_data");

        sc_trace(vcd, o_valid, "/top/proc0/mem0/o_valid");
        sc_trace(vcd, o_pc, "/top/proc0/mem0/o_pc");
        sc_trace(vcd, o_instr, "/top/proc0/mem0/o_instr");
        sc_trace(vcd, o_wena, "/top/proc0/mem0/o_wena");
        sc_trace(vcd, o_waddr, "/top/proc0/mem0/o_waddr");
        sc_trace(vcd, o_wdata, "/top/proc0/mem0/o_wdata");
    }
};


void MemAccess::comb() {
    v = r;

    w_mem_valid = 0;
    w_mem_write = 0;
    wb_mem_sz = 0;
    wb_mem_addr = 0;
    wb_mem_wdata = 0;

    bool is_waiting = r.wait_resp.read() & !i_mem_data_valid.read();
    v.wait_resp = is_waiting;


    v.pc = i_e_pc;
    v.instr = i_e_instr;
    v.valid = i_e_valid;
    if (i_e_valid.read()) {
        v.valid = true;
        v.waddr = i_res_addr;
        v.wdata = i_res_data;
        v.wena = i_res_addr.read().or_reduce(); // Write if none zero

        if (i_memop_store.read() || i_memop_load.read()) {
            v.wait_resp = 1;
            w_mem_valid = 1;
            w_mem_write = i_memop_store;
            wb_mem_sz = i_memop_size;
            wb_mem_addr = i_memop_addr;
            wb_mem_wdata = i_res_data;
        } else {
            v.wait_resp = 0;
        }
    }

    if (r.wait_resp.read() & i_mem_data_valid.read()) {
        wb_res_wdata = i_mem_data;
    } else {
        wb_res_wdata = r.wdata;
    }

    if (!i_nrst.read()) {
        v.valid = false;
        v.pc = 0;
        v.instr = 0;
        v.waddr = 0;
        v.wdata = 0;
        v.wena = 0;
        v.wait_resp = 0;
    }

    o_mem_valid = w_mem_valid;
    o_mem_write = w_mem_write;
    o_mem_sz = wb_mem_sz;
    o_mem_addr = wb_mem_addr;
    o_mem_data = wb_mem_wdata;

    o_wena = r.wena;
    o_waddr = r.waddr;
    o_wdata = wb_res_wdata;
    o_valid = r.valid.read() & !is_waiting;
    o_pc = r.pc;
    o_instr = r.instr;
}

void MemAccess::registers() {
    r = v;
}

}  // namespace debugger

