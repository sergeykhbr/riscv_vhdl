/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug port.
 * @details    Must be connected to DSU.
 */

#include "dbg_port.h"

namespace debugger {

DbgPort::DbgPort(sc_module_name name_, sc_trace_file *vcd)
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dsu_valid;
    sensitive << i_dsu_write;
    sensitive << i_dsu_addr;
    sensitive << i_dsu_wdata;
    sensitive << i_ireg_rdata;
    sensitive << i_csr_rdata;
    sensitive << r.bank_idx;
    sensitive << r.halt;
    sensitive << r.step_cnt;
    sensitive << r.timer;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        //sc_trace(vcd, i_hold, "/top/proc0/bp0/i_hold");
    }
};


void DbgPort::comb() {
    v = r;

    v.timer = r.timer.read() + 1;

    if (!i_nrst.read()) {
        v.bank_idx = 0;
        v.halt = 0;
        v.timer = 0;
        v.step_cnt = 0;
    }

    o_halt = r.halt;
}

void DbgPort::registers() {
    r = v;
}

}  // namespace debugger

