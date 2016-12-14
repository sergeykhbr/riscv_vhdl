/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug port.
 * @details    Must be connected to DSU.
 */

#include "dbg_port.h"

namespace debugger {

DbgPort::DbgPort(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dport_valid;
    sensitive << i_dport_write;
    sensitive << i_dport_region;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_ireg_rdata;
    sensitive << i_csr_rdata;
    sensitive << i_pc;
    sensitive << i_npc;
    sensitive << i_e_valid;
    sensitive << i_m_valid;
    sensitive << r.ready;
    sensitive << r.rdata;
    sensitive << r.halt;
    sensitive << r.stepping_mode;
    sensitive << r.clock_cnt;
    sensitive << r.executed_cnt;
    sensitive << r.stepping_mode_cnt;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void DbgPort::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_dport_valid, "/top/proc0/dbg0/i_dport_valid");
        sc_trace(o_vcd, i_dport_write, "/top/proc0/dbg0/i_dport_write");
        sc_trace(o_vcd, i_dport_region, "/top/proc0/dbg0/i_dport_region");
        sc_trace(o_vcd, i_dport_addr, "/top/proc0/dbg0/i_dport_addr");
        sc_trace(o_vcd, i_dport_wdata, "/top/proc0/dbg0/i_dport_wdata");
        sc_trace(o_vcd, o_dport_ready, "/top/proc0/dbg0/o_dport_ready");
        sc_trace(o_vcd, o_dport_rdata, "/top/proc0/dbg0/o_dport_rdata");
        sc_trace(o_vcd, i_e_valid, "/top/proc0/dbg0/i_e_valid");
        sc_trace(o_vcd, i_m_valid, "/top/proc0/dbg0/i_m_valid");

        sc_trace(o_vcd, o_halt, "/top/proc0/dbg0/o_halt");
        sc_trace(o_vcd, o_core_addr, "/top/proc0/dbg0/o_core_addr");
        sc_trace(o_vcd, o_core_wdata, "/top/proc0/dbg0/o_core_wdata");
        sc_trace(o_vcd, o_ireg_ena, "/top/proc0/dbg0/o_ireg_ena");
        sc_trace(o_vcd, o_npc_write, "/top/proc0/dbg0/o_npc_write");
        sc_trace(o_vcd, o_ireg_write, "/top/proc0/dbg0/o_ireg_write");
        sc_trace(o_vcd, r.clock_cnt, "/top/proc0/dbg0/r_clock_cnt");
        sc_trace(o_vcd, r.stepping_mode_cnt, "/top/proc0/dbg0/r_stepping_mode_cnt");
        sc_trace(o_vcd, r.stepping_mode, "/top/proc0/dbg0/r_stepping_mode");
    }
}

void DbgPort::comb() {
    sc_uint<12> wb_o_core_addr;
    sc_uint<RISCV_ARCH> wb_o_core_wdata;
    sc_uint<64> wb_rdata;
    sc_uint<12> wb_idx;
    bool w_o_csr_ena;
    bool w_o_csr_write;
    bool w_o_ireg_ena;
    bool w_o_ireg_write;
    bool w_o_npc_write;
    bool w_cur_halt;


    v = r;

    wb_o_core_addr = 0;
    wb_o_core_wdata = 0;
    wb_rdata = 0;
    wb_idx = i_dport_addr.read();
    w_o_csr_ena = 0;
    w_o_csr_write = 0;
    w_o_ireg_ena = 0;
    w_o_ireg_write = 0;
    w_o_npc_write = 0;

    v.ready = i_dport_valid.read();

    w_cur_halt = 0;
    if (i_e_valid.read()) {
        if (r.stepping_mode_cnt.read() != 0) {
            v.stepping_mode_cnt = r.stepping_mode_cnt.read() - 1;
            if (r.stepping_mode_cnt.read() == 1) {
                v.halt = 1;
                w_cur_halt = 1;
                v.stepping_mode = 0;
            }
        }
    }

    if (r.halt == 0) {
        v.clock_cnt = r.clock_cnt.read() + 1;
    }
    if (i_m_valid.read()) {
        v.executed_cnt = r.executed_cnt.read() + 1;
    }

    if (i_dport_valid.read()) {
        switch (i_dport_region.read()) {
        case 0:
            w_o_csr_ena = 1;
            wb_o_core_addr = i_dport_addr;
            wb_rdata = i_csr_rdata;
            if (i_dport_write.read()) {
                w_o_csr_write = 1;
                wb_o_core_wdata = i_dport_wdata;
            }
            break;
        case 1:
            if (wb_idx < 32) {
                w_o_ireg_ena = 1;
                wb_o_core_addr = i_dport_addr;
                wb_rdata = i_ireg_rdata;
                if (i_dport_write.read()) {
                    w_o_ireg_write = 1;
                    wb_o_core_wdata = i_dport_wdata;
                }
            } else if (wb_idx == 32) {
                /** Read only register */
                wb_rdata = i_pc;
            } else if (wb_idx == 33) {
                wb_rdata = i_npc;
                if (i_dport_write.read()) {
                    w_o_npc_write = 1;
                    wb_o_core_wdata = i_dport_wdata;
                }
            }
            break;
        case 2:
            switch (wb_idx) {
            case 0:
                wb_rdata[0] = r.halt;
                if (i_dport_write.read()) {
                    v.halt = i_dport_wdata.read()[0];
                    v.stepping_mode = i_dport_wdata.read()[1];
                    if (i_dport_wdata.read()[1]) {
                        v.stepping_mode_cnt = r.stepping_mode_steps;
                    }
                }
                break;
            case 1:
                wb_rdata = r.stepping_mode_steps;
                if (i_dport_write.read()) {
                    v.stepping_mode_steps = i_dport_wdata.read();
                }
                break;
            case 2:
                wb_rdata = r.clock_cnt;
                break;
            case 3:
                wb_rdata = r.executed_cnt;
                break;
            case 4:
                // todo: add hardware breakpoint
                break;
            case 5:
                // todo: remove hardware breakpoint
                break;
            default:;
            }
            break;
        default:;
        }
    }
    v.rdata = wb_rdata;

    if (!i_nrst.read()) {
        v.ready = 0;
        v.halt = 0;
        v.stepping_mode = 0;
        v.rdata = 0;
        v.stepping_mode_cnt = 0;
        v.stepping_mode_steps = 0;
        v.clock_cnt = 0;
        v.executed_cnt = 0;
    }

    o_core_addr = wb_o_core_addr;
    o_core_wdata = wb_o_core_wdata;
    o_csr_ena = w_o_csr_ena;
    o_csr_write = w_o_csr_write;
    o_ireg_ena = w_o_ireg_ena;
    o_ireg_write = w_o_ireg_write;
    o_npc_write = w_o_npc_write;
    o_clock_cnt = r.clock_cnt;
    o_executed_cnt = r.executed_cnt;
    o_halt = r.halt | w_cur_halt;

    o_dport_ready = r.ready;
    o_dport_rdata = r.rdata;
}

void DbgPort::registers() {
    r = v;
}

}  // namespace debugger

