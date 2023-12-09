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

namespace debugger {

SC_MODULE(vip_sdcard_ctrl) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_spi_mode;
    sc_in<bool> i_cs;
    sc_in<bool> i_cmd_req_valid;
    sc_in<sc_uint<6>> i_cmd_req_cmd;
    sc_in<sc_uint<32>> i_cmd_req_data;
    sc_out<bool> o_cmd_req_ready;
    sc_out<bool> o_cmd_resp_valid;
    sc_out<sc_uint<32>> o_cmd_resp_data32;
    sc_in<bool> i_cmd_resp_ready;
    sc_out<bool> o_cmd_resp_r1b;
    sc_out<bool> o_cmd_resp_r2;
    sc_out<bool> o_cmd_resp_r3;
    sc_out<bool> o_cmd_resp_r7;
    sc_out<bool> o_stat_idle_state;
    sc_out<bool> o_stat_illegal_cmd;
    sc_out<sc_uint<41>> o_mem_addr;
    sc_in<sc_uint<8>> i_mem_rdata;
    sc_out<bool> o_crc16_clear;
    sc_out<bool> o_crc16_next;
    sc_in<sc_uint<16>> i_crc16;
    sc_out<bool> o_dat_trans;
    sc_out<sc_uint<4>> o_dat;
    sc_in<bool> i_cmdio_busy;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_sdcard_ctrl);

    vip_sdcard_ctrl(sc_module_name name,
                    bool async_reset,
                    int CFG_SDCARD_POWERUP_DONE_DELAY,
                    bool CFG_SDCARD_HCS,
                    sc_uint<4> CFG_SDCARD_VHS,
                    bool CFG_SDCARD_PCIE_1_2V,
                    bool CFG_SDCARD_PCIE_AVAIL,
                    sc_uint<24> CFG_SDCARD_VDD_VOLTAGE_WINDOW);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int CFG_SDCARD_POWERUP_DONE_DELAY_;
    bool CFG_SDCARD_HCS_;
    sc_uint<4> CFG_SDCARD_VHS_;
    bool CFG_SDCARD_PCIE_1_2V_;
    bool CFG_SDCARD_PCIE_AVAIL_;
    sc_uint<24> CFG_SDCARD_VDD_VOLTAGE_WINDOW_;

    // 
    // SD-card states (see Card Status[12:9] CURRENT_STATE on page 145)
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

    // Data block access state machine:
    static const uint8_t DATASTATE_IDLE = 0;
    static const uint8_t DATASTATE_START = 1;
    static const uint8_t DATASTATE_CRC15 = 2;
    static const uint8_t DATASTATE_STOP = 3;

    struct vip_sdcard_ctrl_registers {
        sc_signal<sc_uint<4>> sdstate;
        sc_signal<sc_uint<3>> datastate;
        sc_signal<sc_uint<32>> powerup_cnt;
        sc_signal<sc_uint<8>> preinit_cnt;
        sc_signal<sc_uint<32>> delay_cnt;
        sc_signal<bool> powerup_done;
        sc_signal<bool> cmd_req_ready;
        sc_signal<bool> cmd_resp_valid;
        sc_signal<bool> cmd_resp_valid_delayed;
        sc_signal<sc_uint<32>> cmd_resp_data32;
        sc_signal<bool> cmd_resp_r1b;
        sc_signal<bool> cmd_resp_r2;
        sc_signal<bool> cmd_resp_r3;
        sc_signal<bool> cmd_resp_r7;
        sc_signal<bool> illegal_cmd;
        sc_signal<bool> ocr_hcs;
        sc_signal<sc_uint<24>> ocr_vdd_window;
        sc_signal<bool> req_mem_valid;
        sc_signal<sc_uint<41>> req_mem_addr;
        sc_signal<sc_uint<16>> shiftdat;
        sc_signal<sc_uint<13>> bitcnt;
        sc_signal<bool> crc16_clear;
        sc_signal<bool> crc16_next;
        sc_signal<bool> dat_trans;
    } v, r;

    void vip_sdcard_ctrl_r_reset(vip_sdcard_ctrl_registers &iv) {
        iv.sdstate = SDSTATE_IDLE;
        iv.datastate = DATASTATE_IDLE;
        iv.powerup_cnt = 0;
        iv.preinit_cnt = 0;
        iv.delay_cnt = 0;
        iv.powerup_done = 0;
        iv.cmd_req_ready = 0;
        iv.cmd_resp_valid = 0;
        iv.cmd_resp_valid_delayed = 0;
        iv.cmd_resp_data32 = 0;
        iv.cmd_resp_r1b = 0;
        iv.cmd_resp_r2 = 0;
        iv.cmd_resp_r3 = 0;
        iv.cmd_resp_r7 = 0;
        iv.illegal_cmd = 0;
        iv.ocr_hcs = 0;
        iv.ocr_vdd_window = 0;
        iv.req_mem_valid = 0;
        iv.req_mem_addr = 0;
        iv.shiftdat = ~0ull;
        iv.bitcnt = 0;
        iv.crc16_clear = 0;
        iv.crc16_next = 0;
        iv.dat_trans = 0;
    }

};

}  // namespace debugger

