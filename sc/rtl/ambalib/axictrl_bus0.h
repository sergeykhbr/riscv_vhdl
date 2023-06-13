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
#include "types_pnp.h"
#include "types_amba.h"
#include "types_bus0.h"
#include "axi_slv.h"

namespace debugger {

SC_MODULE(axictrl_bus0) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_out<dev_config_type> o_cfg;                          // Slave config descriptor
    sc_vector<sc_in<axi4_master_out_type>> i_xmsto;         // AXI4 masters output vector
    sc_vector<sc_out<axi4_master_in_type>> o_xmsti;         // AXI4 masters input vector
    sc_vector<sc_in<axi4_slave_out_type>> i_xslvo;          // AXI4 slaves output vectors
    sc_vector<sc_out<axi4_slave_in_type>> o_xslvi;          // AXI4 slaves input vectors
    sc_vector<sc_out<mapinfo_type>> o_mapinfo;              // AXI devices memory mapping information

    void comb();
    void registers();

    SC_HAS_PROCESS(axictrl_bus0);

    axictrl_bus0(sc_module_name name,
                 bool async_reset);
    virtual ~axictrl_bus0();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct axictrl_bus0_registers {
        sc_signal<sc_uint<CFG_BUS0_XMST_LOG2_TOTAL>> r_midx;
        sc_signal<sc_uint<CFG_BUS0_XSLV_LOG2_TOTAL>> r_sidx;
        sc_signal<sc_uint<CFG_BUS0_XMST_LOG2_TOTAL>> w_midx;
        sc_signal<sc_uint<CFG_BUS0_XSLV_LOG2_TOTAL>> w_sidx;
        sc_signal<sc_uint<CFG_BUS0_XMST_LOG2_TOTAL>> b_midx;
        sc_signal<sc_uint<CFG_BUS0_XSLV_LOG2_TOTAL>> b_sidx;
    } v, r;

    void axictrl_bus0_r_reset(axictrl_bus0_registers &iv) {
        iv.r_midx = CFG_BUS0_XMST_TOTAL;
        iv.r_sidx = CFG_BUS0_XSLV_TOTAL;
        iv.w_midx = CFG_BUS0_XMST_TOTAL;
        iv.w_sidx = CFG_BUS0_XSLV_TOTAL;
        iv.b_midx = CFG_BUS0_XMST_TOTAL;
        iv.b_sidx = CFG_BUS0_XSLV_TOTAL;
    }

    sc_signal<mapinfo_type> wb_def_mapinfo;
    sc_signal<axi4_slave_in_type> wb_def_xslvi;
    sc_signal<axi4_slave_out_type> wb_def_xslvo;
    sc_signal<bool> w_def_req_valid;
    sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> wb_def_req_addr;
    sc_signal<sc_uint<8>> wb_def_req_size;
    sc_signal<bool> w_def_req_write;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_def_req_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_def_req_wstrb;
    sc_signal<bool> w_def_req_last;
    sc_signal<bool> w_def_req_ready;
    sc_signal<bool> w_def_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_def_resp_rdata;
    sc_signal<bool> w_def_resp_err;

    axi_slv *xdef0;

};

}  // namespace debugger

