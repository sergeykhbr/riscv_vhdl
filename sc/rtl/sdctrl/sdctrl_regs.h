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
#include "../ambalib/apb_slv.h"

namespace debugger {

SC_MODULE(sdctrl_regs) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_pmapinfo;                         // APB interconnect slot information
    sc_out<dev_config_type> o_pcfg;                         // APB sd-controller configuration registers descriptor
    sc_in<apb_in_type> i_apbi;                              // APB Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_sck;                                     // SD-card clock usually upto 50 MHz
    sc_out<bool> o_sck_posedge;                             // Strob just before positive edge
    sc_out<bool> o_sck_negedge;                             // Strob just before negative edge
    sc_out<sc_uint<16>> o_watchdog;                         // Number of sclk to detect no response
    sc_out<bool> o_err_clear;                               // Clear err from FW
    // Configuration parameters:
    sc_out<bool> o_spi_mode;                                // SPI mode was selected from FW
    sc_out<bool> o_pcie_12V_support;                        // 0b: not asking 1.2V support
    sc_out<bool> o_pcie_available;                          // 0b: not asking PCIe availability
    sc_out<sc_uint<4>> o_voltage_supply;                    // 0=not defined; 1=2.7-3.6V; 2=reserved for Low Voltage Range
    sc_out<sc_uint<8>> o_check_pattern;                     // Check pattern in CMD8 request
    sc_in<bool> i_400khz_ena;                               // Default frequency enabled in identification mode
    sc_in<sc_uint<3>> i_sdtype;                             // Ver1X or Ver2X standard or Ver2X high/extended capacity
    // Debug command state machine
    sc_in<bool> i_sd_cmd;
    sc_in<bool> i_sd_dat0;
    sc_in<bool> i_sd_dat1;
    sc_in<bool> i_sd_dat2;
    sc_in<bool> i_sd_dat3;
    sc_in<sc_uint<4>> i_err_code;
    sc_in<bool> i_cmd_req_valid;
    sc_in<sc_uint<6>> i_cmd_req_cmd;
    sc_in<bool> i_cmd_resp_valid;
    sc_in<sc_uint<6>> i_cmd_resp_cmd;
    sc_in<sc_uint<32>> i_cmd_resp_reg;
    sc_in<sc_uint<7>> i_cmd_resp_crc7_rx;
    sc_in<sc_uint<7>> i_cmd_resp_crc7_calc;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_regs);

    sdctrl_regs(sc_module_name name,
                bool async_reset);
    virtual ~sdctrl_regs();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct sdctrl_regs_registers {
        sc_signal<bool> sclk_ena;
        sc_signal<bool> spi_mode;
        sc_signal<bool> err_clear;
        sc_signal<sc_uint<24>> scaler_400khz;
        sc_signal<sc_uint<8>> scaler_data;
        sc_signal<sc_uint<32>> scaler_cnt;
        sc_signal<sc_uint<16>> wdog;
        sc_signal<sc_uint<16>> wdog_cnt;
        sc_signal<bool> level;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
        sc_signal<sc_uint<6>> last_req_cmd;
        sc_signal<sc_uint<6>> last_resp_cmd;
        sc_signal<sc_uint<7>> last_resp_crc7_rx;
        sc_signal<sc_uint<7>> last_resp_crc7_calc;
        sc_signal<sc_uint<32>> last_resp_reg;
        sc_signal<bool> pcie_12V_support;
        sc_signal<bool> pcie_available;
        sc_signal<sc_uint<4>> voltage_supply;
        sc_signal<sc_uint<8>> check_pattern;
    } v, r;

    void sdctrl_regs_r_reset(sdctrl_regs_registers &iv) {
        iv.sclk_ena = 0;
        iv.spi_mode = 0;
        iv.err_clear = 0;
        iv.scaler_400khz = 0;
        iv.scaler_data = 0;
        iv.scaler_cnt = 0;
        iv.wdog = 0x0FFF;
        iv.wdog_cnt = 0;
        iv.level = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
        iv.last_req_cmd = ~0ull;
        iv.last_resp_cmd = 0;
        iv.last_resp_crc7_rx = 0;
        iv.last_resp_crc7_calc = 0;
        iv.last_resp_reg = 0;
        iv.pcie_12V_support = 0;
        iv.pcie_available = 0;
        iv.voltage_supply = 0x1;
        iv.check_pattern = 0x55;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;

    apb_slv *pslv0;

};

}  // namespace debugger

