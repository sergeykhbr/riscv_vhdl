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
#include "vip_sdcard_crc7.h"

namespace debugger {

SC_MODULE(vip_sdcard_cmdio) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_cmd;
    sc_out<bool> o_cmd;
    sc_out<bool> o_cmd_dir;
    sc_out<bool> o_cmd_req_valid;
    sc_out<sc_uint<6>> o_cmd_req_cmd;
    sc_out<sc_uint<32>> o_cmd_req_data;
    sc_in<bool> i_cmd_req_ready;
    sc_in<bool> i_cmd_resp_valid;
    sc_in<sc_uint<32>> i_cmd_resp_data32;
    sc_out<bool> o_cmd_resp_ready;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_sdcard_cmdio);

    vip_sdcard_cmdio(sc_module_name name,
                     bool async_reset);
    virtual ~vip_sdcard_cmdio();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // 
    // Receiver CMD state:
    static const uint8_t CMDSTATE_IDLE = 0;
    static const uint8_t CMDSTATE_REQ_STARTBIT = 1;
    static const uint8_t CMDSTATE_REQ_CMD = 2;
    static const uint8_t CMDSTATE_REQ_ARG = 3;
    static const uint8_t CMDSTATE_REQ_CRC7 = 4;
    static const uint8_t CMDSTATE_REQ_STOPBIT = 5;
    static const uint8_t CMDSTATE_REQ_VALID = 6;
    static const uint8_t CMDSTATE_WAIT_RESP = 7;
    static const uint8_t CMDSTATE_RESP = 8;
    static const uint8_t CMDSTATE_RESP_CRC7 = 9;

    struct vip_sdcard_cmdio_registers {
        sc_signal<bool> cmdz;
        sc_signal<bool> cmd_dir;
        sc_signal<sc_uint<48>> cmd_rxshift;
        sc_signal<sc_uint<48>> cmd_txshift;
        sc_signal<sc_uint<4>> cmd_state;
        sc_signal<sc_uint<6>> bitcnt;
        sc_signal<bool> cmd_req_valid;
        sc_signal<sc_uint<6>> cmd_req_cmd;
        sc_signal<sc_uint<32>> cmd_req_data;
        sc_signal<bool> cmd_resp_ready;
    } v, r;

    void vip_sdcard_cmdio_r_reset(vip_sdcard_cmdio_registers &iv) {
        iv.cmdz = 1;
        iv.cmd_dir = 1;
        iv.cmd_rxshift = ~0ull;
        iv.cmd_txshift = ~0ull;
        iv.cmd_state = CMDSTATE_IDLE;
        iv.bitcnt = 0;
        iv.cmd_req_valid = 0;
        iv.cmd_req_cmd = 0;
        iv.cmd_req_data = 0;
        iv.cmd_resp_ready = 0;
    }

    sc_signal<bool> w_cmd_out;
    sc_signal<bool> w_crc7_clear;
    sc_signal<bool> w_crc7_next;
    sc_signal<bool> w_crc7_dat;
    sc_signal<sc_uint<7>> wb_crc7;

    vip_sdcard_crc7 *crccmd0;

};

}  // namespace debugger

