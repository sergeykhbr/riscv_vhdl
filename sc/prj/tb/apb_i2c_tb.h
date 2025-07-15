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
#include "../../rtl/internal/ambalib/types_amba.h"
#include "../../rtl/internal/ambalib/types_pnp.h"
#include "../../rtl/sim/pll/pll_generic.h"
#include "../common/vips/i2c/vip_i2c_s.h"
#include "../../rtl/internal/misclib/apb_i2c.h"

namespace debugger {

SC_MODULE(apb_i2c_tb) {
 public:

    void comb();
    void test();

    apb_i2c_tb(sc_module_name name);
    virtual ~apb_i2c_tb();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<bool> i_nrst;                                 // Power-on system reset active LOW
    sc_signal<bool> i_clk;
    sc_signal<mapinfo_type> wb_i_mapinfo;
    sc_signal<dev_config_type> wb_o_cfg;
    sc_signal<apb_in_type> wb_i_apbi;
    sc_signal<apb_out_type> wb_o_apbo;
    sc_signal<bool> w_o_scl;
    sc_signal<bool> w_o_sda;
    sc_signal<bool> w_o_sda_dir;
    sc_signal<bool> w_i_sda;
    sc_signal<bool> w_o_irq;
    sc_signal<bool> w_o_nreset;
    sc_signal<bool> w_hdmi_sda_dir;
    sc_uint<32> wb_clk_cnt;

    pll_generic *clk0;
    vip_i2c_s *hdmi;
    apb_i2c *tt;

};

}  // namespace debugger

