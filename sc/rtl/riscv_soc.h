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
#include "ambalib/types_bus1.h"
#include "riverlib/river_cfg.h"
#include "riverlib/types_river.h"
#include "riverlib/workgroup.h"
#include "ambalib/axi2apb.h"
#include "misclib/apb_uart.h"

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
    
    static const int SOC_PNP_XCTRL0 = 0;
    static const int SOC_PNP_GROUP0 = 1;
    static const int SOC_PNP_BOOTROM = 2;
    static const int SOC_PNP_SRAM = 3;
    static const int SOC_PNP_DDR = 4;
    static const int SOC_PNP_GPIO = 5;
    static const int SOC_PNP_CLINT = 6;
    static const int SOC_PNP_PLIC = 7;
    static const int SOC_PNP_PNP = 8;
    static const int SOC_PNP_PBRIDGE0 = 9;
    static const int SOC_PNP_DMI = 10;
    static const int SOC_PNP_UART1 = 11;
    static const int SOC_PNP_TOTAL = 12;
    
    static const int CFG_SOC_UART1_LOG2_FIFOSZ = 4;
    // Example FU740: S7 Core0 (M) + 4xU74 Cores (M+S).
    static const int CFG_PLIC_CONTEXT_TOTAL = 9;
    // Any number up to 1024. Zero interrupt must be 0.
    static const int CFG_PLIC_IRQ_TOTAL = 73;

    typedef sc_vector<sc_signal<dev_config_type>> soc_pnp_vector;

    sc_signal<bool> w_sys_nrst;                             // System reset of whole system
    sc_signal<bool> w_dbg_nrst;                             // Reset workgroup debug interface
    sc_signal<bool> w_dmreset;                              // Reset request from workgroup debug interface
    sc_signal<axi4_master_out_type> acpo;
    sc_signal<axi4_master_in_type> acpi;
    bus0_mapinfo_vector bus0_mapinfo;
    bus0_xmst_in_vector aximi;
    bus0_xmst_out_vector aximo;
    bus0_xslv_in_vector axisi;
    bus0_xslv_out_vector axiso;
    bus1_mapinfo_vector bus1_mapinfo;
    bus1_pmst_in_vector apbmi;
    bus1_pmst_out_vector apbmo;
    bus1_pslv_in_vector apbsi;
    bus1_pslv_out_vector apbso;
    soc_pnp_vector dev_pnp;
    sc_signal<sc_uint<64>> wb_clint_mtimer;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_clint_msip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_clint_mtip;
    sc_signal<sc_uint<CFG_PLIC_CONTEXT_TOTAL>> wb_plic_xeip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_meip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_seip;
    sc_signal<bool> w_irq_uart1;
    sc_signal<sc_uint<16>> wb_irq_gpio;
    sc_signal<bool> w_irq_pnp;
    sc_signal<sc_biguint<CFG_PLIC_IRQ_TOTAL>> wb_ext_irqs;

    axi2apb *apbrdg0;
    apb_uart<CFG_SOC_UART1_LOG2_FIFOSZ> *uart1;
    Workgroup *group0;

};

}  // namespace debugger

