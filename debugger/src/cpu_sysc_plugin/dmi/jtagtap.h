/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include "api_core.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(JtagTap) {

 public:
    sc_in<bool> i_trst; // Must be open-train, pullup
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
    sc_out<bool> o_dmi_reset;
    sc_out<bool> o_dmi_hardreset;

    void comb();
    void registers();
    void nregisters();

    SC_HAS_PROCESS(JtagTap);

    JtagTap(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    typedef enum {
        RESET_TAP,
        IDLE,
        SELECT_DR_SCAN,
        CAPTURE_DR,
        SHIFT_DR,
        EXIT1_DR,
        PAUSE_DR,
        EXIT2_DR,
        UPDATE_DR,
        SELECT_IR_SCAN,
        CAPTURE_IR,
        SHIFT_IR,
        EXIT1_IR,
        PAUSE_IR,
        EXIT2_IR,
        UPDATE_IR
    } jtag_state_t;

    const jtag_state_t next[16][2] = {
        /* TEST_LOGIC_RESET */    { IDLE, RESET_TAP },
        /* RUN_TEST_IDLE */       { IDLE, SELECT_DR_SCAN },
        /* SELECT_DR_SCAN */      { CAPTURE_DR, SELECT_IR_SCAN },
        /* CAPTURE_DR */          { SHIFT_DR, EXIT1_DR },
        /* SHIFT_DR */            { SHIFT_DR, EXIT1_DR },
        /* EXIT1_DR */            { PAUSE_DR, UPDATE_DR },
        /* PAUSE_DR */            { PAUSE_DR, EXIT2_DR },
        /* EXIT2_DR */            { SHIFT_DR, UPDATE_DR },
        /* UPDATE_DR */           { IDLE, SELECT_DR_SCAN },
        /* SELECT_IR_SCAN */      { CAPTURE_IR, RESET_TAP },
        /* CAPTURE_IR */          { SHIFT_IR, EXIT1_IR },
        /* SHIFT_IR */            { SHIFT_IR, EXIT1_IR },
        /* EXIT1_IR */            { PAUSE_IR, UPDATE_IR },
        /* PAUSE_IR */            { PAUSE_IR, EXIT2_IR },
        /* EXIT2_IR */            { SHIFT_IR, UPDATE_IR },
        /* UPDATE_IR */           { IDLE, SELECT_DR_SCAN }
    };

    enum {
      IR_IDCODE=1,
      IR_DTMCONTROL=0x10,
      IR_DBUS=0x11,
      IR_BYPASS=0x1f
    };

    static const int idcode = 0x10e31913;
    static const int abits = 7;
    static const int drlen = abits + 32 + 2;
    static const int irlen = 5;


    static const uint64_t DTMCONTROL_IDLE           = 12;
    static const uint64_t DTMCONTROL_DMIRESET       = 16;
    static const uint64_t DTMCONTROL_DMIHARDRESET   = 17;

    static const uint64_t DMISTAT_SUCCESS           = 0;
    static const uint64_t DMISTAT_RESERVED          = 1;
    static const uint64_t DMISTAT_FAILED            = 2;
    static const uint64_t DMISTAT_BUSY              = 3;

    static const uint64_t DMI_OP_NOP	            = 0;
    static const uint64_t DMI_OP_READ	            = 1;
    static const uint64_t DMI_OP_WRITE	            = 2;
    static const uint64_t DMI_OP_RESERVED	        = 3;


    struct RegistersType {
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> tck;
        sc_signal<bool> tms;
        sc_signal<bool> tdi;
        sc_signal<sc_uint<7>> dr_length;
        sc_signal<sc_uint<drlen>> dr;
        sc_signal<bool> bypass;
        sc_uint<32> datacnt;
    } r, v;

    struct NRegistersType {
        sc_signal<sc_uint<irlen>> ir;
        sc_signal<sc_uint<abits>> dmi_addr;
    } nr, nv;


    void R_RESET(RegistersType &iv) {
        iv.state = RESET_TAP;
        iv.tck = 0;
        iv.tdi = 0;
        iv.tms = 0;
        iv.dr_length = 0;
        iv.dr = idcode;
        iv.datacnt = 0;
    }
    void NR_RESET(NRegistersType &iv) {
        iv.ir = IR_IDCODE;
        iv.dmi_addr = 0;
    }
};

}  // namespace debugger

