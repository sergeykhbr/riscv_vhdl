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
    static const uint8_t SDSTATE_IDLE = 0;
    static const uint8_t SDSTATE_READY = 1;
    static const uint8_t SDSTATE_IDENT = 2;
    static const uint8_t SDSTATE_STBY = 3;
    static const uint8_t SDSTATE_TRAN = 4;
    static const uint8_t SDSTATE_DATA = 5;
    static const uint8_t SDSTATE_RCV = 6;
    static const uint8_t SDSTATE_PRG = 7;
    static const uint8_t SDSTATE_DIS = 8;
    // SD-card initalization state:
    static const uint8_t INITSTATE_CMD0 = 0;
    static const uint8_t INITSTATE_CMD8 = 1;
    static const uint8_t INITSTATE_ACMD41 = 2;
    static const uint8_t INITSTATE_CMD11 = 3;
    static const uint8_t INITSTATE_CMD2 = 4;
    static const uint8_t INITSTATE_CMD3 = 5;
    static const uint8_t INITSTATE_WAIT_RESP = 6;
    static const uint8_t INITSTATE_ERROR = 7;
    static const uint8_t INITSTATE_DONE = 8;

    struct sdctrl_registers {
        sc_signal<bool> cmd_req_ena;
        sc_signal<sc_uint<6>> cmd_req_cmd;
        sc_signal<sc_uint<32>> cmd_req_arg;
        sc_signal<sc_uint<3>> cmd_req_rn;
        sc_signal<sc_uint<6>> cmd_resp_r1;
        sc_signal<sc_uint<32>> cmd_resp_reg;
        sc_signal<bool> crc16_clear;
        sc_signal<sc_uint<4>> dat;
        sc_signal<bool> dat_dir;
        sc_signal<sc_uint<4>> sdstate;
        sc_signal<sc_uint<4>> initstate;
        sc_signal<sc_uint<4>> initstate_next;
    } v, r;

    void sdctrl_r_reset(sdctrl_registers &iv) {
        iv.cmd_req_ena = 0;
        iv.cmd_req_cmd = 0;
        iv.cmd_req_arg = 0;
        iv.cmd_req_rn = 0;
        iv.cmd_resp_r1 = 0;
        iv.cmd_resp_reg = 0;
        iv.crc16_clear = 1;
        iv.dat = ~0ul;
        iv.dat_dir = DIR_INPUT;
        iv.sdstate = SDSTATE_IDLE;
        iv.initstate = INITSTATE_CMD0;
        iv.initstate_next = INITSTATE_CMD0;
    }

    sc_signal<bool> w_regs_sck_posedge;
    sc_signal<bool> w_regs_sck;
    sc_signal<bool> w_regs_clear_cmderr;
    sc_signal<sc_uint<16>> wb_regs_watchdog;
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
    sc_signal<bool> w_mem_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_mem_resp_rdata;
    sc_signal<bool> wb_mem_resp_err;
    sc_signal<bool> w_cmd_req_ready;
    sc_signal<bool> w_cmd_resp_valid;
    sc_signal<sc_uint<6>> wb_cmd_resp_cmd;
    sc_signal<sc_uint<32>> wb_cmd_resp_reg;
    sc_signal<sc_uint<7>> wb_cmd_resp_crc7_rx;
    sc_signal<sc_uint<7>> wb_cmd_resp_crc7_calc;
    sc_signal<bool> w_cmd_resp_ready;
    sc_signal<sc_uint<4>> wb_cmdstate;
    sc_signal<sc_uint<4>> wb_cmderr;
    sc_signal<bool> w_crc7_clear;
    sc_signal<bool> w_crc7_next;
    sc_signal<bool> w_crc7_dat;
    sc_signal<sc_uint<7>> wb_crc7;
    sc_signal<bool> w_crc16_next;
    sc_signal<sc_uint<4>> wb_crc16_dat;
    sc_signal<sc_uint<16>> wb_crc16;

    axi_slv *xslv0;
    sdctrl_regs *regs0;
    sdctrl_crc7 *crccmd0;
    sdctrl_crc16 *crcdat0;
    sdctrl_cmd_transmitter *cmdtrx0;

};

}  // namespace debugger

