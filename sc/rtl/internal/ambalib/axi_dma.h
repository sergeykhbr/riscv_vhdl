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
#include "types_amba.h"
#include "api_core.h"

namespace debugger {

template<int abits = 48>                                    // adress bits used
SC_MODULE(axi_dma) {
 public:
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_clk;                                      // CPU clock
    sc_out<bool> o_req_mem_ready;                           // Ready to accept next data
    sc_in<bool> i_req_mem_valid;                            // Request data is ready to accept
    sc_in<bool> i_req_mem_write;                            // 0=read; 1=write operation
    sc_in<sc_uint<12>> i_req_mem_bytes;                     // 0=4096 B; 4=DWORD; 8=QWORD; ...
    sc_in<sc_uint<abits>> i_req_mem_addr;                   // Address to read/write
    sc_in<sc_uint<8>> i_req_mem_strob;                      // Byte enabling write strob
    sc_in<sc_uint<64>> i_req_mem_data;                      // Data to write
    sc_in<bool> i_req_mem_last;                             // Last data payload in a sequence
    sc_out<bool> o_resp_mem_valid;                          // Read/Write data is valid. All write transaction with valid response.
    sc_out<bool> o_resp_mem_last;                           // Last response in a sequence.
    sc_out<bool> o_resp_mem_fault;                          // Error on memory access
    sc_out<sc_uint<abits>> o_resp_mem_addr;                 // Read address value
    sc_out<sc_uint<64>> o_resp_mem_data;                    // Read data value
    sc_in<bool> i_resp_mem_ready;                           // Ready to accept response
    sc_in<axi4_master_in_type> i_msti;                      // AXI master input
    sc_out<axi4_master_out_type> o_msto;                    // AXI master output
    sc_out<bool> o_dbg_valid;
    sc_out<sc_uint<64>> o_dbg_payload;

    void comb();
    void registers();

    axi_dma(sc_module_name name,
            bool async_reset,
            int userbits,
            sc_uint<64> base_offset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int userbits_;
    sc_uint<64> base_offset_;

    static const uint8_t state_idle = 0;
    static const uint8_t state_ar = 1;
    static const uint8_t state_r = 2;
    static const uint8_t state_r_wait_accept = 3;
    static const uint8_t state_aw = 4;
    static const uint8_t state_w = 5;
    static const uint8_t state_w_wait_accept = 6;
    static const uint8_t state_b = 7;

    struct axi_dma_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<bool> ar_valid;
        sc_signal<bool> aw_valid;
        sc_signal<bool> w_valid;
        sc_signal<bool> r_ready;
        sc_signal<bool> b_ready;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> req_wdata;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> req_wstrb;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<bool> req_last;
        sc_signal<bool> req_ready;
        sc_signal<bool> resp_valid;
        sc_signal<bool> resp_last;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> resp_addr;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> resp_data;
        sc_signal<bool> resp_error;
        sc_signal<sc_uint<CFG_SYSBUS_USER_BITS>> user_count;
        sc_signal<bool> dbg_valid;
        sc_signal<sc_uint<64>> dbg_payload;
    };

    void axi_dma_r_reset(axi_dma_registers& iv) {
        iv.state = state_idle;
        iv.ar_valid = 0;
        iv.aw_valid = 0;
        iv.w_valid = 0;
        iv.r_ready = 0;
        iv.b_ready = 0;
        iv.req_addr = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.req_len = 0;
        iv.req_last = 0;
        iv.req_ready = 1;
        iv.resp_valid = 0;
        iv.resp_last = 0;
        iv.resp_addr = 0;
        iv.resp_data = 0;
        iv.resp_error = 0;
        iv.user_count = 0;
        iv.dbg_valid = 0;
        iv.dbg_payload = 0;
    }

    axi_dma_registers v;
    axi_dma_registers r;

};

template<int abits>
axi_dma<abits>::axi_dma(sc_module_name name,
                        bool async_reset,
                        int userbits,
                        sc_uint<64> base_offset)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    o_req_mem_ready("o_req_mem_ready"),
    i_req_mem_valid("i_req_mem_valid"),
    i_req_mem_write("i_req_mem_write"),
    i_req_mem_bytes("i_req_mem_bytes"),
    i_req_mem_addr("i_req_mem_addr"),
    i_req_mem_strob("i_req_mem_strob"),
    i_req_mem_data("i_req_mem_data"),
    i_req_mem_last("i_req_mem_last"),
    o_resp_mem_valid("o_resp_mem_valid"),
    o_resp_mem_last("o_resp_mem_last"),
    o_resp_mem_fault("o_resp_mem_fault"),
    o_resp_mem_addr("o_resp_mem_addr"),
    o_resp_mem_data("o_resp_mem_data"),
    i_resp_mem_ready("i_resp_mem_ready"),
    i_msti("i_msti"),
    o_msto("o_msto"),
    o_dbg_valid("o_dbg_valid"),
    o_dbg_payload("o_dbg_payload") {

    async_reset_ = async_reset;
    userbits_ = userbits;
    base_offset_ = base_offset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_valid;
    sensitive << i_req_mem_write;
    sensitive << i_req_mem_bytes;
    sensitive << i_req_mem_addr;
    sensitive << i_req_mem_strob;
    sensitive << i_req_mem_data;
    sensitive << i_req_mem_last;
    sensitive << i_resp_mem_ready;
    sensitive << i_msti;
    sensitive << r.state;
    sensitive << r.ar_valid;
    sensitive << r.aw_valid;
    sensitive << r.w_valid;
    sensitive << r.r_ready;
    sensitive << r.b_ready;
    sensitive << r.req_addr;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_size;
    sensitive << r.req_len;
    sensitive << r.req_last;
    sensitive << r.req_ready;
    sensitive << r.resp_valid;
    sensitive << r.resp_last;
    sensitive << r.resp_addr;
    sensitive << r.resp_data;
    sensitive << r.resp_error;
    sensitive << r.user_count;
    sensitive << r.dbg_valid;
    sensitive << r.dbg_payload;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int abits>
void axi_dma<abits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, o_req_mem_ready, o_req_mem_ready.name());
        sc_trace(o_vcd, i_req_mem_valid, i_req_mem_valid.name());
        sc_trace(o_vcd, i_req_mem_write, i_req_mem_write.name());
        sc_trace(o_vcd, i_req_mem_bytes, i_req_mem_bytes.name());
        sc_trace(o_vcd, i_req_mem_addr, i_req_mem_addr.name());
        sc_trace(o_vcd, i_req_mem_strob, i_req_mem_strob.name());
        sc_trace(o_vcd, i_req_mem_data, i_req_mem_data.name());
        sc_trace(o_vcd, i_req_mem_last, i_req_mem_last.name());
        sc_trace(o_vcd, o_resp_mem_valid, o_resp_mem_valid.name());
        sc_trace(o_vcd, o_resp_mem_last, o_resp_mem_last.name());
        sc_trace(o_vcd, o_resp_mem_fault, o_resp_mem_fault.name());
        sc_trace(o_vcd, o_resp_mem_addr, o_resp_mem_addr.name());
        sc_trace(o_vcd, o_resp_mem_data, o_resp_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_ready, i_resp_mem_ready.name());
        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, o_dbg_valid, o_dbg_valid.name());
        sc_trace(o_vcd, o_dbg_payload, o_dbg_payload.name());
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.ar_valid, pn + ".r.ar_valid");
        sc_trace(o_vcd, r.aw_valid, pn + ".r.aw_valid");
        sc_trace(o_vcd, r.w_valid, pn + ".r.w_valid");
        sc_trace(o_vcd, r.r_ready, pn + ".r.r_ready");
        sc_trace(o_vcd, r.b_ready, pn + ".r.b_ready");
        sc_trace(o_vcd, r.req_addr, pn + ".r.req_addr");
        sc_trace(o_vcd, r.req_wdata, pn + ".r.req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r.req_wstrb");
        sc_trace(o_vcd, r.req_size, pn + ".r.req_size");
        sc_trace(o_vcd, r.req_len, pn + ".r.req_len");
        sc_trace(o_vcd, r.req_last, pn + ".r.req_last");
        sc_trace(o_vcd, r.req_ready, pn + ".r.req_ready");
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_last, pn + ".r.resp_last");
        sc_trace(o_vcd, r.resp_addr, pn + ".r.resp_addr");
        sc_trace(o_vcd, r.resp_data, pn + ".r.resp_data");
        sc_trace(o_vcd, r.resp_error, pn + ".r.resp_error");
        sc_trace(o_vcd, r.user_count, pn + ".r.user_count");
        sc_trace(o_vcd, r.dbg_valid, pn + ".r.dbg_valid");
        sc_trace(o_vcd, r.dbg_payload, pn + ".r.dbg_payload");
    }

}

