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
#include "../../../rtl/ambalib/types_amba.h"
#include "../../../rtl/ambalib/types_pnp.h"
#include "../../../rtl/techmap/bufg/ids_tech.h"
#include "../../../rtl/techmap/bufg/iobuf_tech.h"
#include "../../../rtl/techmap/pll/SysPLL_tech.h"
#include "../../../rtl/misclib/apb_prci.h"
#include "../../../rtl/riscv_soc.h"
#include "sv_func.h"

namespace debugger {

SC_MODULE(asic_top) {
 public:
    sc_in<bool> i_rst;                                      // Power-on system reset active HIGH
    // Differential clock (LVDS) positive/negaive signal.
    sc_in<bool> i_sclk_p;
    sc_in<bool> i_sclk_n;
    // GPIO: [11:4] LEDs; [3:0] DIP switch
    sc_inout<sc_uint<12>> io_gpio;
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
    sc_out<bool> o_sd_sclk;
    sc_inout<bool> io_sd_cmd;                               // CMD IO Command/Resonse; Data output in SPI mode
    sc_inout<bool> io_sd_dat0;                              // Data[0] IO; Data input in SPI mode
    sc_inout<bool> io_sd_dat1;
    sc_inout<bool> io_sd_dat2;
    sc_inout<bool> io_sd_cd_dat3;                           // CD/DAT3 IO CardDetect/Data[3]; CS output in SPI mode
    sc_in<bool> i_sd_detected;                              // SD-card detected
    sc_in<bool> i_sd_protect;                               // SD-card write protect


    asic_top(sc_module_name name,
             int sim_uart_speedup_rate);
    virtual ~asic_top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    int sim_uart_speedup_rate_;

    static const bool async_reset = 0;

    sc_signal<bool> ib_clk_tcxo;
    sc_signal<sc_uint<12>> ib_gpio_ipins;
    sc_signal<sc_uint<12>> ob_gpio_opins;
    sc_signal<sc_uint<12>> ob_gpio_direction;
    sc_signal<bool> ib_sd_cmd;
    sc_signal<bool> ob_sd_cmd;
    sc_signal<bool> ob_sd_cmd_direction;
    sc_signal<bool> ib_sd_dat0;
    sc_signal<bool> ob_sd_dat0;
    sc_signal<bool> ob_sd_dat0_direction;
    sc_signal<bool> ib_sd_dat1;
    sc_signal<bool> ob_sd_dat1;
    sc_signal<bool> ob_sd_dat1_direction;
    sc_signal<bool> ib_sd_dat2;
    sc_signal<bool> ob_sd_dat2;
    sc_signal<bool> ob_sd_dat2_direction;
    sc_signal<bool> ib_sd_cd_dat3;
    sc_signal<bool> ob_sd_cd_dat3;
    sc_signal<bool> ob_sd_cd_dat3_direction;
    sc_signal<bool> w_sys_rst;
    sc_signal<bool> w_sys_nrst;
    sc_signal<bool> w_dbg_nrst;
    sc_signal<bool> w_dmreset;
    sc_signal<bool> w_sys_clk;
    sc_signal<bool> w_ddr_clk;
    sc_signal<bool> w_pll_lock;
    sc_signal<mapinfo_type> ddr_xmapinfo;
    sc_signal<dev_config_type> ddr_xdev_cfg;
    sc_signal<axi4_slave_out_type> ddr_xslvo;
    sc_signal<axi4_slave_in_type> ddr_xslvi;
    sc_signal<mapinfo_type> ddr_pmapinfo;
    sc_signal<dev_config_type> ddr_pdev_cfg;
    sc_signal<apb_in_type> ddr_apbi;
    sc_signal<apb_out_type> ddr_apbo;
    sc_signal<bool> w_ddr_ui_nrst;
    sc_signal<bool> w_ddr_ui_clk;
    sc_signal<bool> w_ddr3_init_calib_complete;
    sc_signal<mapinfo_type> prci_pmapinfo;
    sc_signal<dev_config_type> prci_dev_cfg;
    sc_signal<apb_in_type> prci_apbi;
    sc_signal<apb_out_type> prci_apbo;

    ids_tech *iclk0;
    iobuf_tech *iosdcmd0;
    iobuf_tech *iosddat0;
    iobuf_tech *iosddat1;
    iobuf_tech *iosddat2;
    iobuf_tech *iosddat3;
    SysPLL_tech *pll0;
    apb_prci *prci0;
    riscv_soc *soc0;

};

}  // namespace debugger

