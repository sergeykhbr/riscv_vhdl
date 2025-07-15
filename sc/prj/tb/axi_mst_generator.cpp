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

#include "axi_mst_generator.h"
#include "api_core.h"

namespace debugger {

axi_mst_generator::axi_mst_generator(sc_module_name name,
                                     sc_uint<48> req_bar,
                                     sc_uint<4> unique_id,
                                     sc_uint<64> read_compare,
                                     bool read_only,
                                     bool burst_disable)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_xmst("i_xmst"),
    o_xmst("o_xmst"),
    i_start_test("i_start_test"),
    i_test_selector("i_test_selector"),
    i_show_result("i_show_result"),
    o_writing("o_writing"),
    o_reading("o_reading") {

    req_bar_ = req_bar;
    unique_id_ = unique_id;
    read_compare_ = read_compare;
    read_only_ = read_only;
    burst_disable_ = burst_disable;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xmst;
    sensitive << i_start_test;
    sensitive << i_test_selector;
    sensitive << i_show_result;
    sensitive << r.err_cnt;
    sensitive << r.compare_cnt;
    sensitive << r.run_cnt;
    sensitive << r.state;
    sensitive << r.xsize;
    sensitive << r.aw_wait_cnt;
    sensitive << r.aw_valid;
    sensitive << r.aw_addr;
    sensitive << r.aw_unmap;
    sensitive << r.aw_xlen;
    sensitive << r.w_use_axi_light;
    sensitive << r.w_wait_states;
    sensitive << r.w_wait_cnt;
    sensitive << r.w_valid;
    sensitive << r.w_data;
    sensitive << r.w_strb;
    sensitive << r.w_last;
    sensitive << r.w_burst_cnt;
    sensitive << r.b_wait_states;
    sensitive << r.b_wait_cnt;
    sensitive << r.b_ready;
    sensitive << r.ar_wait_cnt;
    sensitive << r.ar_valid;
    sensitive << r.ar_addr;
    sensitive << r.ar_unmap;
    sensitive << r.ar_xlen;
    sensitive << r.r_wait_states;
    sensitive << r.r_wait_cnt;
    sensitive << r.r_ready;
    sensitive << r.r_burst_cnt;
    sensitive << r.compare_ena;
    sensitive << r.compare_a;
    sensitive << r.compare_b;

    SC_METHOD(test);
    sensitive << i_clk.pos();

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void axi_mst_generator::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xmst, i_xmst.name());
        sc_trace(o_vcd, o_xmst, o_xmst.name());
        sc_trace(o_vcd, i_start_test, i_start_test.name());
        sc_trace(o_vcd, i_test_selector, i_test_selector.name());
        sc_trace(o_vcd, i_show_result, i_show_result.name());
        sc_trace(o_vcd, o_writing, o_writing.name());
        sc_trace(o_vcd, o_reading, o_reading.name());
        sc_trace(o_vcd, r.err_cnt, pn + ".r.err_cnt");
        sc_trace(o_vcd, r.compare_cnt, pn + ".r.compare_cnt");
        sc_trace(o_vcd, r.run_cnt, pn + ".r.run_cnt");
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.xsize, pn + ".r.xsize");
        sc_trace(o_vcd, r.aw_wait_cnt, pn + ".r.aw_wait_cnt");
        sc_trace(o_vcd, r.aw_valid, pn + ".r.aw_valid");
        sc_trace(o_vcd, r.aw_addr, pn + ".r.aw_addr");
        sc_trace(o_vcd, r.aw_unmap, pn + ".r.aw_unmap");
        sc_trace(o_vcd, r.aw_xlen, pn + ".r.aw_xlen");
        sc_trace(o_vcd, r.w_use_axi_light, pn + ".r.w_use_axi_light");
        sc_trace(o_vcd, r.w_wait_states, pn + ".r.w_wait_states");
        sc_trace(o_vcd, r.w_wait_cnt, pn + ".r.w_wait_cnt");
        sc_trace(o_vcd, r.w_valid, pn + ".r.w_valid");
        sc_trace(o_vcd, r.w_data, pn + ".r.w_data");
        sc_trace(o_vcd, r.w_strb, pn + ".r.w_strb");
        sc_trace(o_vcd, r.w_last, pn + ".r.w_last");
        sc_trace(o_vcd, r.w_burst_cnt, pn + ".r.w_burst_cnt");
        sc_trace(o_vcd, r.b_wait_states, pn + ".r.b_wait_states");
        sc_trace(o_vcd, r.b_wait_cnt, pn + ".r.b_wait_cnt");
        sc_trace(o_vcd, r.b_ready, pn + ".r.b_ready");
        sc_trace(o_vcd, r.ar_wait_cnt, pn + ".r.ar_wait_cnt");
        sc_trace(o_vcd, r.ar_valid, pn + ".r.ar_valid");
        sc_trace(o_vcd, r.ar_addr, pn + ".r.ar_addr");
        sc_trace(o_vcd, r.ar_unmap, pn + ".r.ar_unmap");
        sc_trace(o_vcd, r.ar_xlen, pn + ".r.ar_xlen");
        sc_trace(o_vcd, r.r_wait_states, pn + ".r.r_wait_states");
        sc_trace(o_vcd, r.r_wait_cnt, pn + ".r.r_wait_cnt");
        sc_trace(o_vcd, r.r_ready, pn + ".r.r_ready");
        sc_trace(o_vcd, r.r_burst_cnt, pn + ".r.r_burst_cnt");
        sc_trace(o_vcd, r.compare_ena, pn + ".r.compare_ena");
        sc_trace(o_vcd, r.compare_a, pn + ".r.compare_a");
        sc_trace(o_vcd, r.compare_b, pn + ".r.compare_b");
    }

}

