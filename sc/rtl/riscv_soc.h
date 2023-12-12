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
#include "ambalib/types_amba.h"
#include "ambalib/types_pnp.h"
#include "ambalib/types_bus0.h"
#include "ambalib/types_bus1.h"
#include "riverlib/river_cfg.h"
#include "../prj/impl/asic/target_cfg.h"
#include "ambalib/axictrl_bus0.h"
#include "ambalib/axi2apb_bus1.h"
#include "misclib/axi_rom.h"
#include "misclib/axi_sram.h"
#include "misclib/clint.h"
#include "misclib/plic.h"
#include "misclib/apb_uart.h"
#include "misclib/apb_gpio.h"
#include "sdctrl/sdctrl.h"
#include "misclib/apb_pnp.h"
#include "riverlib/workgroup.h"
#include "techmap/cdc_axi_sync/cdc_axi_sync_tech.h"
#include "sv_func.h"

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
    // SD-card signals:
    sc_out<bool> o_sd_sclk;                                 // Clock up to 50 MHz
    sc_in<bool> i_sd_cmd;                                   // Command response;
    sc_out<bool> o_sd_cmd;                                  // Command request; DO in SPI mode
    sc_out<bool> o_sd_cmd_dir;                              // Direction bit: 1=input; 0=output
    sc_in<bool> i_sd_dat0;                                  // Data Line[0] input; DI in SPI mode
    sc_out<bool> o_sd_dat0;                                 // Data Line[0] output
    sc_out<bool> o_sd_dat0_dir;                             // Direction bit: 1=input; 0=output
    sc_in<bool> i_sd_dat1;                                  // Data Line[1] input
    sc_out<bool> o_sd_dat1;                                 // Data Line[1] output
    sc_out<bool> o_sd_dat1_dir;                             // Direction bit: 1=input; 0=output
    sc_in<bool> i_sd_dat2;                                  // Data Line[2] input
    sc_out<bool> o_sd_dat2;                                 // Data Line[2] output
    sc_out<bool> o_sd_dat2_dir;                             // Direction bit: 1=input; 0=output
    sc_in<bool> i_sd_cd_dat3;                               // Card Detect / Data Line[3] input
    sc_out<bool> o_sd_cd_dat3;                              // Card Detect / Data Line[3] output; CS output in SPI mode
    sc_out<bool> o_sd_cd_dat3_dir;                          // Direction bit: 1=input; 0=output
    sc_in<bool> i_sd_detected;                              // SD-card detected
    sc_in<bool> i_sd_protect;                               // SD-card write protect
    // PLL and Reset interfaces:
    sc_out<bool> o_dmreset;                                 // Debug reset request. Everything except DMI.
    sc_out<mapinfo_type> o_prci_pmapinfo;                   // PRCI mapping information
    sc_in<dev_config_type> i_prci_pdevcfg;                  // PRCI device descriptor
    sc_out<apb_in_type> o_prci_apbi;                        // APB: PLL and Reset configuration interface
    sc_in<apb_out_type> i_prci_apbo;                        // APB: PLL and Reset configuration interface
    // DDR interfaces:
    sc_out<mapinfo_type> o_ddr_pmapinfo;                    // DDR configuration mapping information
    sc_in<dev_config_type> i_ddr_pdevcfg;                   // DDR configuration device descriptor
    sc_out<apb_in_type> o_ddr_apbi;                         // APB: DDR configuration interface
    sc_in<apb_out_type> i_ddr_apbo;                         // APB: DDR configuration interface
    sc_out<mapinfo_type> o_ddr_xmapinfo;                    // DDR memory bank mapping information
    sc_in<dev_config_type> i_ddr_xdevcfg;                   // DDR memory bank descriptor
    sc_out<axi4_slave_in_type> o_ddr_xslvi;                 // AXI DDR memory interface
    sc_in<axi4_slave_out_type> i_ddr_xslvo;                 // AXI DDR memory interface

    void comb();

    SC_HAS_PROCESS(riscv_soc);

    riscv_soc(sc_module_name name,
              bool async_reset,
              int sim_uart_speedup_rate);
    virtual ~riscv_soc();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int sim_uart_speedup_rate_;


    // Hardware SoC Identificator.
    // Read Only unique platform identificator that could be read by FW
    static const uint32_t SOC_HW_ID = 0x20220903;

    // UARTx fifo log2(size) in bytes:
    static const int SOC_UART1_LOG2_FIFOSZ = 4;

    // Number of available generic IO pins:
    static const int SOC_GPIO0_WIDTH = 12;

    // SD-card in SPI mode buffer size. It should be at least log2(512) Bytes:
    static const int SOC_SPI0_LOG2_FIFOSZ = 9;

    // Number of contexts in PLIC controller.
    // Example FU740: S7 Core0 (M) + 4xU74 Cores (M+S).
    static const int SOC_PLIC_CONTEXT_TOTAL = 9;
    // Any number up to 1024. Zero interrupt must be 0.
    static const int SOC_PLIC_IRQ_TOTAL = 73;

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
    sc_signal<sc_uint<SOC_PLIC_CONTEXT_TOTAL>> wb_plic_xeip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_meip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_plic_seip;
    sc_signal<bool> w_irq_uart1;
    sc_signal<sc_uint<SOC_GPIO0_WIDTH>> wb_irq_gpio;
    sc_signal<bool> w_irq_pnp;
    sc_signal<sc_biguint<SOC_PLIC_IRQ_TOTAL>> wb_ext_irqs;

    axictrl_bus0 *bus0;
    axi2apb_bus1 *bus1;
    axi_rom<CFG_BOOTROM_LOG2_SIZE> *rom0;
    axi_sram<CFG_SRAM_LOG2_SIZE> *sram0;
    clint<CFG_CPU_MAX> *clint0;
    plic<SOC_PLIC_CONTEXT_TOTAL, SOC_PLIC_IRQ_TOTAL> *plic0;
    apb_uart<SOC_UART1_LOG2_FIFOSZ> *uart1;
    apb_gpio<SOC_GPIO0_WIDTH> *gpio0;
    sdctrl *sdctrl0;
    apb_pnp<SOC_PNP_TOTAL> *pnp0;
    Workgroup *group0;
    cdc_axi_sync_tech *u_cdc_ddr0;

};

}  // namespace debugger

