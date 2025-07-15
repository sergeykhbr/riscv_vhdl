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

SC_MODULE(vip_jtag_tap) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_tck;
    sc_in<bool> i_req_valid;                                // request
    sc_in<sc_uint<4>> i_req_irlen;                          // irlen in an range 1..15: ARM=4
    sc_in<sc_uint<16>> i_req_ir;                            // ir reg
    sc_in<sc_uint<7>> i_req_drlen;                          // drlen in an range 1..64: ARM=35/32
    sc_in<sc_uint<64>> i_req_dr;                            // dr reg
    sc_out<bool> o_resp_valid;                              // response valid
    sc_out<sc_uint<64>> o_resp_data;                        // response data
    sc_out<bool> o_trst;                                    // Must be open-train, pullup
    sc_out<bool> o_tck;
    sc_out<bool> o_tms;
    sc_out<bool> o_tdo;
    sc_in<bool> i_tdi;

    void comb();
    void registers();
    void rnegisters();

    vip_jtag_tap(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    // JTAG states:
    static const uint8_t RESET_TAP = 0;
    static const uint8_t IDLE = 1;
    static const uint8_t SELECT_DR_SCAN1 = 2;
    static const uint8_t SELECT_DR_SCAN = 3;
    static const uint8_t CAPTURE_DR = 4;
    static const uint8_t SHIFT_DR = 5;
    static const uint8_t EXIT1_DR = 6;
    static const uint8_t UPDATE_DR = 7;
    static const uint8_t SELECT_IR_SCAN = 8;
    static const uint8_t CAPTURE_IR = 9;
    static const uint8_t SHIFT_IR = 10;
    static const uint8_t EXIT1_IR = 11;
    static const uint8_t UPDATE_IR = 12;
    static const uint8_t INIT_RESET = 15;

    struct vip_jtag_tap_registers {
        sc_signal<bool> req_valid;
        sc_signal<sc_uint<4>> req_irlen;
        sc_signal<sc_uint<7>> req_drlen;
        sc_signal<sc_uint<16>> req_ir;
        sc_signal<sc_uint<64>> req_dr;
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<7>> dr_length;
        sc_signal<sc_uint<64>> dr;
        sc_signal<bool> bypass;
        sc_signal<sc_uint<32>> datacnt;
        sc_signal<sc_uint<64>> shiftreg;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<64>> resp_data;
        sc_signal<sc_uint<16>> ir;
    };

    void vip_jtag_tap_r_reset(vip_jtag_tap_registers& iv) {
        iv.req_valid = 0;
        iv.req_irlen = 0x1;
        iv.req_drlen = 0x01;
        iv.req_ir = 0;
        iv.req_dr = 0;
        iv.state = INIT_RESET;
        iv.dr_length = 0x7F;
        iv.dr = 0;
        iv.bypass = 0;
        iv.datacnt = 0;
        iv.shiftreg = 0;
        iv.resp_valid = 0;
        iv.resp_data = 0;
        iv.ir = ~0ull;
    }

    struct vip_jtag_tap_rnegisters {
        sc_signal<bool> trst;
        sc_signal<bool> tms;
        sc_signal<bool> tdo;
    };

    void vip_jtag_tap_rn_reset(vip_jtag_tap_rnegisters& iv) {
        iv.trst = 0;
        iv.tms = 1;
        iv.tdo = 0;
    }

    sc_signal<bool> w_tck;
    vip_jtag_tap_registers v;
    vip_jtag_tap_registers r;
    vip_jtag_tap_rnegisters vn;
    vip_jtag_tap_rnegisters rn;

};

}  // namespace debugger

