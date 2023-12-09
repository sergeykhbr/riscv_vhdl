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

SC_MODULE(sdctrl_sdmode) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_posedge;                                  // SPI clock posedge pulse
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
    sc_out<bool> o_dat3;                                    // Data Line[3] output; CS output in SPI mode
    sc_out<bool> o_dat3_dir;                                // Direction bit: 1=input; 0=output
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
    sc_in<sc_uint<6>> i_cmd_resp_cmd;
    sc_in<sc_uint<32>> i_cmd_resp_arg32;
    sc_out<bool> o_data_req_ready;
    sc_in<bool> i_data_req_valid;
    sc_in<bool> i_data_req_write;
    sc_in<sc_uint<CFG_SDCACHE_ADDR_BITS>> i_data_req_addr;
    sc_in<sc_biguint<512>> i_data_req_wdata;
    sc_out<bool> o_data_resp_valid;
    sc_out<sc_biguint<512>> o_data_resp_rdata;
    sc_in<sc_uint<16>> i_crc16_0;
    sc_in<sc_uint<16>> i_crc16_1;
    sc_in<sc_uint<16>> i_crc16_2;
    sc_in<sc_uint<16>> i_crc16_3;
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

    SC_HAS_PROCESS(sdctrl_sdmode);

    sdctrl_sdmode(sc_module_name name,
                  bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

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
    static const uint8_t SDSTATE_INA = 9;
    // SD-card 'idle' state substates:
    static const uint8_t IDLESTATE_CMD0 = 0;
    static const uint8_t IDLESTATE_CMD8 = 1;
    static const uint8_t IDLESTATE_CMD55 = 2;
    static const uint8_t IDLESTATE_ACMD41 = 3;
    static const uint8_t IDLESTATE_CARD_IDENTIFICATION = 5;
    // SD-card 'ready' state substates:
    static const uint8_t READYSTATE_CMD11 = 0;
    static const uint8_t READYSTATE_CMD2 = 1;
    static const uint8_t READYSTATE_CHECK_CID = 2;
    // SD-card 'ident' state substates:
    static const bool IDENTSTATE_CMD3 = 0;
    static const bool IDENTSTATE_CHECK_RCA = 1;

    struct sdctrl_sdmode_registers {
        sc_signal<sc_uint<7>> clkcnt;
        sc_signal<bool> cmd_req_valid;
        sc_signal<sc_uint<6>> cmd_req_cmd;
        sc_signal<sc_uint<32>> cmd_req_arg;
        sc_signal<sc_uint<3>> cmd_req_rn;
        sc_signal<sc_uint<6>> cmd_resp_cmd;
        sc_signal<sc_uint<32>> cmd_resp_arg32;
        sc_signal<sc_uint<32>> data_addr;
        sc_signal<sc_biguint<512>> data_data;
        sc_signal<bool> data_resp_valid;
        sc_signal<bool> wdog_ena;
        sc_signal<bool> crc16_clear;
        sc_signal<sc_uint<16>> crc16_calc0;
        sc_signal<sc_uint<16>> crc16_calc1;
        sc_signal<sc_uint<16>> crc16_calc2;
        sc_signal<sc_uint<16>> crc16_calc3;
        sc_signal<sc_uint<16>> crc16_rx0;
        sc_signal<sc_uint<16>> crc16_rx1;
        sc_signal<sc_uint<16>> crc16_rx2;
        sc_signal<sc_uint<16>> crc16_rx3;
        sc_signal<bool> dat_full_ena;
        sc_signal<bool> dat_csn;
        sc_signal<bool> err_clear;
        sc_signal<bool> err_valid;
        sc_signal<sc_uint<4>> err_code;
        sc_signal<bool> sck_400khz_ena;
        sc_signal<sc_uint<4>> sdstate;
        sc_signal<sc_uint<3>> initstate;
        sc_signal<sc_uint<2>> readystate;
        sc_signal<bool> identstate;
        sc_signal<bool> wait_cmd_resp;
        sc_signal<sc_uint<3>> sdtype;
        sc_signal<bool> HCS;                                // High Capacity Support
        sc_signal<bool> S18;                                // 1.8V Low voltage
        sc_signal<sc_uint<24>> OCR_VoltageWindow;           // all ranges 2.7 to 3.6 V
        sc_signal<sc_uint<12>> bitcnt;
    } v, r;

    void sdctrl_sdmode_r_reset(sdctrl_sdmode_registers &iv) {
        iv.clkcnt = 0;
        iv.cmd_req_valid = 0;
        iv.cmd_req_cmd = 0;
        iv.cmd_req_arg = 0;
        iv.cmd_req_rn = 0;
        iv.cmd_resp_cmd = 0;
        iv.cmd_resp_arg32 = 0;
        iv.data_addr = 0;
        iv.data_data = 0;
        iv.data_resp_valid = 0;
        iv.wdog_ena = 0;
        iv.crc16_clear = 1;
        iv.crc16_calc0 = 0;
        iv.crc16_calc1 = 0;
        iv.crc16_calc2 = 0;
        iv.crc16_calc3 = 0;
        iv.crc16_rx0 = 0;
        iv.crc16_rx1 = 0;
        iv.crc16_rx2 = 0;
        iv.crc16_rx3 = 0;
        iv.dat_full_ena = 0;
        iv.dat_csn = 1;
        iv.err_clear = 0;
        iv.err_valid = 0;
        iv.err_code = 0;
        iv.sck_400khz_ena = 1;
        iv.sdstate = SDSTATE_IDLE;
        iv.initstate = IDLESTATE_CMD0;
        iv.readystate = READYSTATE_CMD11;
        iv.identstate = IDENTSTATE_CMD3;
        iv.wait_cmd_resp = 0;
        iv.sdtype = SDCARD_UNKNOWN;
        iv.HCS = 1;
        iv.S18 = 0;
        iv.OCR_VoltageWindow = 0xFF8000;
        iv.bitcnt = 0;
    }

};

}  // namespace debugger

