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
#include "../../internal/ambalib/types_amba.h"
#include "../../internal/ambalib/types_pnp.h"
#include "../../sim/pll/pll_generic.h"
#include "../../internal/misclib/apb_ddr.h"
#include "../../internal/misclib/axi_sram.h"

namespace debugger {

SC_MODULE(ddr3_tech) {
 public:
    sc_in<bool> i_apb_nrst;
    sc_in<bool> i_apb_clk;
    sc_in<bool> i_xslv_nrst;
    sc_in<bool> i_xslv_clk;
    // AXI memory access (ddr clock)
    sc_in<mapinfo_type> i_xmapinfo;
    sc_out<dev_config_type> o_xcfg;
    sc_in<axi4_slave_in_type> i_xslvi;
    sc_out<axi4_slave_out_type> o_xslvo;
    // APB control interface (sys clock):
    sc_in<mapinfo_type> i_pmapinfo;
    sc_out<dev_config_type> o_pcfg;
    sc_in<apb_in_type> i_apbi;
    sc_out<apb_out_type> o_apbo;
    // to debug PIN:
    sc_out<bool> o_ui_nrst;                                 // xilinx generte ddr clock inside ddr controller
    sc_out<bool> o_ui_clk;                                  // xilinx generte ddr clock inside ddr controller
    sc_out<bool> o_init_calib_done;

    void init();
    void comb();
    void registers();

    ddr3_tech(sc_module_name name);
    virtual ~ddr3_tech();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct ddr3_tech_registers {
        sc_signal<sc_uint<8>> ddr_calib;
    };

    void ddr3_tech_r_reset(ddr3_tech_registers& iv) {
        iv.ddr_calib = 0;
    }

    sc_signal<bool> w_ui_nrst;
    sc_signal<bool> w_ui_clk;
    sc_signal<bool> w_init_calib_done;
    sc_signal<sc_uint<12>> wb_device_temp;
    sc_signal<bool> w_sr_active;
    sc_signal<bool> w_ref_ack;
    sc_signal<bool> w_zq_ack;
    sc_signal<dev_config_type> wb_xcfg_unused;
    ddr3_tech_registers v;
    ddr3_tech_registers r;

    pll_generic *clk0;
    apb_ddr *pctrl0;
    axi_sram<12> *sram0;

};

}  // namespace debugger

