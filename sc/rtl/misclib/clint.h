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
#pragma once

#include <systemc.h>
#include "../ambalib/types_amba.h"
#include "../ambalib/types_pnp.h"
#include "../ambalib/axi_slv.h"
#include "api_core.h"

namespace debugger {

template<int cpu_total = 4>
SC_MODULE(clint) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI Slave to Bridge interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI Bridge to Slave interface
    sc_out<sc_uint<64>> o_mtimer;                           // Shadow read-only access from Harts
    sc_out<sc_uint<cpu_total>> o_msip;                      // Machine mode Softare Pending Interrupt
    sc_out<sc_uint<cpu_total>> o_mtip;                      // Machine mode Timer Pending Interrupt

    void comb();
    void registers();

    SC_HAS_PROCESS(clint);

    clint(sc_module_name name,
          bool async_reset);
    virtual ~clint();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct clint_cpu_type {
        sc_signal<bool> msip;
        sc_signal<bool> mtip;
        sc_signal<sc_uint<64>> mtimecmp;
    };


    struct clint_registers {
        sc_signal<sc_uint<64>> mtime;
        clint_cpu_type hart[cpu_total];
        sc_signal<sc_uint<64>> rdata;
    } v, r;

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> wb_req_addr;
    sc_signal<sc_uint<8>> wb_req_size;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_req_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_req_wstrb;
    sc_signal<bool> w_req_last;
    sc_signal<bool> w_req_ready;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_resp_rdata;
    sc_signal<bool> wb_resp_err;

    axi_slv *xslv0;

};

template<int cpu_total>
clint<cpu_total>::clint(sc_module_name name,
                        bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    o_mtimer("o_mtimer"),
    o_msip("o_msip"),
    o_mtip("o_mtip") {

    async_reset_ = async_reset;
    xslv0 = 0;

    xslv0 = new axi_slv("xslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_CLINT);
    xslv0->i_clk(i_clk);
    xslv0->i_nrst(i_nrst);
    xslv0->i_mapinfo(i_mapinfo);
    xslv0->o_cfg(o_cfg);
    xslv0->i_xslvi(i_xslvi);
    xslv0->o_xslvo(o_xslvo);
    xslv0->o_req_valid(w_req_valid);
    xslv0->o_req_addr(wb_req_addr);
    xslv0->o_req_size(wb_req_size);
    xslv0->o_req_write(w_req_write);
    xslv0->o_req_wdata(wb_req_wdata);
    xslv0->o_req_wstrb(wb_req_wstrb);
    xslv0->o_req_last(w_req_last);
    xslv0->i_req_ready(w_req_ready);
    xslv0->i_resp_valid(w_resp_valid);
    xslv0->i_resp_rdata(wb_resp_rdata);
    xslv0->i_resp_err(wb_resp_err);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_xslvi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << wb_req_size;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << wb_req_wstrb;
    sensitive << w_req_last;
    sensitive << w_req_ready;
    sensitive << w_resp_valid;
    sensitive << wb_resp_rdata;
    sensitive << wb_resp_err;
    sensitive << r.mtime;
    for (int i = 0; i < cpu_total; i++) {
        sensitive << r.hart[i].msip;
        sensitive << r.hart[i].mtip;
        sensitive << r.hart[i].mtimecmp;
    }
    sensitive << r.rdata;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int cpu_total>
clint<cpu_total>::~clint() {
    if (xslv0) {
        delete xslv0;
    }
}

template<int cpu_total>
void clint<cpu_total>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, o_mtimer, o_mtimer.name());
        sc_trace(o_vcd, o_msip, o_msip.name());
        sc_trace(o_vcd, o_mtip, o_mtip.name());
        sc_trace(o_vcd, r.mtime, pn + ".r_mtime");
        for (int i = 0; i < cpu_total; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_hart%d_msip", pn.c_str(), i);
            sc_trace(o_vcd, r.hart[i].msip, tstr);
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_hart%d_mtip", pn.c_str(), i);
            sc_trace(o_vcd, r.hart[i].mtip, tstr);
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_hart%d_mtimecmp", pn.c_str(), i);
            sc_trace(o_vcd, r.hart[i].mtimecmp, tstr);
        }
        sc_trace(o_vcd, r.rdata, pn + ".r_rdata");
    }

    if (xslv0) {
        xslv0->generateVCD(i_vcd, o_vcd);
    }
}

