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
#include "../../rtl/internal/ambalib/types_pnp.h"
#include "../../rtl/internal/ambalib/types_amba.h"
#include "../../rtl/sim/pll/pll_generic.h"
#include "../../rtl/internal/ambalib/axi_slv.h"

namespace debugger {

SC_MODULE(axi_slv_tb) {
 public:

    void init();
    void comb();
    void test_clk();
    void registers();

    axi_slv_tb(sc_module_name name);
    virtual ~axi_slv_tb();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct axi_slv_tb_registers {
        sc_signal<sc_uint<32>> wb_clk_cnt;
        sc_signal<sc_uint<32>> test_cnt;
    };

    void axi_slv_tb_r_reset(axi_slv_tb_registers& iv) {
        iv.wb_clk_cnt = 0;
        iv.test_cnt = 0;
    }

    sc_signal<bool> w_nrst;                                 // Power-on system reset active LOW
    sc_signal<bool> w_clk;
    sc_signal<dev_config_type> wb_mst_o_cfg;                // Master config descriptor
    sc_signal<axi4_slave_in_type> wb_xslvi;
    sc_signal<axi4_slave_out_type> wb_xslvo;
    sc_signal<dev_config_type> wb_slv_o_cfg;                // Slave config descriptor
    sc_signal<mapinfo_type> wb_slv_mapinfo;
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
    axi_slv_tb_registers v;
    axi_slv_tb_registers r;

    pll_generic *clk;
    axi_slv *slv0;

};

}  // namespace debugger

