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
#include "../prj/impl/asic_full/config_target.h"
#include "ambalib/types_amba.h"
#include "ambalib/types_bus0.h"
#include "riverlib/river_cfg.h"
#include "riverlib/types_river.h"
#include "riverlib/workgroup.h"

namespace debugger {

SC_MODULE(riscv_soc) {
 public:
    sc_in<bool> i_rst;                                      // System reset active HIGH
    sc_in<bool> i_clk;                                      // CPU clock
    // GPIO signals:
    sc_in<sc_uint<12>> i_gpio;
    sc_out<sc_uint<12>> o_gpio;
    sc_out<sc_uint<12>> o_gpio_dir;
    // JTAG signals:
    sc_in<bool> i_jtag_trst;
    sc_in<bool> i_jtag_tck;
    sc_in<bool> i_jtag_tms;
    sc_in<bool> i_jtag_tdi;
    sc_out<bool> o_jtag_tdo;
    sc_out<bool> o_jtag_vref;
    // UART1 signals
    sc_in<bool> i_uart1_rd;
    sc_out<bool> o_uart1_td;

    void comb();

    SC_HAS_PROCESS(riscv_soc);

    riscv_soc(sc_module_name name);
    virtual ~riscv_soc();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const bool async_reset = CFG_ASYNC_RESET;

    sc_signal<bool> w_sys_nrst;                             // System reset of whole system
    sc_signal<bool> w_dbg_nrst;                             // Reset workgroup debug interface
    sc_signal<bool> w_dmreset;                              // Reset request from workgroup debug interface
    sc_signal<axi4_master_out_type> acpo;
    sc_signal<axi4_master_in_type> acpi;
    sc_signal<apb_in_type> apb_dmi_i;
    sc_signal<apb_out_type> apb_dmi_o;
    bus0_xmst_in_vector aximi;
    bus0_xmst_out_vector aximo;
    bus0_xslv_in_vector axisi;
    bus0_xslv_out_vector axiso;
    bus0_xslv_cfg_vector slv_cfg;
    bus0_xmst_cfg_vector mst_cfg;
    sc_signal<sc_uint<64>> wb_clint_mtimer;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_clint_msip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_clint_mtip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_meip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_seip;

    Workgroup *group0;

};

}  // namespace debugger

