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
#include "sdctrl_crc7.h"
#include "sdctrl_crc16.h"
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

    static const int log2_fifosz = 9;
    static const int fifo_dbits = 8;
    // SD-card states see Card Status[12:9] CURRENT_STATE on page 145:
    static const uint8_t SDSTATE_SPI_DATA = 0xE;
    static const uint8_t SDSTATE_PRE_INIT = 0xF;
    static const uint8_t SDSTATE_IDLE = 0;
    static const uint8_t SDSTATE_READY = 1;
    static const uint8_t SDSTATE_IDENT = 2;
    static const uint8_t SDSTATE_STBY = 3;
    static const uint8_t SDSTATE_TRAN = 4;
    static const uint8_t SDSTATE_DATA = 5;
    static const uint8_t SDSTATE_RCV = 6;
    static const uint8_t SDSTATE_PRG = 7;
    static const uint8_t SDSTATE_DIS = 8;
    static const uint8_t SDSTATE_INA = 9;
    // SD-card 'idle' state substates:
    static const uint8_t IDLESTATE_CMD0 = 0;
    static const uint8_t IDLESTATE_CMD8 = 1;
    static const uint8_t IDLESTATE_CMD55 = 2;
    static const uint8_t IDLESTATE_ACMD41 = 3;
    static const uint8_t IDLESTATE_CMD58 = 4;
    static const uint8_t IDLESTATE_CARD_IDENTIFICATION = 5;
    // SD-card 'ready' state substates:
    static const uint8_t READYSTATE_CMD11 = 0;
    static const uint8_t READYSTATE_CMD2 = 1;
    static const uint8_t READYSTATE_CHECK_CID = 2;
    // SD-card 'ident' state substates:
    static const bool IDENTSTATE_CMD3 = 0;
    static const bool IDENTSTATE_CHECK_RCA = 1;
    // 
    static const uint8_t SPIDATASTATE_WAIT_MEM_REQ = 0;
    static const uint8_t SPIDATASTATE_CACHE_REQ = 1;
    static const uint8_t SPIDATASTATE_CACHE_WAIT_RESP = 2;
    static const uint8_t SPIDATASTATE_CMD17_READ_SINGLE_BLOCK = 3;
    static const uint8_t SPIDATASTATE_CMD24_WRITE_SINGLE_BLOCK = 4;
    static const uint8_t SPIDATASTATE_WAIT_DATA_START = 5;
    static const uint8_t SPIDATASTATE_READING_DATA = 6;
    static const uint8_t SPIDATASTATE_READING_CRC15 = 7;
    static const uint8_t SPIDATASTATE_READING_END = 8;

    struct sdctrl_registers {
        sc_signal<sc_uint<7>> clkcnt;
        sc_signal<bool> cmd_set_low;
        sc_signal<bool> cmd_req_valid;
        sc_signal<sc_uint<6>> cmd_req_cmd;
        sc_signal<sc_uint<32>> cmd_req_arg;
        sc_signal<sc_uint<3>> cmd_req_rn;
        sc_signal<sc_uint<6>> cmd_resp_r1;
        sc_signal<sc_uint<32>> cmd_resp_reg;
        sc_signal<sc_uint<15>> cmd_resp_spistatus;
        sc_signal<bool> cache_req_valid;
        sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> cache_req_addr;
        sc_signal<bool> cache_req_write;
        sc_signal<sc_uint<64>> cache_req_wdata;
        sc_signal<sc_uint<8>> cache_req_wstrb;
        sc_signal<sc_uint<32>> sdmem_addr;
        sc_signal<sc_biguint<512>> sdmem_data;
        sc_signal<bool> sdmem_valid;
        sc_signal<bool> sdmem_err;
        sc_signal<bool> crc16_clear;
        sc_signal<sc_uint<16>> crc16_calc0;
        sc_signal<sc_uint<16>> crc16_rx0;
        sc_signal<sc_uint<4>> dat;
        sc_signal<bool> dat_dir;
        sc_signal<bool> dat3_dir;
        sc_signal<bool> dat_tran;
        sc_signal<sc_uint<4>> sdstate;
        sc_signal<sc_uint<3>> initstate;
        sc_signal<sc_uint<2>> readystate;
        sc_signal<bool> identstate;
        sc_signal<sc_uint<4>> spidatastate;
        sc_signal<bool> wait_cmd_resp;
        sc_signal<sc_uint<3>> sdtype;
        sc_signal<bool> HCS;                                // High Capacity Support
        sc_signal<bool> S18;                                // 1.8V Low voltage
        sc_signal<sc_uint<32>> RCA;                         // Relative Address
        sc_signal<sc_uint<24>> OCR_VoltageWindow;           // all ranges 2.7 to 3.6 V
        sc_signal<sc_uint<12>> bitcnt;
    } v, r;

    void sdctrl_r_reset(sdctrl_registers &iv) {
        iv.clkcnt = 0;
        iv.cmd_set_low = 0;
        iv.cmd_req_valid = 0;
        iv.cmd_req_cmd = 0;
        iv.cmd_req_arg = 0;
        iv.cmd_req_rn = 0;
        iv.cmd_resp_r1 = 0;
        iv.cmd_resp_reg = 0;
        iv.cmd_resp_spistatus = 0;
        iv.cache_req_valid = 0;
        iv.cache_req_addr = 0ull;
        iv.cache_req_write = 0;
        iv.cache_req_wdata = 0ull;
        iv.cache_req_wstrb = 0;
        iv.sdmem_addr = 0;
        iv.sdmem_data = 0ull;
        iv.sdmem_valid = 0;
        iv.sdmem_err = 0;
        iv.crc16_clear = 1;
        iv.crc16_calc0 = 0;
        iv.crc16_rx0 = 0;
        iv.dat = ~0ul;
        iv.dat_dir = DIR_OUTPUT;
        iv.dat3_dir = DIR_INPUT;
        iv.dat_tran = 1;
        iv.sdstate = SDSTATE_PRE_INIT;
        iv.initstate = IDLESTATE_CMD0;
        iv.readystate = READYSTATE_CMD11;
        iv.identstate = IDENTSTATE_CMD3;
        iv.spidatastate = SPIDATASTATE_WAIT_MEM_REQ;
        iv.wait_cmd_resp = 0;
        iv.sdtype = SDCARD_UNKNOWN;
        iv.HCS = 1;
        iv.S18 = 0;
        iv.RCA = 0;
        iv.OCR_VoltageWindow = 0xff8000;
        iv.bitcnt = 0;
    }

    sc_signal<bool> w_regs_sck_posedge;
    sc_signal<bool> w_regs_sck;
    sc_signal<bool> w_regs_clear_cmderr;
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
    sc_signal<bool> w_mem_req_ready;
    sc_signal<bool> w_cache_req_ready;
    sc_signal<bool> w_cache_resp_valid;
    sc_signal<sc_uint<64>> wb_cache_resp_rdata;
    sc_signal<bool> w_cache_resp_err;
    sc_signal<bool> w_cache_resp_ready;
    sc_signal<bool> w_req_sdmem_ready;
    sc_signal<bool> w_req_sdmem_valid;
    sc_signal<bool> w_req_sdmem_write;
    sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> wb_req_sdmem_addr;
    sc_signal<sc_biguint<SDCACHE_LINE_BITS>> wb_req_sdmem_wdata;
    sc_signal<bool> w_regs_flush_valid;
    sc_signal<bool> w_cache_flush_end;
    sc_signal<bool> w_trx_cmd_dir;
    sc_signal<bool> w_trx_cmd_cs;
    sc_signal<bool> w_cmd_in;
    sc_signal<bool> w_cmd_req_ready;
    sc_signal<bool> w_cmd_resp_valid;
    sc_signal<sc_uint<6>> wb_cmd_resp_cmd;
    sc_signal<sc_uint<32>> wb_cmd_resp_reg;
    sc_signal<sc_uint<7>> wb_cmd_resp_crc7_rx;
    sc_signal<sc_uint<7>> wb_cmd_resp_crc7_calc;
    sc_signal<sc_uint<15>> wb_cmd_resp_spistatus;
    sc_signal<bool> w_cmd_resp_ready;
    sc_signal<sc_uint<4>> wb_trx_cmdstate;
    sc_signal<sc_uint<4>> wb_trx_cmderr;
    sc_signal<bool> w_clear_cmderr;
    sc_signal<bool> w_400kHz_ena;
    sc_signal<bool> w_crc7_clear;
    sc_signal<bool> w_crc7_next;
    sc_signal<bool> w_crc7_dat;
    sc_signal<sc_uint<7>> wb_crc7;
    sc_signal<bool> w_crc16_next;
    sc_signal<sc_uint<16>> wb_crc16_0;
    sc_signal<sc_uint<16>> wb_crc16_1;
    sc_signal<sc_uint<16>> wb_crc16_2;
    sc_signal<sc_uint<16>> wb_crc16_3;

    axi_slv *xslv0;
    sdctrl_regs *regs0;
    sdctrl_crc7 *crccmd0;
    sdctrl_crc16 *crcdat0;
    sdctrl_crc16 *crcdat1;
    sdctrl_crc16 *crcdat2;
    sdctrl_crc16 *crcdat3;
    sdctrl_cmd_transmitter *cmdtrx0;
    sdctrl_cache *cache0;

};

}  // namespace debugger