template<int cpu_total>
void clint<cpu_total>::comb() {
    sc_uint<CFG_SYSBUS_DATA_BITS> vrdata;
    sc_uint<cpu_total> vb_msip;
    sc_uint<cpu_total> vb_mtip;
    int regidx;

    vrdata = 0;
    vb_msip = 0;
    vb_mtip = 0;
    regidx = 0;

    v.mtime = r.mtime;
    for (int i = 0; i < cpu_total; i++) {
        v.hart[i].msip = r.hart[i].msip;
        v.hart[i].mtip = r.hart[i].mtip;
        v.hart[i].mtimecmp = r.hart[i].mtimecmp;
    }
    v.rdata = r.rdata;

    v.mtime = (r.mtime.read() + 1);
    regidx = wb_req_addr.read()(13, 3).to_int();

    for (int i = 0; i < cpu_total; i++) {
        v.hart[i].mtip = 0;
        if (r.mtime.read() >= r.hart[i].mtimecmp) {
            v.hart[i].mtip = 1;
        }
    }

    switch (wb_req_addr.read()(15, 14)) {
    case 0:
        vrdata[0] = r.hart[regidx].msip;
        vrdata[32] = r.hart[(regidx + 1)].msip;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            if (wb_req_wstrb.read()(3, 0).or_reduce() == 1) {
                v.hart[regidx].msip = wb_req_wdata.read()[0];
            }
            if (wb_req_wstrb.read()(7, 4).or_reduce() == 1) {
                v.hart[(regidx + 1)].msip = wb_req_wdata.read()[32];
            }
        }
        break;
    case 1:
        vrdata = r.hart[regidx].mtimecmp;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.hart[regidx].mtimecmp = wb_req_wdata;
        }
        break;
    case 2:
        if (wb_req_addr.read()(13, 3) == 0x7FF) {
            vrdata = r.mtime;                               // [RO]
        }
        break;
    default:
        break;
    }
    v.rdata = vrdata;

    if (!async_reset_ && i_nrst.read() == 0) {
        v.mtime = 0;
        for (int i = 0; i < cpu_total; i++) {
            v.hart[i].msip = 0;
            v.hart[i].mtip = 0;
            v.hart[i].mtimecmp = 0;
        }
        v.rdata = 0;
    }

    for (int i = 0; i < cpu_total; i++) {
        vb_msip[i] = r.hart[i].msip;
        vb_mtip[i] = r.hart[i].mtip;
    }

    w_req_ready = 1;
    w_resp_valid = 1;
    wb_resp_rdata = r.rdata;
    wb_resp_err = 0;
    o_msip = vb_msip;
    o_mtip = vb_mtip;
    o_mtimer = r.mtime;
}

template<int cpu_total>
void clint<cpu_total>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.mtime = 0;
        for (int i = 0; i < cpu_total; i++) {
            r.hart[i].msip = 0;
            r.hart[i].mtip = 0;
            r.hart[i].mtimecmp = 0;
        }
        r.rdata = 0;
    } else {
        r.mtime = v.mtime;
        for (int i = 0; i < cpu_total; i++) {
            r.hart[i].msip = v.hart[i].msip;
            r.hart[i].mtip = v.hart[i].mtip;
            r.hart[i].mtimecmp = v.hart[i].mtimecmp;
        }
        r.rdata = v.rdata;
    }
}

}  // namespace debugger