template<int abits>
void axi_dma<abits>::comb() {
    sc_uint<12> vb_req_mem_bytes_m1;
    sc_uint<CFG_SYSBUS_ADDR_BITS> vb_req_addr_inc;
    sc_uint<CFG_SYSBUS_DATA_BITS> vb_r_data_swap;
    axi4_master_out_type vmsto;

    v = r;
    vb_req_mem_bytes_m1 = 0;
    vb_req_addr_inc = 0;
    vb_r_data_swap = 0;
    vmsto = axi4_master_out_none;

    vb_req_mem_bytes_m1 = (i_req_mem_bytes.read() - 1);
    vb_req_addr_inc = r.req_addr.read();

    // Byte swapping:
    if (r.req_size.read() == 0) {
        vb_req_addr_inc(11, 0) = (r.req_addr.read()(11, 0) + 0x001);
        if (r.req_addr.read()(2, 0) == 0) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(7, 0), i_msti.read().r_data(7, 0), i_msti.read().r_data(7, 0), i_msti.read().r_data(7, 0));
        } else if (r.req_addr.read()(2, 0) == 1) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(15, 8), i_msti.read().r_data(15, 8), i_msti.read().r_data(15, 8), i_msti.read().r_data(15, 8));
        } else if (r.req_addr.read()(2, 0) == 2) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(23, 16), i_msti.read().r_data(23, 16), i_msti.read().r_data(23, 16), i_msti.read().r_data(23, 16));
        } else if (r.req_addr.read()(2, 0) == 3) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(31, 24), i_msti.read().r_data(31, 24), i_msti.read().r_data(31, 24), i_msti.read().r_data(31, 24));
        } else if (r.req_addr.read()(2, 0) == 4) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(39, 32), i_msti.read().r_data(39, 32), i_msti.read().r_data(39, 32), i_msti.read().r_data(39, 32));
        } else if (r.req_addr.read()(2, 0) == 5) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(47, 40), i_msti.read().r_data(47, 40), i_msti.read().r_data(47, 40), i_msti.read().r_data(47, 40));
        } else if (r.req_addr.read()(2, 0) == 6) {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(55, 48), i_msti.read().r_data(55, 48), i_msti.read().r_data(55, 48), i_msti.read().r_data(55, 48));
        } else {
            vb_r_data_swap(31, 0) = (i_msti.read().r_data(63, 56), i_msti.read().r_data(63, 56), i_msti.read().r_data(63, 56), i_msti.read().r_data(63, 56));
        }
        vb_r_data_swap(63, 32) = vb_r_data_swap(31, 0);
    } else if (r.req_size.read() == 1) {
        vb_req_addr_inc(11, 0) = (r.req_addr.read()(11, 0) + 0x002);
        if (r.req_addr.read()(2, 1) == 0) {
            vb_r_data_swap = (i_msti.read().r_data(15, 0), i_msti.read().r_data(15, 0), i_msti.read().r_data(15, 0), i_msti.read().r_data(15, 0));
        } else if (r.req_addr.read()(2, 1) == 1) {
            vb_r_data_swap = (i_msti.read().r_data(31, 16), i_msti.read().r_data(31, 16), i_msti.read().r_data(31, 16), i_msti.read().r_data(31, 16));
        } else if (r.req_addr.read()(2, 1) == 2) {
            vb_r_data_swap = (i_msti.read().r_data(47, 32), i_msti.read().r_data(47, 32), i_msti.read().r_data(47, 32), i_msti.read().r_data(47, 32));
        } else {
            vb_r_data_swap = (i_msti.read().r_data(63, 48), i_msti.read().r_data(63, 48), i_msti.read().r_data(63, 48), i_msti.read().r_data(63, 48));
        }
    } else if (r.req_size.read() == 2) {
        vb_req_addr_inc(11, 0) = (r.req_addr.read()(11, 0) + 0x004);
        if (r.req_addr.read()[2] == 0) {
            vb_r_data_swap = (i_msti.read().r_data(31, 0), i_msti.read().r_data(31, 0));
        } else {
            vb_r_data_swap = (i_msti.read().r_data(63, 32), i_msti.read().r_data(63, 32));
        }
    } else {
        vb_req_addr_inc(11, 0) = (r.req_addr.read()(11, 0) + 0x008);
        vb_r_data_swap = i_msti.read().r_data;
    }

    v.dbg_valid = 0;
    switch (r.state.read()) {
    case state_idle:
        v.req_ready = 1;
        v.resp_valid = 0;
        v.resp_last = 0;
        if (i_req_mem_valid.read() == 1) {
            v.req_ready = 0;
            v.req_addr = (base_offset_((CFG_SYSBUS_ADDR_BITS - 1), abits), i_req_mem_addr.read());
            if (i_req_mem_bytes.read() == 1) {
                v.req_size = 0;
                v.req_len = 0;
            } else if (i_req_mem_bytes.read() == 2) {
                v.req_size = 1;
                v.req_len = 0;
            } else if (i_req_mem_bytes.read() == 4) {
                v.req_size = 2;
                v.req_len = 0;
            } else {
                v.req_size = 3;
                v.req_len = vb_req_mem_bytes_m1(10, 3);
            }
            if (i_req_mem_write.read() == 0) {
                v.ar_valid = 1;
                v.state = state_ar;
                v.req_wdata = 0;
                v.req_wstrb = 0;
                v.req_last = 0;
            } else {
                v.aw_valid = 1;
                v.w_valid = i_req_mem_last.read();          // Try to use AXI Lite
                v.req_wdata = i_req_mem_data.read();
                v.req_wstrb = i_req_mem_strob.read();
                v.req_last = i_req_mem_last.read();
                v.state = state_aw;
                v.dbg_valid = 1;
            }
            // debug interface:
            v.dbg_payload = (1,
                    i_req_mem_addr.read()(10, 0),
                    i_req_mem_bytes.read(),
                    i_req_mem_strob.read(),
                    i_req_mem_data.read()(31, 0));
        }
        break;
    case state_ar:
        if (i_msti.read().ar_ready == 1) {
            v.resp_addr = r.req_addr.read();
            v.ar_valid = 0;
            v.r_ready = 1;
            v.state = state_r;
        }
        break;
    case state_r:
        if (i_msti.read().r_valid == 1) {
            v.resp_valid = 1;
            v.resp_addr = r.req_addr.read();
            v.resp_data = vb_r_data_swap;
            v.resp_last = i_msti.read().r_last;
            v.resp_error = i_msti.read().r_resp[1];
            v.req_addr = vb_req_addr_inc;

            if ((i_resp_mem_ready.read() == 0) || (i_msti.read().r_last == 1)) {
                v.r_ready = 0;
                v.state = state_r_wait_accept;
            }
        }
        break;
    case state_r_wait_accept:
        if (i_resp_mem_ready.read() == 1) {
            v.resp_valid = 0;
            // debug interface:
            v.dbg_valid = 1;
            v.dbg_payload = (0, r.dbg_payload.read()(62, 32), r.resp_data.read()(31, 0));

            if (r.resp_last.read() == 1) {
                v.resp_last = 0;
                v.user_count = (r.user_count.read() + 1);
                v.req_ready = 1;
                v.state = state_idle;
            } else {
                v.r_ready = 1;
                v.state = state_r;
            }
        }
        break;

    case state_aw:
        if (i_msti.read().aw_ready == 1) {
            v.aw_valid = 0;
            v.state = state_w;
            v.resp_addr = r.req_addr.read();

            if (r.w_valid.read() && (i_msti.read().w_ready == 1)) {
                // AXI Lite accepted
                v.w_valid = 0;
                v.b_ready = i_resp_mem_ready.read();
                v.state = state_b;
            } else {
                v.w_valid = 1;
                v.req_ready = (!r.req_last.read());
                v.state = state_w;
            }
        }
        break;
    case state_w:
        if (i_msti.read().w_ready == 1) {
            // Burst write:
            v.w_valid = i_req_mem_valid.read();
            v.req_last = i_req_mem_last.read();
            v.req_wstrb = i_req_mem_strob.read();
            v.req_wdata = i_req_mem_data.read();
            v.req_addr = vb_req_addr_inc;
            if (r.req_last.read() == 1) {
                v.req_last = 0;
                v.w_valid = 0;
                v.b_ready = i_resp_mem_ready.read();
                v.req_ready = 0;
                v.state = state_b;
            }
        } else if (r.w_valid.read() == 1) {
            v.req_ready = 0;
            v.state = state_w_wait_accept;
        }
        break;
    case state_w_wait_accept:
        if (i_msti.read().w_ready == 1) {
            v.w_valid = 0;
            if (r.req_last.read() == 1) {
                v.req_last = 0;
                v.b_ready = i_resp_mem_ready.read();
                v.state = state_b;
            } else {
                v.req_ready = 1;
                v.state = state_w;
            }
        }
        break;
    case state_b:
        v.b_ready = i_resp_mem_ready.read();
        if ((r.b_ready.read() == 1) && (i_msti.read().b_valid == 1)) {
            v.b_ready = 0;
            v.state = state_idle;
            v.user_count = (r.user_count.read() + 1);
            v.resp_error = i_msti.read().b_resp[1];
            v.resp_valid = 1;
            v.resp_last = 1;
        }
        break;
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        axi_dma_r_reset(v);
    }

    o_resp_mem_valid = r.resp_valid.read();
    o_resp_mem_last = r.resp_last.read();
    o_resp_mem_fault = r.resp_error.read();
    o_resp_mem_addr = r.resp_addr.read();
    o_resp_mem_data = r.resp_data.read();
    o_req_mem_ready = r.req_ready.read();

    vmsto.ar_valid = r.ar_valid.read();
    vmsto.ar_bits.addr = r.req_addr.read();
    vmsto.ar_bits.size = r.req_size.read();
    vmsto.ar_bits.len = r.req_len.read();
    vmsto.ar_user = r.user_count.read();
    vmsto.ar_bits.burst = AXI_BURST_INCR;
    vmsto.r_ready = r.r_ready.read();

    vmsto.aw_valid = r.aw_valid.read();
    vmsto.aw_bits.addr = r.req_addr.read();
    vmsto.aw_bits.size = r.req_size.read();
    vmsto.aw_bits.len = r.req_len.read();
    vmsto.aw_user = r.user_count.read();
    vmsto.aw_bits.burst = AXI_BURST_INCR;
    vmsto.w_valid = r.w_valid.read();
    vmsto.w_last = r.req_last.read();
    vmsto.w_data = r.req_wdata.read();
    vmsto.w_strb = r.req_wstrb.read();
    vmsto.w_user = r.user_count.read();
    vmsto.b_ready = r.b_ready.read();

    o_msto = vmsto;
    o_dbg_valid = r.dbg_valid.read();
    o_dbg_payload = r.dbg_payload.read();
}

template<int abits>
void axi_dma<abits>::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        axi_dma_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

