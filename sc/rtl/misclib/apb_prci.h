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
#include "../ambalib/apb_slv.h"

namespace debugger {

SC_MODULE(apb_prci) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_pwrreset;                                 // Power-on reset, external button. Active HIGH
    sc_in<bool> i_dmireset;                                 // Debug reset: system reset except DMI interface
    sc_in<bool> i_sys_locked;
    sc_in<bool> i_ddr_locked;
    sc_out<bool> o_sys_rst;                                 // System reset except DMI. Active HIGH
    sc_out<bool> o_sys_nrst;                                // System reset except DMI. Active LOW
    sc_out<bool> o_dbg_nrst;                                // Reset DMI. Active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB  Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface

    void comb();
    void registers();

    SC_HAS_PROCESS(apb_prci);

    apb_prci(sc_module_name name,
             bool async_reset);
    virtual ~apb_prci();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct apb_prci_registers {
        sc_signal<bool> sys_rst;
        sc_signal<bool> sys_nrst;
        sc_signal<bool> dbg_nrst;
        sc_signal<bool> sys_locked;
        sc_signal<bool> ddr_locked;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    void apb_prci_r_reset(apb_prci_registers &iv) {
        iv.sys_rst = 0;
        iv.sys_nrst = 0;
        iv.dbg_nrst = 0;
        iv.sys_locked = 0;
        iv.ddr_locked = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;

    apb_slv *pslv0;

};

}  // namespace debugger

