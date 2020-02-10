/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "l2_dst.h"

namespace debugger {

L2Destination::L2Destination(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_msg_type("i_msg_type"),
    i_msg_src("i_msg_src"),
    i_msg_payload("i_msg_payload"),
    i_l1o0("i_l1o0"),
    o_l1i0("o_l1i0"),
    i_l1o1("i_l1o1"),
    o_l1i1("o_l1i1"),
    i_l1o2("i_l1o2"),
    o_l1i2("o_l1i2"),
    i_l1o3("i_l1o3"),
    o_l1i3("o_l1i3"),
    i_acpo("i_acpo"),
    o_acpi("o_acpi"),
    o_eos("o_eos") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_l1o0;
    sensitive << i_l1o1;
    sensitive << i_l1o2;
    sensitive << i_l1o3;
    sensitive << i_acpo;
    sensitive << i_msg_type;
    sensitive << i_msg_src;
    sensitive << i_msg_payload;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

L2Destination::~L2Destination() {
}

void L2Destination::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_msg_type, i_msg_type.name());
        sc_trace(o_vcd, i_msg_src, i_msg_src.name());
        sc_trace(o_vcd, i_msg_payload, i_msg_payload.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
    }
}

void L2Destination::comb() {
    axi4_l1_in_type vlxi[4];
    axi4_master_in_type vacpi;
    bool v_eos;

    v = r;

    v_eos = 0;
    for (int i = 0; i < 4; i++) {
        vlxi[i] = axi4_l1_in_none;
    }
    vacpi = axi4_master_in_none;

    switch (r.state.read()) {
    case Idle:
        if (i_msg_type.read() == L2_MSG_AW_ACCEPT) {
            v.state = WriteMem;
            for (int i = 0; i < 5; i++) {
                if (i_msg_src.read()[i] == 1) {
                    if (i == 0) {
                        vacpi.aw_ready = 1;
                    } else {
                        vlxi[i-1].aw_ready = 1;
                    }
                }
            }
        } else if (i_msg_type.read() == L2_MSG_AR_ACCEPT) {
            v.state = ReadMem;
            for (int i = 0; i < 5; i++) {
                if (i_msg_src.read()[i] == 1) {
                    if (i == 0) {
                        vacpi.ar_ready = 1;
                    } else {
                        vlxi[i-1].ar_ready = 1;
                    }
                }
            }
        }
        break;
    case ReadMem:
        if (i_msg_type.read() == L2_MSG_R_DONE) {
            v_eos = 1;
            for (int i = 0; i < 5; i++) {
                if (i_msg_src.read()[i] == 1) {
                    if (i == 0) {
                        vacpi.r_valid = 1;
                        vacpi.r_last = 1;
                        vacpi.r_data = i_msg_payload.read()(L1CACHE_LINE_BITS-1, 0);
                        vacpi.r_resp = i_msg_payload.read()(L1CACHE_LINE_BITS+1, L1CACHE_LINE_BITS);
                    } else {
                        vlxi[i-1].r_valid = 1;
                        vlxi[i-1].r_last = 1;
                        vlxi[i-1].r_data = i_msg_payload.read()(L1CACHE_LINE_BITS-1, 0);
                        vlxi[i-1].r_resp = i_msg_payload.read()(L1CACHE_LINE_BITS+1, L1CACHE_LINE_BITS);
                    }
                }
            }
            v.state = Idle;          // ??? Always one clock. fixme
        }
        break;
    case WriteMem:
        if (i_msg_type.read() == L2_MSG_WB_DONE) {
            for (int i = 0; i < 5; i++) {
                if (i_msg_src.read()[i] == 1) {
                    if (i == 0) {
                        vacpi.w_ready = 1;
                    } else {
                        vlxi[i-1].w_ready = 1;
                    }
                }
            }
            v.state = WriteNoAck;
       } else if (i_msg_type.read() == L2_MSG_W_DONE) {
            for (int i = 0; i < 5; i++) {
                if (i_msg_src.read()[i] == 1) {
                    if (i == 0) {
                        vacpi.w_ready = 1;
                    } else {
                        vlxi[i-1].w_ready = 1;
                    }
                }
            }
            v.src = i_msg_src;
            v.state = WriteAck;
        }
        break;
    case WriteNoAck:
        for (int i = 0; i < 5; i++) {
            if (r.src.read()[i] == 1) {
                if (i == 0) {
                    vacpi.b_valid = 1;
                    vacpi.b_resp = 0;
                } else {
                    vlxi[i-1].b_valid = 1;
                    vlxi[i-1].b_resp = 0;
                }
            }
        }
        v_eos = 1;
        v.state = Idle;
        break;
    case WriteAck:
        if (i_msg_type.read() == L2_MSG_B_DONE) {
            v_eos = 1;
            for (int i = 0; i < 5; i++) {
                if (i_msg_src.read()[i] == 1) {
                    if (i == 0) {
                        vacpi.b_valid = 1;
                        vacpi.b_resp = i_msg_payload.read()(L1CACHE_LINE_BITS+1, L1CACHE_LINE_BITS);
                    } else {
                        vlxi[i-1].b_valid = 1;
                        vlxi[i-1].b_resp = i_msg_payload.read()(L1CACHE_LINE_BITS+1, L1CACHE_LINE_BITS);
                    }
                }
            }
            v.state = Idle;
        }
        break;
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_l1i0 = vlxi[0];
    o_l1i1 = vlxi[1];
    o_l1i2 = vlxi[2];
    o_l1i3 = vlxi[3];
    o_acpi = vacpi;
    o_eos = v_eos;
}

void L2Destination::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