void axi_mst_generator::comb() {
    axi4_master_out_type vb_xmsto;
    sc_uint<48> vb_bar;
    sc_uint<4> vb_w_burst_cnt_next;
    sc_uint<32> vb_run_cnt_inv;
    bool v_writing;
    bool v_reading;

    v = r;
    vb_bar = 0;
    vb_w_burst_cnt_next = 0;
    vb_run_cnt_inv = 0;
    v_writing = 0;
    v_reading = 0;

    vb_run_cnt_inv = (~r.run_cnt.read());
    vb_w_burst_cnt_next = (r.w_burst_cnt.read() + 1);
    vb_bar = req_bar_;
    v.compare_ena = 0;

    // AXI master request state machines
    switch (r.state.read()) {
    case 0:
        if (i_start_test.read() == 1) {
            if (read_only_ == 1) {
                if (i_test_selector.read()(1, 0).or_reduce() == 0) {
                    v.state = 5;                            // ar_request
                } else {
                    v.state = 14;                           // wait to ar_request
                }
            } else {
                if (i_test_selector.read()(1, 0).or_reduce() == 0) {
                    v.state = 1;                            // aw_request
                } else {
                    v.state = 13;                           // wait to aw_request
                }
            }
            v.run_cnt = (r.run_cnt.read() + 1);
            v.aw_unmap = 0;
            v.ar_unmap = i_test_selector.read()[0];
            v.aw_wait_cnt = i_test_selector.read()(1, 0);
            v.ar_wait_cnt = i_test_selector.read()(1, 0);
            v.w_wait_states = i_test_selector.read()(4, 2);
            v.b_wait_states = i_test_selector.read()(6, 5);
            v.r_wait_states = i_test_selector.read()(9, 7);
            if (burst_disable_ == 0) {
                v.aw_xlen = (0, i_test_selector.read()(11, 10));
                v.ar_xlen = (0, i_test_selector.read()(11, 10));
            } else {
                v.aw_xlen = 0;
                v.ar_xlen = 0;
            }
            if ((i_test_selector.read()(11, 10).or_reduce() == 0) && (i_test_selector.read()(4, 2) == 7)) {
                v.w_use_axi_light = 1;
            } else {
                v.w_use_axi_light = 0;
            }
            v.xsize = 3;                                    // 8-bytes
            if (i_test_selector.read()[12] == 1) {
                v.xsize = 2;                                // 4-bytes
            }
        }
        break;
    case 13:                                                // wait to aw request
        if (r.aw_wait_cnt.read().or_reduce() == 1) {
            v.aw_wait_cnt = (r.aw_wait_cnt.read() - 1);
        } else {
            v.state = 1;                                    // aw_request
        }
        break;
    case 14:                                                // wait to ar request
        if (r.ar_wait_cnt.read().or_reduce() == 1) {
            v.ar_wait_cnt = (r.ar_wait_cnt.read() - 1);
        } else {
            v.state = 5;                                    // ar_request
        }
        break;
    case 1:                                                 // aw request
        v_writing = 1;
        v.aw_valid = 1;
        if (r.aw_unmap.read() == 0) {
            v.aw_addr = (vb_bar + (r.run_cnt.read()(6, 0) << 5));
        } else {
            v.aw_addr = 0xFFFFFFFFFC00;
        }
        v.w_burst_cnt = 0;
        if (r.w_use_axi_light.read() == 1) {
            v.w_valid = 1;
            v.w_last = 1;
            v.w_data = (unique_id_, vb_run_cnt_inv(27, 0), r.run_cnt.read()(27, 0), r.w_burst_cnt.read());
            v.w_strb = 0xFF;
        }
        if ((r.aw_valid.read() == 1) && (i_xmst.read().aw_ready == 1)) {
            v.aw_valid = 0;
            v.w_data = (unique_id_, vb_run_cnt_inv(27, 0), r.run_cnt.read()(27, 0), r.w_burst_cnt.read());
            v.w_strb = 0xFF;
            if ((r.w_valid.read() == 1) && (i_xmst.read().w_ready == 1)) {
                v.w_valid = 0;
                v.w_last = 0;
                if (r.b_wait_states.read().or_reduce() == 0) {
                    v.b_wait_cnt = 0;
                    v.b_ready = 1;
                } else {
                    v.b_wait_cnt = r.b_wait_states.read();
                }
                v.state = 4;
            } else if ((r.w_wait_states.read().or_reduce() == 0) || (r.aw_xlen.read().or_reduce() == 1) || (r.w_valid.read() == 1)) {
                // 1. Generate first w_valid just after aw in burst transaction to check buffering
                // 2. Cannot inject waits for AXI Light requests
                v.w_wait_cnt = 0;
                v.w_valid = 1;
                v.w_last = (!r.aw_xlen.read().or_reduce());
                v.state = 3;
            } else {
                v.state = 2;
                v.w_wait_cnt = r.w_wait_states.read();
            }
        }
        break;
    case 2:                                                 // w wait request
        v_writing = 1;
        if (r.w_wait_cnt.read().or_reduce() == 1) {
            v.w_wait_cnt = (r.w_wait_cnt.read() - 1);
        } else {
            v.w_valid = 1;
            v.w_last = (!r.aw_xlen.read().or_reduce());
            v.state = 3;
        }
        break;
    case 3:                                                 // w request
        v_writing = 1;
        v.w_valid = 1;
        v.w_data = (unique_id_, vb_run_cnt_inv(27, 0), r.run_cnt.read()(27, 0), r.w_burst_cnt.read());
        if ((r.w_valid.read() == 1) && (i_xmst.read().w_ready == 1)) {
            v.w_burst_cnt = vb_w_burst_cnt_next;
            v.w_data = (unique_id_, vb_run_cnt_inv(27, 0), r.run_cnt.read()(27, 0), vb_w_burst_cnt_next);
            v.w_valid = 0;
            v.w_last = 0;
            v.w_wait_cnt = r.w_wait_states.read();
            if (r.aw_xlen.read().or_reduce() == 1) {
                v.aw_xlen = (r.aw_xlen.read() - 1);
                if (r.w_wait_states.read().or_reduce() == 0) {
                    v.w_valid = 1;
                    v.w_last = (!r.aw_xlen.read()(7, 1).or_reduce());
                } else {
                    v.state = 2;
                }
            } else {
                if (r.b_wait_states.read().or_reduce() == 0) {
                    v.b_wait_cnt = 0;
                    v.b_ready = 1;
                } else {
                    v.b_wait_cnt = r.b_wait_states.read();
                }
                v.state = 4;
            }
        }
        break;
    case 4:                                                 // b response
        v_writing = 1;
        v.w_burst_cnt = 0;
        if (r.b_wait_cnt.read().or_reduce() == 1) {
            v.b_wait_cnt = (r.b_wait_cnt.read() - 1);
        } else {
            v.b_ready = 1;
            if ((r.b_ready.read() == 1) && (i_xmst.read().b_valid == 1)) {
                v.b_ready = 0;
                v.state = 5;
                v.ar_valid = 1;
                if (r.ar_unmap.read() == 0) {
                    v.ar_addr = (vb_bar + (r.run_cnt.read()(6, 0) << 5));
                } else {
                    v.ar_addr = 0xFFFFFFFFFC00;
                }
            }
        }
        break;
    case 5:                                                 // ar request
        v_reading = 1;
        v.ar_valid = 1;
        if (r.ar_unmap.read() == 0) {
            v.ar_addr = (vb_bar + (r.run_cnt.read()(6, 0) << 5));
        } else {
            v.ar_addr = 0xFFFFFFFFFC00;
        }
        if ((r.ar_valid.read() == 1) && (i_xmst.read().ar_ready == 1)) {
            v.ar_valid = 0;
            v.r_burst_cnt = 0;
            if ((r.r_wait_states.read().or_reduce() == 0) || (r.ar_xlen.read().or_reduce() == 1)) {
                v.r_wait_cnt = 0;
                v.r_ready = 1;
                v.state = 7;
            } else {
                v.state = 6;
                v.r_wait_cnt = r.r_wait_states.read();
            }
        }
        break;
    case 6:
        v_reading = 1;
        if (r.r_wait_cnt.read().or_reduce() == 1) {
            v.r_wait_cnt = (r.r_wait_cnt.read() - 1);
        } else {
            v.r_ready = 1;
            v.state = 7;
        }
        break;
    case 7:                                                 // r response
        v_reading = 1;
        v.r_ready = 1;
        if ((r.r_ready.read() == 1) && (i_xmst.read().r_valid == 1)) {
            v.r_burst_cnt = (r.r_burst_cnt.read() + 1);
            v.r_ready = 0;
            v.compare_ena = 1;
            v.compare_a = i_xmst.read().r_data;
            if (r.ar_unmap.read() == 0) {
                if (read_only_ == 0) {
                    v.compare_b = (unique_id_, vb_run_cnt_inv(27, 0), r.run_cnt.read()(27, 0), r.r_burst_cnt.read());
                } else {
                    v.compare_b = read_compare_;
                }
            } else {
                v.compare_b = 0xFFFFFFFFFFFFFFFF;
            }
            if (i_xmst.read().r_last == 1) {
                // Goto idle
                v.state = 0;
            } else {
                if (r.r_wait_states.read().or_reduce() == 0) {
                    v.r_ready = 1;
                } else {
                    v.r_wait_cnt = r.r_wait_states.read();
                    v.state = 6;
                }
            }
        }
        break;
    case 15:                                                // do nothing
        break;
    }

    if (r.compare_ena.read() == 1) {
        v.compare_cnt = (r.compare_cnt.read() + 1);
        if (r.compare_a.read() != r.compare_b.read()) {
            v.err_cnt = (r.err_cnt.read() + 1);
        }
    }

    vb_xmsto.ar_valid = r.ar_valid.read();
    vb_xmsto.ar_bits.addr = r.ar_addr.read();
    vb_xmsto.ar_bits.len = r.ar_xlen.read();
    vb_xmsto.ar_bits.size = r.xsize.read();
    vb_xmsto.ar_bits.burst = 1;
    vb_xmsto.ar_bits.lock = 0;
    vb_xmsto.ar_bits.cache = 0;
    vb_xmsto.ar_bits.prot = 0;
    vb_xmsto.ar_bits.qos = 0;
    vb_xmsto.ar_bits.region = 0;
    vb_xmsto.ar_id = ~0ull;
    vb_xmsto.ar_user = ~0ull;
    vb_xmsto.aw_valid = r.aw_valid.read();
    vb_xmsto.aw_bits.addr = r.aw_addr.read();
    vb_xmsto.aw_bits.len = r.aw_xlen.read();
    vb_xmsto.aw_bits.size = r.xsize.read();
    vb_xmsto.aw_bits.burst = 1;
    vb_xmsto.aw_bits.lock = 0;
    vb_xmsto.aw_bits.cache = 0;
    vb_xmsto.aw_bits.prot = 0;
    vb_xmsto.aw_bits.qos = 0;
    vb_xmsto.aw_bits.region = 0;
    vb_xmsto.aw_id = ~0ull;
    vb_xmsto.aw_user = ~0ull;
    vb_xmsto.w_valid = r.w_valid.read();
    vb_xmsto.w_data = r.w_data.read();
    vb_xmsto.w_last = r.w_last.read();
    vb_xmsto.w_strb = r.w_strb.read();
    vb_xmsto.w_user = ~0ull;
    vb_xmsto.b_ready = r.b_ready.read();
    vb_xmsto.r_ready = r.r_ready.read();
    o_xmst = vb_xmsto;
    o_writing = v_writing;
    o_reading = v_reading;
}

void axi_mst_generator::test() {
    if (r.compare_ena.read() == 1) {
        if (r.compare_a.read() != r.compare_b.read()) {
            std::cout << "@" << sc_time_stamp() << " + error: "
                      << std::hex << r.compare_a.read() << " != "
                      << std::hex << r.compare_b.read() << std::endl;
        }
    }
    if (i_show_result.read() == 1) {
        if (r.err_cnt.read() == 0) {
            std::cout << "@" << sc_time_stamp() << " No errors. "
                      << r.compare_cnt.read() << " TESTS PASSED" << std::endl;
        } else {
            std::cout << "@" << sc_time_stamp() << " TESTS FAILED. Total errors = "
                      << r.err_cnt.read() << std::endl;
        }
    }
}

void axi_mst_generator::registers() {
    if (i_nrst.read() == 0) {
        axi_mst_generator_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

