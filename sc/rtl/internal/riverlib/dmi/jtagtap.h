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
#include "api_core.h"

namespace debugger {

template<int abits = 7,
         int irlen = 5,
         uint32_t idcode = 0x10E31913>
SC_MODULE(jtagtap) {
 public:
    sc_in<bool> i_trst;                                     // Must be open-train, pullup
    sc_in<bool> i_tck;
    sc_in<bool> i_tms;
    sc_in<bool> i_tdi;
    sc_out<bool> o_tdo;
    sc_out<bool> o_dmi_req_valid;
    sc_out<bool> o_dmi_req_write;
    sc_out<sc_uint<7>> o_dmi_req_addr;
    sc_out<sc_uint<32>> o_dmi_req_data;
    sc_in<sc_uint<32>> i_dmi_resp_data;
    sc_in<bool> i_dmi_busy;
    sc_in<bool> i_dmi_error;
    sc_out<bool> o_dmi_hardreset;

    void comb();
    void rhegisters();
    void rnhegisters();

    jtagtap(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int drlen = ((abits + 32) + 2);

    static const uint8_t IR_IDCODE = 0x01;
    static const uint8_t IR_DTMCONTROL = 0x10;
    static const uint8_t IR_DBUS = 0x11;
    static const uint8_t IR_BYPASS = 0x1F;

    static const uint8_t DMISTAT_SUCCESS = 0x0;
    static const uint8_t DMISTAT_RESERVED = 0x1;
    static const uint8_t DMISTAT_FAILED = 0x2;
    static const uint8_t DMISTAT_BUSY = 0x3;

    // DTMCONTROL register bits
    static const int DTMCONTROL_DMIRESET = 16;
    static const int DTMCONTROL_DMIHARDRESET = 17;

    // JTAG states:
    static const uint8_t RESET_TAP = 0;
    static const uint8_t IDLE = 1;
    static const uint8_t SELECT_DR_SCAN = 2;
    static const uint8_t CAPTURE_DR = 3;
    static const uint8_t SHIFT_DR = 4;
    static const uint8_t EXIT1_DR = 5;
    static const uint8_t PAUSE_DR = 6;
    static const uint8_t EXIT2_DR = 7;
    static const uint8_t UPDATE_DR = 8;
    static const uint8_t SELECT_IR_SCAN = 9;
    static const uint8_t CAPTURE_IR = 10;
    static const uint8_t SHIFT_IR = 11;
    static const uint8_t EXIT1_IR = 12;
    static const uint8_t PAUSE_IR = 13;
    static const uint8_t EXIT2_IR = 14;
    static const uint8_t UPDATE_IR = 15;

    struct jtagtap_rhegisters {
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<7>> dr_length;
        sc_signal<sc_uint<drlen>> dr;
        sc_signal<bool> bypass;
        sc_signal<sc_uint<32>> datacnt;
        sc_signal<bool> dmi_busy;
        sc_signal<sc_uint<2>> err_sticky;
    };

    void jtagtap_rh_reset(jtagtap_rhegisters& iv) {
        iv.state = RESET_TAP;
        iv.dr_length = 0;
        iv.dr = idcode;
        iv.bypass = 0;
        iv.datacnt = 0;
        iv.dmi_busy = 0;
        iv.err_sticky = 0;
    }

    struct jtagtap_rnhegisters {
        sc_signal<sc_uint<irlen>> ir;
        sc_signal<sc_uint<abits>> dmi_addr;
    };

    void jtagtap_rnh_reset(jtagtap_rnhegisters& iv) {
        iv.ir = IR_IDCODE;
        iv.dmi_addr = 0;
    }

    jtagtap_rhegisters vh;
    jtagtap_rhegisters rh;
    jtagtap_rnhegisters vnh;
    jtagtap_rnhegisters rnh;

};

template<int abits, int irlen, uint32_t idcode>
jtagtap<abits, irlen, idcode>::jtagtap(sc_module_name name)
    : sc_module(name),
    i_trst("i_trst"),
    i_tck("i_tck"),
    i_tms("i_tms"),
    i_tdi("i_tdi"),
    o_tdo("o_tdo"),
    o_dmi_req_valid("o_dmi_req_valid"),
    o_dmi_req_write("o_dmi_req_write"),
    o_dmi_req_addr("o_dmi_req_addr"),
    o_dmi_req_data("o_dmi_req_data"),
    i_dmi_resp_data("i_dmi_resp_data"),
    i_dmi_busy("i_dmi_busy"),
    i_dmi_error("i_dmi_error"),
    o_dmi_hardreset("o_dmi_hardreset") {


    SC_METHOD(comb);
    sensitive << i_trst;
    sensitive << i_tck;
    sensitive << i_tms;
    sensitive << i_tdi;
    sensitive << i_dmi_resp_data;
    sensitive << i_dmi_busy;
    sensitive << i_dmi_error;
    sensitive << rh.state;
    sensitive << rh.dr_length;
    sensitive << rh.dr;
    sensitive << rh.bypass;
    sensitive << rh.datacnt;
    sensitive << rh.dmi_busy;
    sensitive << rh.err_sticky;
    sensitive << rnh.ir;
    sensitive << rnh.dmi_addr;

    SC_METHOD(rhegisters);
    sensitive << i_trst;
    sensitive << i_tck.pos();

    SC_METHOD(rnhegisters);
    sensitive << i_trst;
    sensitive << i_tck.neg();
}

template<int abits, int irlen, uint32_t idcode>
void jtagtap<abits, irlen, idcode>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_trst, i_trst.name());
        sc_trace(o_vcd, i_tck, i_tck.name());
        sc_trace(o_vcd, i_tms, i_tms.name());
        sc_trace(o_vcd, i_tdi, i_tdi.name());
        sc_trace(o_vcd, o_tdo, o_tdo.name());
        sc_trace(o_vcd, o_dmi_req_valid, o_dmi_req_valid.name());
        sc_trace(o_vcd, o_dmi_req_write, o_dmi_req_write.name());
        sc_trace(o_vcd, o_dmi_req_addr, o_dmi_req_addr.name());
        sc_trace(o_vcd, o_dmi_req_data, o_dmi_req_data.name());
        sc_trace(o_vcd, i_dmi_resp_data, i_dmi_resp_data.name());
        sc_trace(o_vcd, i_dmi_busy, i_dmi_busy.name());
        sc_trace(o_vcd, i_dmi_error, i_dmi_error.name());
        sc_trace(o_vcd, o_dmi_hardreset, o_dmi_hardreset.name());
        sc_trace(o_vcd, rh.state, pn + ".rh.state");
        sc_trace(o_vcd, rh.dr_length, pn + ".rh.dr_length");
        sc_trace(o_vcd, rh.dr, pn + ".rh.dr");
        sc_trace(o_vcd, rh.bypass, pn + ".rh.bypass");
        sc_trace(o_vcd, rh.datacnt, pn + ".rh.datacnt");
        sc_trace(o_vcd, rh.dmi_busy, pn + ".rh.dmi_busy");
        sc_trace(o_vcd, rh.err_sticky, pn + ".rh.err_sticky");
        sc_trace(o_vcd, rnh.ir, pn + ".rnh.ir");
        sc_trace(o_vcd, rnh.dmi_addr, pn + ".rnh.dmi_addr");
    }

}

