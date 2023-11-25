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
#include "sdctrl_crc7.h"

namespace debugger {

SC_MODULE(sdctrl_cmd_transmitter) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_sclk_posedge;
    sc_in<bool> i_sclk_negedge;
    sc_in<bool> i_cmd;
    sc_out<bool> o_cmd;
    sc_out<bool> o_cmd_dir;
    sc_out<bool> o_cmd_cs;
    sc_in<bool> i_spi_mode;                                 // SPI mode was selected by FW
    sc_in<sc_uint<4>> i_err_code;
    sc_in<bool> i_wdog_trigger;                             // Event from wdog timer
    sc_in<bool> i_cmd_set_low;                              // Set forcibly o_cmd output to LOW
    sc_in<bool> i_req_valid;
    sc_in<sc_uint<6>> i_req_cmd;
    sc_in<sc_uint<32>> i_req_arg;
    sc_in<sc_uint<3>> i_req_rn;                             // R1, R3,R6 or R2
    sc_out<bool> o_req_ready;
    sc_out<bool> o_resp_valid;
    sc_out<sc_uint<6>> o_resp_cmd;                          // Mirrored command
    sc_out<sc_uint<32>> o_resp_reg;                         // Card Status, OCR register (R3) or RCA register (R6)
    sc_out<sc_uint<7>> o_resp_crc7_rx;                      // Received CRC7
    sc_out<sc_uint<7>> o_resp_crc7_calc;                    // Calculated CRC7
    sc_out<sc_uint<15>> o_resp_spistatus;                   // {R1,R2} response valid only in SPI mode
    sc_in<bool> i_resp_ready;
    sc_out<bool> o_wdog_ena;
    sc_out<bool> o_err_valid;
    sc_out<sc_uint<4>> o_err_setcode;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_cmd_transmitter);

    sdctrl_cmd_transmitter(sc_module_name name,
                           bool async_reset);
    virtual ~sdctrl_cmd_transmitter();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // Command request states:
    static const uint8_t CMDSTATE_IDLE = 0;
    static const uint8_t CMDSTATE_REQ_CONTENT = 1;
    static const uint8_t CMDSTATE_REQ_CRC7 = 2;
    static const uint8_t CMDSTATE_REQ_STOPBIT = 3;
    static const uint8_t CMDSTATE_RESP_WAIT = 4;
    static const uint8_t CMDSTATE_RESP_TRANSBIT = 5;
    static const uint8_t CMDSTATE_RESP_CMD_MIRROR = 6;
    static const uint8_t CMDSTATE_RESP_REG = 7;
    static const uint8_t CMDSTATE_RESP_CID_CSD = 8;
    static const uint8_t CMDSTATE_RESP_CRC7 = 9;
    static const uint8_t CMDSTATE_RESP_STOPBIT = 10;
    static const uint8_t CMDSTATE_RESP_SPI_R1 = 11;
    static const uint8_t CMDSTATE_RESP_SPI_R2 = 12;
    static const uint8_t CMDSTATE_RESP_SPI_DATA = 13;
    static const uint8_t CMDSTATE_PAUSE = 15;

    struct sdctrl_cmd_transmitter_registers {
        sc_signal<sc_uint<6>> req_cmd;
        sc_signal<sc_uint<3>> req_rn;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<6>> resp_cmd;
        sc_signal<sc_uint<32>> resp_arg;
        sc_signal<sc_uint<15>> resp_spistatus;
        sc_signal<sc_uint<40>> cmdshift;
        sc_signal<sc_uint<6>> cmdmirror;
        sc_signal<sc_uint<32>> regshift;
        sc_signal<sc_biguint<120>> cidshift;
        sc_signal<sc_uint<7>> crc_calc;
        sc_signal<sc_uint<7>> crc_rx;
        sc_signal<sc_uint<7>> cmdbitcnt;
        sc_signal<bool> crc7_clear;
        sc_signal<sc_uint<4>> cmdstate;
        sc_signal<bool> err_valid;
        sc_signal<sc_uint<4>> err_setcode;
        sc_signal<bool> cmd_cs;
        sc_signal<bool> cmd_dir;
        sc_signal<bool> wdog_ena;
    } v, r;

    void sdctrl_cmd_transmitter_r_reset(sdctrl_cmd_transmitter_registers &iv) {
        iv.req_cmd = 0;
        iv.req_rn = 0;
        iv.resp_valid = 0;
        iv.resp_cmd = 0;
        iv.resp_arg = 0;
        iv.resp_spistatus = 0;
        iv.cmdshift = ~0ull;
        iv.cmdmirror = 0;
        iv.regshift = 0;
        iv.cidshift = 0;
        iv.crc_calc = 0;
        iv.crc_rx = 0;
        iv.cmdbitcnt = 0;
        iv.crc7_clear = 1;
        iv.cmdstate = CMDSTATE_IDLE;
        iv.err_valid = 0;
        iv.err_setcode = CMDERR_NONE;
        iv.cmd_cs = 1;
        iv.cmd_dir = 1;
        iv.wdog_ena = 0;
    }

    sc_signal<sc_uint<7>> wb_crc7;
    sc_signal<bool> w_crc7_next;
    sc_signal<bool> w_crc7_dat;

    sdctrl_crc7 *crc0;

};

}  // namespace debugger

