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
#include "sdctrl_cfg.h"
#include "../ambalib/axi_slv.h"
#include "sdctrl_regs.h"
#include "sdctrl_err.h"
#include "sdctrl_wdog.h"
#include "sdctrl_crc16.h"
#include "sdctrl_spimode.h"
#include "sdctrl_sdmode.h"
#include "sdctrl_cmd_transmitter.h"
#include "sdctrl_cache.h"

namespace debugger {

SC_MODULE(sdctrl) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_xmapinfo;                         // APB interconnect slot information
    sc_out<dev_config_type> o_xcfg;                         // APB Device descriptor
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI input interface to access SD-card memory
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI output interface to access SD-card memory
    sc_in<mapinfo_type> i_pmapinfo;                         // APB interconnect slot information
    sc_out<dev_config_type> o_pcfg;                         // APB sd-controller configuration registers descriptor
    sc_in<apb_in_type> i_apbi;                              // APB Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_sclk;                                    // Clock up to 50 MHz
    sc_in<bool> i_cmd;                                      // Command response;
    sc_out<bool> o_cmd;                                     // Command request; DO in SPI mode
    sc_out<bool> o_cmd_dir;                                 // Direction bit: 1=input; 0=output
    sc_in<bool> i_dat0;                                     // Data Line[0] input; DI in SPI mode
    sc_out<bool> o_dat0;                                    // Data Line[0] output
    sc_out<bool> o_dat0_dir;                                // Direction bit: 1=input; 0=output
    sc_in<bool> i_dat1;                                     // Data Line[1] input
    sc_out<bool> o_dat1;                                    // Data Line[1] output
    sc_out<bool> o_dat1_dir;                                // Direction bit: 1=input; 0=output
    sc_in<bool> i_dat2;                                     // Data Line[2] input
    sc_out<bool> o_dat2;                                    // Data Line[2] output
    sc_out<bool> o_dat2_dir;                                // Direction bit: 1=input; 0=output
    sc_in<bool> i_cd_dat3;                                  // Card Detect / Data Line[3] input
    sc_out<bool> o_cd_dat3;                                 // Card Detect / Data Line[3] output; CS output in SPI mode
    sc_out<bool> o_cd_dat3_dir;                             // Direction bit: 1=input; 0=output
    sc_in<bool> i_detected;
    sc_in<bool> i_protect;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl);

    sdctrl(sc_module_name name,
           bool async_reset);
    virtual ~sdctrl();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // SD controller modes:
    static const uint8_t MODE_PRE_INIT = 0;
    static const uint8_t MODE_SPI = 1;
    static const uint8_t MODE_SD = 2;

    struct sdctrl_registers {
        sc_signal<bool> nrst_spimode;
        sc_signal<bool> nrst_sdmode;
        sc_signal<sc_uint<7>> clkcnt;
        sc_signal<bool> cmd_set_low;
        sc_signal<sc_uint<2>> mode;
    } v, r;

    void sdctrl_r_reset(sdctrl_registers &iv) {
        iv.nrst_spimode = 0;
        iv.nrst_sdmode = 0;
        iv.clkcnt = 0;
        iv.cmd_set_low = 0;
        iv.mode = MODE_PRE_INIT;
    }

    sc_signal<bool> w_regs_sck_posedge;
    sc_signal<bool> w_regs_sck;
    sc_signal<bool> w_regs_err_clear;
    sc_signal<sc_uint<16>> wb_regs_watchdog;
    sc_signal<bool> w_regs_spi_mode;
    sc_signal<bool> w_regs_pcie_12V_support;
    sc_signal<bool> w_regs_pcie_available;
    sc_signal<sc_uint<4>> wb_regs_voltage_supply;
    sc_signal<sc_uint<8>> wb_regs_check_pattern;
    sc_signal<bool> w_mem_req_valid;
    sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> wb_mem_req_addr;
    sc_signal<sc_uint<8>> wb_mem_req_size;
    sc_signal<bool> w_mem_req_write;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_mem_req_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_mem_req_wstrb;
    sc_signal<bool> w_mem_req_last;
    sc_signal<bool> w_cache_req_ready;
    sc_signal<bool> w_cache_resp_valid;
    sc_signal<sc_uint<64>> wb_cache_resp_rdata;
    sc_signal<bool> w_cache_resp_err;
    sc_signal<bool> w_cache_resp_ready;
    sc_signal<bool> w_req_sdmem_valid;
    sc_signal<bool> w_req_sdmem_write;
    sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> wb_req_sdmem_addr;
    sc_signal<sc_biguint<SDCACHE_LINE_BITS>> wb_req_sdmem_wdata;
    sc_signal<bool> w_regs_flush_valid;
    sc_signal<bool> w_cache_flush_end;
    sc_signal<bool> w_trx_cmd;
    sc_signal<bool> w_trx_cmd_dir;
    sc_signal<bool> w_trx_cmd_csn;
    sc_signal<bool> w_trx_wdog_ena;
    sc_signal<bool> w_trx_err_valid;
    sc_signal<sc_uint<4>> wb_trx_err_setcode;
    sc_signal<bool> w_cmd_in;
    sc_signal<bool> w_cmd_req_ready;
    sc_signal<bool> w_cmd_resp_valid;
    sc_signal<sc_uint<6>> wb_cmd_resp_cmd;
    sc_signal<sc_uint<32>> wb_cmd_resp_reg;
    sc_signal<sc_uint<7>> wb_cmd_resp_crc7_rx;
    sc_signal<sc_uint<7>> wb_cmd_resp_crc7_calc;
    sc_signal<sc_uint<15>> wb_cmd_resp_spistatus;
    sc_signal<bool> w_cmd_resp_ready;
    sc_signal<sc_uint<16>> wb_crc16_0;
    sc_signal<sc_uint<16>> wb_crc16_1;
    sc_signal<sc_uint<16>> wb_crc16_2;
    sc_signal<sc_uint<16>> wb_crc16_3;
    sc_signal<bool> w_wdog_trigger;
    sc_signal<sc_uint<4>> wb_err_code;
    sc_signal<bool> w_err_pending;

    // SPI-mode controller signals:
    sc_signal<bool> w_spi_dat;
    sc_signal<bool> w_spi_dat_csn;
    sc_signal<bool> w_spi_cmd_req_valid;
    sc_signal<sc_uint<6>> wb_spi_cmd_req_cmd;
    sc_signal<sc_uint<32>> wb_spi_cmd_req_arg;
    sc_signal<sc_uint<3>> wb_spi_cmd_req_rn;
    sc_signal<bool> w_spi_req_sdmem_ready;
    sc_signal<bool> w_spi_resp_sdmem_valid;
    sc_signal<sc_biguint<512>> wb_spi_resp_sdmem_data;
    sc_signal<bool> w_spi_err_valid;
    sc_signal<bool> w_spi_err_clear;
    sc_signal<sc_uint<4>> wb_spi_err_setcode;
    sc_signal<bool> w_spi_400kHz_ena;
    sc_signal<sc_uint<3>> wb_spi_sdtype;
    sc_signal<bool> w_spi_wdog_ena;
    sc_signal<bool> w_spi_crc16_clear;
    sc_signal<bool> w_spi_crc16_next;

    // SD-mode controller signals:
    sc_signal<bool> w_sd_dat0;
    sc_signal<bool> w_sd_dat0_dir;
    sc_signal<bool> w_sd_dat1;
    sc_signal<bool> w_sd_dat1_dir;
    sc_signal<bool> w_sd_dat2;
    sc_signal<bool> w_sd_dat2_dir;
    sc_signal<bool> w_sd_dat3;
    sc_signal<bool> w_sd_dat3_dir;
    sc_signal<bool> w_sd_cmd_req_valid;
    sc_signal<sc_uint<6>> wb_sd_cmd_req_cmd;
    sc_signal<sc_uint<32>> wb_sd_cmd_req_arg;
    sc_signal<sc_uint<3>> wb_sd_cmd_req_rn;
    sc_signal<bool> w_sd_req_sdmem_ready;
    sc_signal<bool> w_sd_resp_sdmem_valid;
    sc_signal<sc_biguint<512>> wb_sd_resp_sdmem_data;
    sc_signal<bool> w_sd_err_valid;
    sc_signal<bool> w_sd_err_clear;
    sc_signal<sc_uint<4>> wb_sd_err_setcode;
    sc_signal<bool> w_sd_400kHz_ena;
    sc_signal<sc_uint<3>> wb_sd_sdtype;
    sc_signal<bool> w_sd_wdog_ena;
    sc_signal<bool> w_sd_crc16_clear;
    sc_signal<bool> w_sd_crc16_next;

    // Mode multiplexed signals:
    sc_signal<bool> w_cmd_req_valid;
    sc_signal<sc_uint<6>> wb_cmd_req_cmd;
    sc_signal<sc_uint<32>> wb_cmd_req_arg;
    sc_signal<sc_uint<3>> wb_cmd_req_rn;
    sc_signal<bool> w_req_sdmem_ready;
    sc_signal<bool> w_resp_sdmem_valid;
    sc_signal<sc_biguint<512>> wb_resp_sdmem_data;
    sc_signal<bool> w_err_valid;
    sc_signal<bool> w_err_clear;
    sc_signal<sc_uint<4>> wb_err_setcode;
    sc_signal<bool> w_400kHz_ena;
    sc_signal<sc_uint<3>> wb_sdtype;
    sc_signal<bool> w_wdog_ena;
    sc_signal<bool> w_crc16_clear;
    sc_signal<bool> w_crc16_next;

    axi_slv *xslv0;
    sdctrl_regs *regs0;
    sdctrl_err *err0;
    sdctrl_wdog *wdog0;
    sdctrl_crc16 *crcdat0;
    sdctrl_crc16 *crcdat1;
    sdctrl_crc16 *crcdat2;
    sdctrl_crc16 *crcdat3;
    sdctrl_spimode *spimode0;
    sdctrl_sdmode *sdmode0;
    sdctrl_cmd_transmitter *cmdtrx0;
    sdctrl_cache *cache0;

};

}  // namespace debugger

