/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CSR registers module.
 */

#include "csr.h"
#include "riscv-isa.h"

namespace debugger {

CsrRegs::CsrRegs(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xret;
    sensitive << i_addr;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << r.mode;
    sensitive << i_trap_ena;
    sensitive << i_trap_code;
    sensitive << i_trap_pc;
    sensitive << r.mode;
    sensitive << r.mie;


    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_xret, "/top/proc0/csr0/i_xret");
        sc_trace(vcd, o_mode, "/top/proc0/csr0/o_mode");
        sc_trace(vcd, o_ie, "/top/proc0/csr0/o_ie");
        sc_trace(vcd, o_rdata, "/top/proc0/csr0/o_rdata");
    }
};


void CsrRegs::comb() {
    sc_uint<RISCV_ARCH> wb_rdata = 0;
    bool w_ie;

    v = r;

    switch (i_addr.read()) {
    case CSR_misa:
        break;
    case CSR_mvendorid:
        break;
    case CSR_marchid:
        break;
    case CSR_mimplementationid:
        break;
    case CSR_mhartid:
        break;
    case CSR_uepc:// - User mode program counter
        break;
    case CSR_mstatus:// - Machine mode status register
        wb_rdata = 0;
        wb_rdata[0] = r.uie;
        wb_rdata[3] = r.mie;
        wb_rdata[7] = r.mpie;
        wb_rdata(12, 11) = r.mpp;
        if (i_wena.read()) {
            v.uie = i_wdata.read()[0];
            v.mie = i_wdata.read()[3];
            v.mpie = i_wdata.read()[7];
            v.mpp = i_wdata.read()(12, 11);
        }
        break;
    case CSR_medeleg:// - Machine exception delegation
        break;
    case CSR_mideleg:// - Machine itnerrupt delegation
        break;
    case CSR_mie:// - Machine interrupt enable bit
        break;
    case CSR_mtvec:
        wb_rdata = r.mtvec;
        if (i_wena.read()) {
            v.mtvec = i_wdata;
        }
        break;
    case CSR_mtimecmp:// - Machine wall-clock timer compare value
        break;
    case CSR_mscratch:// - Machine scratch register
        wb_rdata = r.mscratch;
        if (i_wena.read()) {
            v.mscratch = i_wdata;
        }
        break;
    case CSR_mepc:// - Machine program counter
        wb_rdata = r.mepc;
        if (i_xret.read()) {
            // Switch to previous mode
            v.mie = r.mpie;
            v.mpie = 1;
            v.mode = r.mpp;
            v.mpp = PRV_U;
        }
        if (i_wena.read()) {
            v.mepc = i_wdata;
        }
        break;
    case CSR_mcause:// - Machine trap cause
        wb_rdata = 0;
        wb_rdata[63] = r.trap_irq;
        wb_rdata(3, 0) = r.trap_code;
        break;
    case CSR_mbadaddr:// - Machine bad address
        wb_rdata = r.mbadaddr;
        break;
    case CSR_mip:// - Machine interrupt pending
        break;
    default:;
    }

    if (i_trap_ena.read()) {
        v.mie = 0;
        v.mpp = r.mode;
        v.mepc = i_trap_pc.read();
        v.trap_code = i_trap_code.read()(3, 0);
        v.trap_irq = i_trap_code.read()[4];
        v.mode = PRV_M;
        switch (r.mode.read()) {
        case PRV_U:
            v.mpie = r.uie;
            break;
        case PRV_M:
            v.mpie = r.mie;
            break;
        default:;
        }
    }

    w_ie = 0;
    if ((r.mode.read() != PRV_M) || r.mie.read()) {
        w_ie = 1;
    }

    if (!i_nrst.read()) {
        v.mtvec = 0;
        v.mscratch = 0;
        v.mbadaddr = 0;
        v.mode = PRV_M;
        v.uie = 0;
        v.mie = 0;
        v.mpie = 0;
        v.mpp = 0;
        v.mepc = 0;
        v.trap_code = 0;
        v.trap_irq = 0;
    }


    o_rdata = wb_rdata;
    o_ie = w_ie;
    o_mode = r.mode;
    o_mtvec = r.mtvec.read()(BUS_ADDR_WIDTH - 1, 0);
}

void CsrRegs::registers() {
    r = v;
}

}  // namespace debugger

