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
    sc_in<bool> i_cmd_req_valid;
    sc_in<sc_uint<6>> i_cmd_req_cmd;
    sc_in<sc_uint<32>> i_cmd_req_data;
    sc_out<bool> o_cmd_req_ready;
    sc_out<bool> o_cmd_resp_valid;
    sc_out<sc_uint<32>> o_cmd_resp_data32;
    sc_in<bool> i_cmd_resp_ready;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_sdcard_ctrl);

    vip_sdcard_ctrl(sc_module_name name,
                    bool async_reset,
                    int CFG_SDCARD_POWERUP_DONE_DELAY,
                    sc_uint<4> CFG_SDCARD_VHS,
                    bool CFG_SDCARD_PCIE_1_2V,
                    bool CFG_SDCARD_PCIE_AVAIL,
                    sc_uint<24> CFG_SDCARD_VDD_VOLTAGE_WINDOW);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int CFG_SDCARD_POWERUP_DONE_DELAY_;
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

    struct vip_sdcard_ctrl_registers {
        sc_signal<sc_uint<4>> sdstate;
        sc_signal<sc_uint<32>> powerup_cnt;
        sc_signal<sc_uint<8>> preinit_cnt;
        sc_signal<sc_uint<32>> delay_cnt;
        sc_signal<bool> powerup_done;
        sc_signal<bool> cmd_req_ready;
        sc_signal<bool> cmd_resp_valid;
        sc_signal<bool> cmd_resp_valid_delayed;
        sc_signal<sc_uint<32>> cmd_resp_data32;
    } v, r;

    void vip_sdcard_ctrl_r_reset(vip_sdcard_ctrl_registers &iv) {
        iv.sdstate = SDSTATE_IDLE;
        iv.powerup_cnt = 0;
        iv.preinit_cnt = 0;
        iv.delay_cnt = 0;
        iv.powerup_done = 0;
        iv.cmd_req_ready = 0;
        iv.cmd_resp_valid = 0;
        iv.cmd_resp_valid_delayed = 0;
        iv.cmd_resp_data32 = 0;
    }

};

}  // namespace debugger

