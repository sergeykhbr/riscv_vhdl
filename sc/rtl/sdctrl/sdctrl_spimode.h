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
#include "sdctrl_cfg.h"

namespace debugger {

SC_MODULE(sdctrl_spimode) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_posedge;                                  // SPI clock posedge pulse
    sc_in<bool> i_miso;                                     // SPI data input
    sc_out<bool> o_mosi;                                    // SPI master output slave input
    sc_out<bool> o_csn;                                     // Chip select active LOW
    sc_in<bool> i_detected;
    sc_in<bool> i_protect;
    sc_in<bool> i_cfg_pcie_12V_support;
    sc_in<bool> i_cfg_pcie_available;
    sc_in<sc_uint<4>> i_cfg_voltage_supply;
    sc_in<sc_uint<8>> i_cfg_check_pattern;
    sc_in<bool> i_cmd_req_ready;
    sc_out<bool> o_cmd_req_valid;
    sc_out<sc_uint<6>> o_cmd_req_cmd;
    sc_out<sc_uint<32>> o_cmd_req_arg;
    sc_out<sc_uint<3>> o_cmd_req_rn;
    sc_in<bool> i_cmd_resp_valid;
    sc_in<sc_uint<15>> i_cmd_resp_r1r2;
    sc_in<sc_uint<32>> i_cmd_resp_arg32;
    sc_out<bool> o_data_req_ready;
    sc_in<bool> i_data_req_valid;
    sc_in<bool> i_data_req_write;
    sc_in<sc_uint<CFG_SDCACHE_ADDR_BITS>> i_data_req_addr;
    sc_in<sc_biguint<512>> i_data_req_wdata;
    sc_out<bool> o_data_resp_valid;
    sc_out<sc_biguint<512>> o_data_resp_rdata;
    sc_in<sc_uint<16>> i_crc16_0;
    sc_out<bool> o_crc16_clear;
    sc_out<bool> o_crc16_next;
    sc_out<bool> o_wdog_ena;
    sc_in<bool> i_wdog_trigger;
    sc_in<sc_uint<4>> i_err_code;
    sc_out<bool> o_err_valid;
    sc_out<bool> o_err_clear;
    sc_out<sc_uint<4>> o_err_code;
    sc_out<bool> o_400khz_ena;
    sc_out<sc_uint<3>> o_sdtype;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_spimode);

    sdctrl_spimode(sc_module_name name,
                   bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // Controller state
    static const uint8_t STATE_CMD0 = 0;
    static const uint8_t STATE_CMD8 = 1;
    static const uint8_t STATE_CMD55 = 2;
    static const uint8_t STATE_ACMD41 = 3;
    static const uint8_t STATE_CMD58 = 4;
    static const uint8_t STATE_WAIT_DATA_REQ = 5;
    static const uint8_t STATE_CMD17_READ_SINGLE_BLOCK = 6;
    static const uint8_t STATE_CMD24_WRITE_SINGLE_BLOCK = 7;
    static const uint8_t STATE_WAIT_DATA_START = 8;
    static const uint8_t STATE_READING_DATA = 9;
    static const uint8_t STATE_READING_CRC15 = 10;
    static const uint8_t STATE_READING_END = 11;

    struct sdctrl_spimode_registers {
        sc_signal<sc_uint<7>> clkcnt;
        sc_signal<bool> cmd_req_valid;
        sc_signal<sc_uint<6>> cmd_req_cmd;
        sc_signal<sc_uint<32>> cmd_req_arg;
        sc_signal<sc_uint<3>> cmd_req_rn;
        sc_signal<sc_uint<6>> cmd_resp_cmd;
        sc_signal<sc_uint<32>> cmd_resp_arg32;
        sc_signal<sc_uint<7>> cmd_resp_r1;
        sc_signal<sc_uint<8>> cmd_resp_r2;
        sc_signal<sc_uint<32>> data_addr;
        sc_signal<sc_biguint<512>> data_data;
        sc_signal<bool> data_resp_valid;
        sc_signal<bool> wdog_ena;
        sc_signal<bool> crc16_clear;
        sc_signal<sc_uint<16>> crc16_calc0;
        sc_signal<sc_uint<16>> crc16_rx0;
        sc_signal<bool> dat_csn;
        sc_signal<bool> dat_reading;
        sc_signal<bool> err_clear;
        sc_signal<bool> err_valid;
        sc_signal<sc_uint<4>> err_code;
        sc_signal<bool> sck_400khz_ena;
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> wait_cmd_resp;
        sc_signal<sc_uint<3>> sdtype;
        sc_signal<bool> HCS;                                // High Capacity Support
        sc_signal<bool> S18;                                // 1.8V Low voltage
        sc_signal<sc_uint<24>> OCR_VoltageWindow;           // all ranges 2.7 to 3.6 V
        sc_signal<sc_uint<12>> bitcnt;
    } v, r;

    void sdctrl_spimode_r_reset(sdctrl_spimode_registers &iv) {
        iv.clkcnt = 0;
        iv.cmd_req_valid = 0;
        iv.cmd_req_cmd = 0;
        iv.cmd_req_arg = 0;
        iv.cmd_req_rn = 0;
        iv.cmd_resp_cmd = 0;
        iv.cmd_resp_arg32 = 0;
        iv.cmd_resp_r1 = 0;
        iv.cmd_resp_r2 = 0;
        iv.data_addr = 0;
        iv.data_data = 0;
        iv.data_resp_valid = 0;
        iv.wdog_ena = 0;
        iv.crc16_clear = 1;
        iv.crc16_calc0 = 0;
        iv.crc16_rx0 = 0;
        iv.dat_csn = 1;
        iv.dat_reading = 0;
        iv.err_clear = 0;
        iv.err_valid = 0;
        iv.err_code = 0;
        iv.sck_400khz_ena = 1;
        iv.state = STATE_CMD0;
        iv.wait_cmd_resp = 0;
        iv.sdtype = SDCARD_UNKNOWN;
        iv.HCS = 1;
        iv.S18 = 0;
        iv.OCR_VoltageWindow = 0xFF8000;
        iv.bitcnt = 0;
    }

};

}  // namespace debugger

