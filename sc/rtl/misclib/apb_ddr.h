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

SC_MODULE(apb_ddr) {
 public:
    sc_in<bool> i_clk;                                      // APB clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB input interface
    sc_out<apb_out_type> o_apbo;                            // APB output interface
    sc_in<bool> i_pll_locked;                               // PLL locked
    sc_in<bool> i_init_calib_done;                          // DDR initialization done
    sc_in<sc_uint<12>> i_device_temp;                       // Temperature monitor value
    sc_in<bool> i_sr_active;
    sc_in<bool> i_ref_ack;
    sc_in<bool> i_zq_ack;

    void comb();
    void registers();

    SC_HAS_PROCESS(apb_ddr);

    apb_ddr(sc_module_name name,
            bool async_reset);
    virtual ~apb_ddr();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct apb_ddr_registers {
        sc_signal<bool> pll_locked;
        sc_signal<bool> init_calib_done;
        sc_signal<sc_uint<12>> device_temp;
        sc_signal<bool> sr_active;
        sc_signal<bool> ref_ack;
        sc_signal<bool> zq_ack;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    void apb_ddr_r_reset(apb_ddr_registers &iv) {
        iv.pll_locked = 0;
        iv.init_calib_done = 0;
        iv.device_temp = 0;
        iv.sr_active = 0;
        iv.ref_ack = 0;
        iv.zq_ack = 0;
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

