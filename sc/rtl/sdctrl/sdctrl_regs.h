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
#include "../ambalib/apb_slv.h"

namespace debugger {

SC_MODULE(sdctrl_regs) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_pmapinfo;                         // APB interconnect slot information
    sc_out<dev_config_type> o_pcfg;                         // APB sd-controller configuration registers descriptor
    sc_in<apb_in_type> i_apbi;                              // APB Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_sck;                                     // SD-card clock usually upto 50 MHz
    sc_out<bool> o_sck_posedge;                             // Strob just before positive edge
    sc_out<bool> o_sck_negedge;                             // Strob just before negative edge

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_regs);

    sdctrl_regs(sc_module_name name,
                bool async_reset);
    virtual ~sdctrl_regs();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct sdctrl_regs_registers {
        sc_signal<sc_uint<32>> scaler;
        sc_signal<sc_uint<32>> scaler_cnt;
        sc_signal<sc_uint<16>> wdog;
        sc_signal<sc_uint<16>> wdog_cnt;
        sc_signal<bool> level;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    void sdctrl_regs_r_reset(sdctrl_regs_registers &iv) {
        iv.scaler = 0;
        iv.scaler_cnt = 0;
        iv.wdog = 0;
        iv.wdog_cnt = 0;
        iv.level = 1;
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

