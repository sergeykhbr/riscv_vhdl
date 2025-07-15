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
#include "types_gencpu64_bus1.h"
#include "../ambalib/axi_slv.h"

namespace debugger {

SC_MODULE(gencpu64_axi2apb_bus1) {
 public:
    sc_in<bool> i_clk;                                      // APB clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // Base address information from the interconnect port
    sc_out<dev_config_type> o_cfg;                          // Slave config descriptor
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI4 Interconnect Bridge interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI4 Bridge to Interconnect interface
    sc_vector<sc_in<apb_out_type>> i_apbo;                  // APB slaves output vector
    sc_vector<sc_out<apb_in_type>> o_apbi;                  // APB slaves input vector
    sc_vector<sc_out<mapinfo_type>> o_mapinfo;              // APB devices memory mapping information

    void comb();
    void registers();

    gencpu64_axi2apb_bus1(sc_module_name name,
                          bool async_reset);
    virtual ~gencpu64_axi2apb_bus1();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t State_Idle = 0;
    static const uint8_t State_setup = 1;
    static const uint8_t State_access = 2;
    static const uint8_t State_out = 3;

    struct gencpu64_axi2apb_bus1_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CFG_BUS1_PSLV_LOG2_TOTAL>> selidx;
        sc_signal<bool> pvalid;
        sc_signal<sc_uint<32>> paddr;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> pwdata;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> prdata;
        sc_signal<bool> pwrite;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> pstrb;
        sc_signal<sc_uint<3>> pprot;
        sc_signal<bool> pselx;
        sc_signal<bool> penable;
        sc_signal<bool> pslverr;
        sc_signal<sc_uint<8>> size;
    };

    void gencpu64_axi2apb_bus1_r_reset(gencpu64_axi2apb_bus1_registers& iv) {
        iv.state = State_Idle;
        iv.selidx = 0;
        iv.pvalid = 0;
        iv.paddr = 0;
        iv.pwdata = 0;
        iv.prdata = 0;
        iv.pwrite = 0;
        iv.pstrb = 0;
        iv.pprot = 0;
        iv.pselx = 0;
        iv.penable = 0;
        iv.pslverr = 0;
        iv.size = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> wb_req_addr;
    sc_signal<sc_uint<8>> wb_req_size;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_req_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_req_wstrb;
    sc_signal<bool> w_req_last;
    sc_signal<bool> w_req_ready;
    gencpu64_axi2apb_bus1_registers v;
    gencpu64_axi2apb_bus1_registers r;

    axi_slv *axi0;

};

}  // namespace debugger

