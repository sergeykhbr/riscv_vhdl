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
    sc_out<sc_uint<16>> o_watchdog;                         // Number of sclk to detect no response
    sc_out<bool> o_clear_cmderr;                            // Clear cmderr from FW
    // Debug command state machine
    sc_in<sc_uint<4>> i_cmd_state;
    sc_in<sc_uint<4>> i_cmd_err;
    sc_in<bool> i_cmd_req_valid;
    sc_in<sc_uint<6>> i_cmd_req_cmd;
    sc_in<bool> i_cmd_resp_valid;
    sc_in<sc_uint<6>> i_cmd_resp_cmd;
    sc_in<sc_uint<32>> i_cmd_resp_reg;
    sc_in<sc_uint<7>> i_cmd_resp_crc7_rx;
    sc_in<sc_uint<7>> i_cmd_resp_crc7_calc;

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
        sc_signal<bool> sclk_ena;
        sc_signal<bool> clear_cmderr;
        sc_signal<sc_uint<32>> scaler;
        sc_signal<sc_uint<32>> scaler_cnt;
        sc_signal<sc_uint<16>> wdog;
        sc_signal<sc_uint<16>> wdog_cnt;
        sc_signal<bool> level;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
        sc_signal<sc_uint<6>> last_req_cmd;
        sc_signal<sc_uint<6>> last_resp_cmd;
        sc_signal<sc_uint<7>> last_resp_crc7_rx;
        sc_signal<sc_uint<7>> last_resp_crc7_calc;
        sc_signal<sc_uint<32>> last_resp_reg;
    } v, r;

    void sdctrl_regs_r_reset(sdctrl_regs_registers &iv) {
        iv.sclk_ena = 0;
        iv.clear_cmderr = 0;
        iv.scaler = 0;
        iv.scaler_cnt = 0;
        iv.wdog = 0x0FFF;
        iv.wdog_cnt = 0;
        iv.level = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
        iv.last_req_cmd = ~0ul;
        iv.last_resp_cmd = 0;
        iv.last_resp_crc7_rx = 0;
        iv.last_resp_crc7_calc = 0;
        iv.last_resp_reg = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;

    apb_slv *pslv0;

};

}  // namespace debugger

