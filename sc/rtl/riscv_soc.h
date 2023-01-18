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
#include "misclib/apb_gpio.h"
#include "misclib/apb_spi.h"

namespace debugger {

SC_MODULE(riscv_soc) {
 public:
    sc_in<bool> i_sys_nrst;                                 // Power-on system reset active LOW
    sc_in<bool> i_sys_clk;                                  // System/Bus clock
    sc_in<bool> i_dbg_nrst;                                 // Reset from Debug interface (DMI). Reset everything except DMI
    sc_in<bool> i_ddr_nrst;                                 // DDR related logic reset (AXI clock transformator)
    sc_in<bool> i_ddr_clk;                                  // DDR memoru clock
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
    // SPI SD-card signals:
    sc_out<bool> o_spi_cs;
    sc_out<bool> o_spi_sclk;
    sc_out<bool> o_spi_mosi;                                // SPI: Master Output Slave Input
    sc_in<bool> i_spi_miso;                                 // SPI: Master Input Slave Output
    sc_in<bool> i_sd_detected;                              // SD-card detected
    sc_in<bool> i_sd_protect;                               // SD-card write protect
    // PLL and Reset interfaces:
    sc_out<bool> o_dmreset;                                 // Debug reset request. Everything except DMI.
    sc_out<mapinfo_type> o_prci_pmapinfo;                   // PRCI mapping information
    sc_out<dev_config_type> i_prci_pdevcfg;                 // PRCI device descriptor
    sc_out<apb_in_type> o_prci_apbi;                        // APB: PLL and Reset configuration interface
    sc_in<apb_out_type> i_prci_apbo;                        // APB: PLL and Reset configuration interface
    // DDR interfaces:
    sc_out<mapinfo_type> o_ddr_pmapinfo;                    // DDR configuration mapping information
    sc_out<dev_config_type> i_ddr_pdevcfg;                  // DDR configuration device descriptor
    sc_out<apb_in_type> o_ddr_apbi;                         // APB: DDR configuration interface
    sc_in<apb_out_type> i_ddr_apbo;                         // APB: DDR configuration interface
    sc_out<mapinfo_type> o_ddr_xmapinfo;                    // DDR memory bank mapping information
    sc_out<dev_config_type> i_ddr_xdevcfg;                  // DDR memory bank descriptor
    sc_out<axi4_slave_in_type> o_ddr_xslvi;                 // AXI DDR memory interface
    sc_in<axi4_slave_out_type> i_ddr_xslvo;                 // AXI DDR memory interface

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
    static const int SOC_PNP_SPI = 12;
    static const int SOC_PNP_TOTAL = 13;
    
    static const int CFG_SOC_UART1_LOG2_FIFOSZ = 4;
    
    static const int CFG_SOC_GPIO0_WIDTH = 12;
    
    static const int CFG_SOC_SPI0_LOG2_FIFOSZ = 9;
    // Example FU740: S7 Core0 (M) + 4xU74 Cores (M+S).
    static const int CFG_PLIC_CONTEXT_TOTAL = 9;
    // Any number up to 1024. Zero interrupt must be 0.
    static const int CFG_PLIC_IRQ_TOTAL = 73;

    typedef sc_vector<sc_signal<dev_config_type>> soc_pnp_vector;

    sc_signal<axi4_master_out_type> acpo;
    sc_signal<axi4_master_in_type> acpi;
    bus0_mapinfo_vector bus0_mapinfo;
    bus0_xmst_in_vector aximi;
    bus0_xmst_out_vector aximo;
    bus0_xslv_in_vector axisi;
    bus0_xslv_out_vector axiso;
    bus1_mapinfo_vector bus1_mapinfo;
    bus1_apb_in_vector apbi;
    bus1_apb_out_vector apbo;
    soc_pnp_vector dev_pnp;
    sc_signal<sc_uint<64>> wb_clint_mtimer;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_clint_msip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_clint_mtip;
    sc_signal<sc_uint<CFG_PLIC_CONTEXT_TOTAL>> wb_plic_xeip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_meip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_seip;
    sc_signal<bool> w_irq_uart1;
    sc_signal<sc_uint<CFG_SOC_GPIO0_WIDTH>> wb_irq_gpio;
    sc_signal<bool> w_irq_pnp;
    sc_signal<sc_biguint<CFG_PLIC_IRQ_TOTAL>> wb_ext_irqs;

    axi2apb *apbrdg0;
    apb_uart<CFG_SOC_UART1_LOG2_FIFOSZ> *uart1;
    apb_gpio<CFG_SOC_GPIO0_WIDTH> *gpio0;
    apb_spi<CFG_SOC_SPI0_LOG2_FIFOSZ> *spi0;
    Workgroup *group0;

};

}  // namespace debugger

