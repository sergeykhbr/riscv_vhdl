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
#include "../../rtl/internal/ambalib/axi_slv.h"
#include "../../rtl/internal/cdc/afifo_xslv.h"

namespace debugger {

SC_MODULE(afifo_xslv_tb) {
 public:

    void comb();
    void test_clk1();
    void test_clk2();

    afifo_xslv_tb(sc_module_name name);
    virtual ~afifo_xslv_tb();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<bool> i_nrst;                                 // Power-on system reset active LOW
    sc_signal<bool> w_clk1;
    sc_signal<bool> w_clk2;
    sc_uint<32> wb_clk1_cnt;
    sc_uint<32> wb_clk2_cnt;
    sc_signal<axi4_slave_in_type> wb_clk1_xslvi;            // Clock 1 input
    sc_signal<axi4_slave_out_type> wb_clk1_xslvo;           // Clock 1 output
    sc_signal<axi4_slave_in_type> wb_clk2_xmsto;            // Clock 2 output
    sc_signal<axi4_slave_out_type> wb_clk2_xmsti;           // Clock 2 input
    sc_signal<mapinfo_type> wb_slv_i_mapinfo;               // Base address information from the interconnect port
    sc_signal<dev_config_type> wb_slv_o_cfg;                // Slave config descriptor
    sc_signal<bool> w_slv_o_req_valid;
    sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> wb_slv_o_req_addr;
    sc_signal<sc_uint<8>> wb_slv_o_req_size;
    sc_signal<bool> w_slv_o_req_write;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_slv_o_req_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_slv_o_req_wstrb;
    sc_signal<bool> w_slv_o_req_last;
    sc_signal<bool> w_slv_i_req_ready;
    sc_signal<bool> w_slv_i_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_slv_i_resp_rdata;
    sc_signal<bool> w_slv_i_resp_err;
    bool v_busy;
    sc_uint<3> rd_valid;
    bool req_ready;
    sc_uint<4> rd_addr;
    sc_uint<64> rd_data;
    sc_uint<64> mem[16];

    pll_generic *clk1;
    pll_generic *clk2;
    axi_slv *slv0;
    afifo_xslv<2, 2> *tt;

};

}  // namespace debugger

