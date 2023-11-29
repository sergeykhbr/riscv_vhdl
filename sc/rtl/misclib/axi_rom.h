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
#include "../ambalib/axi_slv.h"
#include "../techmap/mem/rom_tech.h"
#include "api_core.h"
#include "sv_func.h"

namespace debugger {

template<int abits = 17>
SC_MODULE(axi_rom) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI Slave to Bridge interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI Bridge to Slave interface

    void comb();

    SC_HAS_PROCESS(axi_rom);

    axi_rom(sc_module_name name,
            bool async_reset,
            std::string filename);
    virtual ~axi_rom();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    std::string filename_;

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> wb_req_addr;
    sc_signal<sc_uint<8>> wb_req_size;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_req_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_req_wstrb;
    sc_signal<bool> w_req_last;
    sc_signal<bool> w_req_ready;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_resp_rdata;
    sc_signal<bool> wb_resp_err;
    sc_signal<sc_uint<abits>> wb_req_addr_abits;

    axi_slv *xslv0;
    rom_tech<abits, CFG_LOG2_SYSBUS_DATA_BYTES> *tech0;

};

template<int abits>
axi_rom<abits>::axi_rom(sc_module_name name,
                        bool async_reset,
                        std::string filename)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo") {

    async_reset_ = async_reset;
    filename_ = filename;
    xslv0 = 0;
    tech0 = 0;

    xslv0 = new axi_slv("xslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_ROM);
    xslv0->i_clk(i_clk);
    xslv0->i_nrst(i_nrst);
    xslv0->i_mapinfo(i_mapinfo);
    xslv0->o_cfg(o_cfg);
    xslv0->i_xslvi(i_xslvi);
    xslv0->o_xslvo(o_xslvo);
    xslv0->o_req_valid(w_req_valid);
    xslv0->o_req_addr(wb_req_addr);
    xslv0->o_req_size(wb_req_size);
    xslv0->o_req_write(w_req_write);
    xslv0->o_req_wdata(wb_req_wdata);
    xslv0->o_req_wstrb(wb_req_wstrb);
    xslv0->o_req_last(w_req_last);
    xslv0->i_req_ready(w_req_ready);
    xslv0->i_resp_valid(w_resp_valid);
    xslv0->i_resp_rdata(wb_resp_rdata);
    xslv0->i_resp_err(wb_resp_err);

    tech0 = new rom_tech<abits,
                         CFG_LOG2_SYSBUS_DATA_BYTES>("tech0",
                                                     filename);
    tech0->i_clk(i_clk);
    tech0->i_addr(wb_req_addr_abits);
    tech0->o_rdata(wb_resp_rdata);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_xslvi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << wb_req_size;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << wb_req_wstrb;
    sensitive << w_req_last;
    sensitive << w_req_ready;
    sensitive << w_resp_valid;
    sensitive << wb_resp_rdata;
    sensitive << wb_resp_err;
    sensitive << wb_req_addr_abits;
}

template<int abits>
axi_rom<abits>::~axi_rom() {
    if (xslv0) {
        delete xslv0;
    }
    if (tech0) {
        delete tech0;
    }
}

template<int abits>
void axi_rom<abits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
    }

    if (xslv0) {
        xslv0->generateVCD(i_vcd, o_vcd);
    }
    if (tech0) {
        tech0->generateVCD(i_vcd, o_vcd);
    }
}

template<int abits>
void axi_rom<abits>::comb() {

    wb_req_addr_abits = wb_req_addr.read()((abits - 1), 0);
    w_req_ready = 1;
    w_resp_valid = 1;
    wb_resp_err = 0;
}

}  // namespace debugger