template<int abits, int irlen, uint32_t idcode>
void jtagtap<abits, irlen, idcode>::comb() {
    sc_uint<drlen> vb_dr;
    bool v_dmi_req_valid;
    bool v_dmi_req_write;
    sc_uint<32> vb_dmi_req_data;
    sc_uint<abits> vb_dmi_req_addr;
    sc_uint<2> vb_err_sticky;
    bool v_dmi_hardreset;

    vnh = rnh;
    vh = rh;
    vb_dr = 0;
    v_dmi_req_valid = 0;
    v_dmi_req_write = 0;
    vb_dmi_req_data = 0;
    vb_dmi_req_addr = 0;
    vb_err_sticky = 0;
    v_dmi_hardreset = 0;

    vb_dr = rh.dr.read();
    vb_err_sticky = rh.err_sticky.read();

    switch (rh.state.read()) {
    case RESET_TAP:
        vnh.ir = IR_IDCODE;
        if (i_tms.read() == 1) {
            vh.state = RESET_TAP;
        } else {
            vh.state = IDLE;
        }
        break;
    case IDLE:
        if (i_tms.read() == 1) {
            vh.state = SELECT_DR_SCAN;
        } else {
            vh.state = IDLE;
        }
        break;
    case SELECT_DR_SCAN:
        if (i_tms.read() == 1) {
            vh.state = SELECT_IR_SCAN;
        } else {
            vh.state = CAPTURE_DR;
        }
        break;
    case CAPTURE_DR:
        if (i_tms.read() == 1) {
            vh.state = EXIT1_DR;
        } else {
            vh.state = SHIFT_DR;
        }
        if (rnh.ir.read() == IR_IDCODE) {
            vb_dr = idcode;
            vh.dr_length = 32;
        } else if (rnh.ir.read() == IR_DTMCONTROL) {
            vb_dr(31, 0) = 0;
            vb_dr(3, 0) = 0x1;                              // version
            vb_dr(9, 4) = abits;                            // the size of the address
            vb_dr(11, 10) = rh.err_sticky.read();
            vh.dr_length = 32;
        } else if (rnh.ir.read() == IR_DBUS) {
            if (i_dmi_error.read() == 1) {
                vb_err_sticky = DMISTAT_FAILED;
                vb_dr(1, 0) = DMISTAT_FAILED;
            } else {
                vb_dr(1, 0) = rh.err_sticky.read();
            }
            vb_dr(33, 2) = i_dmi_resp_data.read();
            vb_dr(((34 + abits) - 1), 34) = rnh.dmi_addr.read();
            vh.dr_length = (abits + 34);
        } else if (rnh.ir.read() == IR_BYPASS) {
            vb_dr[0] = rh.bypass.read();
            vh.dr_length = 1;
        }
        vh.datacnt = 0;
        break;
    case SHIFT_DR:
        if (i_tms.read() == 1) {
            vh.state = EXIT1_DR;
        } else {
            vh.state = SHIFT_DR;
        }
        if (rh.dr_length.read() > 1) {
            // For the bypass dr_length = 1
            vb_dr = (0, rh.dr.read()((drlen - 1), 1));
            vb_dr[(rh.dr_length.read().to_int() - 1)] = i_tdi.read();
        } else {
            vb_dr[0] = i_tdi.read();
        }
        vh.datacnt = (rh.datacnt.read() + 1);               // debug counter no need in rtl
        break;
    case EXIT1_DR:
        if (i_tms.read() == 1) {
            vh.state = UPDATE_DR;
        } else {
            vh.state = PAUSE_DR;
        }
        break;
    case PAUSE_DR:
        if (i_tms.read() == 1) {
            vh.state = EXIT2_DR;
        } else {
            vh.state = PAUSE_DR;
        }
        break;
    case EXIT2_DR:
        if (i_tms.read() == 1) {
            vh.state = UPDATE_DR;
        } else {
            vh.state = SHIFT_DR;
        }
        break;
    case UPDATE_DR:
        if (i_tms.read() == 1) {
            vh.state = SELECT_DR_SCAN;
        } else {
            vh.state = IDLE;
        }
        if (rnh.ir.read() == IR_DTMCONTROL) {
            v_dmi_hardreset = rh.dr.read()[DTMCONTROL_DMIHARDRESET];
            if (rh.dr.read()[DTMCONTROL_DMIRESET] == 1) {
                vb_err_sticky = DMISTAT_SUCCESS;
            }
        } else if (rnh.ir.read() == IR_BYPASS) {
            vh.bypass = rh.dr.read()[0];
        } else if (rnh.ir.read() == IR_DBUS) {
            if (rh.err_sticky.read() != DMISTAT_SUCCESS) {
                // This operation should never result in a busy or error response.
            } else if (rh.dmi_busy.read() == 1) {
                vb_err_sticky = DMISTAT_BUSY;
            } else {
                v_dmi_req_valid = rh.dr.read()(1, 0).or_reduce();
            }
            v_dmi_req_write = rh.dr.read()[1];
            vb_dmi_req_data = rh.dr.read()(33, 2);
            vb_dmi_req_addr = rh.dr.read()(((34 + abits) - 1), 34);

            vnh.dmi_addr = rh.dr.read()(((34 + abits) - 1), 34);
        }
        break;
    case SELECT_IR_SCAN:
        if (i_tms.read() == 1) {
            vh.state = RESET_TAP;
        } else {
            vh.state = CAPTURE_IR;
        }
        break;
    case CAPTURE_IR:
        if (i_tms.read() == 1) {
            vh.state = EXIT1_IR;
        } else {
            vh.state = SHIFT_IR;
        }
        vb_dr((irlen - 1), 2) = rnh.ir.read()((irlen - 1), 2);
        vb_dr(1, 0) = 0x1;
        break;
    case SHIFT_IR:
        if (i_tms.read() == 1) {
            vh.state = EXIT1_IR;
        } else {
            vh.state = SHIFT_IR;
        }
        vb_dr[(irlen - 1)] = i_tdi.read();
        vb_dr((irlen - 2), 0) = rh.dr.read()((irlen - 1), 1);
        break;
    case EXIT1_IR:
        if (i_tms.read() == 1) {
            vh.state = UPDATE_IR;
        } else {
            vh.state = PAUSE_IR;
        }
        break;
    case PAUSE_IR:
        if (i_tms.read() == 1) {
            vh.state = EXIT2_IR;
        } else {
            vh.state = PAUSE_IR;
        }
        break;
    case EXIT2_IR:
        if (i_tms.read() == 1) {
            vh.state = UPDATE_IR;
        } else {
            vh.state = SHIFT_IR;
        }
        break;
    case UPDATE_IR:
        if (i_tms.read() == 1) {
            vh.state = SELECT_DR_SCAN;
        } else {
            vh.state = IDLE;
        }
        vnh.ir = rh.dr.read()((irlen - 1), 0);
        break;
    default:
        break;
    }
    vh.dr = vb_dr;
    vh.dmi_busy = i_dmi_busy.read();
    vh.err_sticky = vb_err_sticky;

    o_tdo = rh.dr.read()[0];
    o_dmi_req_valid = v_dmi_req_valid;
    o_dmi_req_write = v_dmi_req_write;
    o_dmi_req_data = vb_dmi_req_data;
    o_dmi_req_addr = vb_dmi_req_addr;
    o_dmi_hardreset = v_dmi_hardreset;
}

template<int abits, int irlen, uint32_t idcode>
void jtagtap<abits, irlen, idcode>::rhegisters() {
    if (i_trst.read() == 1) {
        jtagtap_rh_reset(rh);
    } else {
        rh = vh;
    }
}

template<int abits, int irlen, uint32_t idcode>
void jtagtap<abits, irlen, idcode>::rnhegisters() {
    if (i_trst.read() == 1) {
        jtagtap_rnh_reset(rnh);
    } else {
        rnh = vnh;
    }
}

}  // namespace debugger

