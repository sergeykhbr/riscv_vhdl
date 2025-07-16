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
#include "../ambalib/types_amba.h"
#include "river_cfg.h"
#include "types_river.h"
#include "api_core.h"

namespace debugger {

template<int abits = 48>                                    // adress bits used
SC_MODULE(l1_dma_snoop) {
 public:
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_clk;                                      // CPU clock
    sc_out<bool> o_req_mem_ready;                           // Ready to accept next data
    sc_in<bool> i_req_mem_path;                             // instruction/data
    sc_in<bool> i_req_mem_valid;                            // Request data is ready to accept
    sc_in<sc_uint<REQ_MEM_TYPE_BITS>> i_req_mem_type;       // read/write/cached
    sc_in<sc_uint<3>> i_req_mem_size;
    sc_in<sc_uint<abits>> i_req_mem_addr;                   // Address to read/write
    sc_in<sc_uint<L1CACHE_BYTES_PER_LINE>> i_req_mem_strob; // Byte enabling write strob
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_req_mem_data;    // Data to write
    sc_out<bool> o_resp_mem_path;                           // instruction/data.
    sc_out<bool> o_resp_mem_valid;                          // Read/Write data is valid. All write transaction with valid response.
    sc_out<bool> o_resp_mem_load_fault;                     // Error on memory access
    sc_out<bool> o_resp_mem_store_fault;                    // Error on memory access
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_mem_data;  // Read data value
    // D$ Snoop interface
    sc_out<bool> o_req_snoop_valid;
    sc_out<sc_uint<SNOOP_REQ_TYPE_BITS>> o_req_snoop_type;
    sc_in<bool> i_req_snoop_ready;
    sc_out<sc_uint<abits>> o_req_snoop_addr;
    sc_out<bool> o_resp_snoop_ready;
    sc_in<bool> i_resp_snoop_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_snoop_data;
    sc_in<sc_uint<DTAG_FL_TOTAL>> i_resp_snoop_flags;
    sc_in<axi4_l1_in_type> i_msti;                          // L1-cache master input
    sc_out<axi4_l1_out_type> o_msto;                        // L1-cache master output

    void comb();
    void registers();

    l1_dma_snoop(sc_module_name name,
                 bool async_reset,
                 int userbits,
                 sc_uint<64> base_offset,
                 bool coherence_ena);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int userbits_;
    sc_uint<64> base_offset_;
    bool coherence_ena_;

    static const uint8_t state_idle = 0;
    static const uint8_t state_ar = 1;
    static const uint8_t state_r = 2;
    static const uint8_t state_aw = 3;
    static const uint8_t state_w = 4;
    static const uint8_t state_b = 5;
    static const uint8_t snoop_idle = 0;
    static const uint8_t snoop_ac_wait_accept = 1;
    static const uint8_t snoop_cr = 2;
    static const uint8_t snoop_cr_wait_accept = 3;
    static const uint8_t snoop_cd = 4;
    static const uint8_t snoop_cd_wait_accept = 5;

    struct l1_dma_snoop_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<abits>> req_addr;
        sc_signal<bool> req_path;
        sc_signal<sc_uint<4>> req_cached;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<sc_uint<4>> req_ar_snoop;
        sc_signal<sc_uint<3>> req_aw_snoop;
        sc_signal<sc_uint<3>> snoop_state;
        sc_signal<sc_uint<abits>> ac_addr;
        sc_signal<sc_uint<4>> ac_snoop;                     // Table C3-19
        sc_signal<sc_uint<5>> cr_resp;
        sc_signal<sc_uint<SNOOP_REQ_TYPE_BITS>> req_snoop_type;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_snoop_data;
        sc_signal<bool> cache_access;
    };

    void l1_dma_snoop_r_reset(l1_dma_snoop_registers& iv) {
        iv.state = state_idle;
        iv.req_addr = 0;
        iv.req_path = 0;
        iv.req_cached = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_ar_snoop = 0;
        iv.req_aw_snoop = 0;
        iv.snoop_state = snoop_idle;
        iv.ac_addr = 0;
        iv.ac_snoop = 0;
        iv.cr_resp = 0;
        iv.req_snoop_type = 0;
        iv.resp_snoop_data = 0;
        iv.cache_access = 0;
    }

    sc_uint<4> reqtype2arsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype);
    sc_uint<4> reqtype2awsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype);

    l1_dma_snoop_registers v;
    l1_dma_snoop_registers r;

};

template<int abits>
l1_dma_snoop<abits>::l1_dma_snoop(sc_module_name name,
                                  bool async_reset,
                                  int userbits,
                                  sc_uint<64> base_offset,
                                  bool coherence_ena)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    o_req_mem_ready("o_req_mem_ready"),
    i_req_mem_path("i_req_mem_path"),
    i_req_mem_valid("i_req_mem_valid"),
    i_req_mem_type("i_req_mem_type"),
    i_req_mem_size("i_req_mem_size"),
    i_req_mem_addr("i_req_mem_addr"),
    i_req_mem_strob("i_req_mem_strob"),
    i_req_mem_data("i_req_mem_data"),
    o_resp_mem_path("o_resp_mem_path"),
    o_resp_mem_valid("o_resp_mem_valid"),
    o_resp_mem_load_fault("o_resp_mem_load_fault"),
    o_resp_mem_store_fault("o_resp_mem_store_fault"),
    o_resp_mem_data("o_resp_mem_data"),
    o_req_snoop_valid("o_req_snoop_valid"),
    o_req_snoop_type("o_req_snoop_type"),
    i_req_snoop_ready("i_req_snoop_ready"),
    o_req_snoop_addr("o_req_snoop_addr"),
    o_resp_snoop_ready("o_resp_snoop_ready"),
    i_resp_snoop_valid("i_resp_snoop_valid"),
    i_resp_snoop_data("i_resp_snoop_data"),
    i_resp_snoop_flags("i_resp_snoop_flags"),
    i_msti("i_msti"),
    o_msto("o_msto") {

    async_reset_ = async_reset;
    userbits_ = userbits;
    base_offset_ = base_offset;
    coherence_ena_ = coherence_ena;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_path;
    sensitive << i_req_mem_valid;
    sensitive << i_req_mem_type;
    sensitive << i_req_mem_size;
    sensitive << i_req_mem_addr;
    sensitive << i_req_mem_strob;
    sensitive << i_req_mem_data;
    sensitive << i_req_snoop_ready;
    sensitive << i_resp_snoop_valid;
    sensitive << i_resp_snoop_data;
    sensitive << i_resp_snoop_flags;
    sensitive << i_msti;
    sensitive << r.state;
    sensitive << r.req_addr;
    sensitive << r.req_path;
    sensitive << r.req_cached;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_size;
    sensitive << r.req_prot;
    sensitive << r.req_ar_snoop;
    sensitive << r.req_aw_snoop;
    sensitive << r.snoop_state;
    sensitive << r.ac_addr;
    sensitive << r.ac_snoop;
    sensitive << r.cr_resp;
    sensitive << r.req_snoop_type;
    sensitive << r.resp_snoop_data;
    sensitive << r.cache_access;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int abits>
void l1_dma_snoop<abits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, o_req_mem_ready, o_req_mem_ready.name());
        sc_trace(o_vcd, i_req_mem_path, i_req_mem_path.name());
        sc_trace(o_vcd, i_req_mem_valid, i_req_mem_valid.name());
        sc_trace(o_vcd, i_req_mem_type, i_req_mem_type.name());
        sc_trace(o_vcd, i_req_mem_size, i_req_mem_size.name());
        sc_trace(o_vcd, i_req_mem_addr, i_req_mem_addr.name());
        sc_trace(o_vcd, i_req_mem_strob, i_req_mem_strob.name());
        sc_trace(o_vcd, i_req_mem_data, i_req_mem_data.name());
        sc_trace(o_vcd, o_resp_mem_path, o_resp_mem_path.name());
        sc_trace(o_vcd, o_resp_mem_valid, o_resp_mem_valid.name());
        sc_trace(o_vcd, o_resp_mem_load_fault, o_resp_mem_load_fault.name());
        sc_trace(o_vcd, o_resp_mem_store_fault, o_resp_mem_store_fault.name());
        sc_trace(o_vcd, o_resp_mem_data, o_resp_mem_data.name());
        sc_trace(o_vcd, o_req_snoop_valid, o_req_snoop_valid.name());
        sc_trace(o_vcd, o_req_snoop_type, o_req_snoop_type.name());
        sc_trace(o_vcd, i_req_snoop_ready, i_req_snoop_ready.name());
        sc_trace(o_vcd, o_req_snoop_addr, o_req_snoop_addr.name());
        sc_trace(o_vcd, o_resp_snoop_ready, o_resp_snoop_ready.name());
        sc_trace(o_vcd, i_resp_snoop_valid, i_resp_snoop_valid.name());
        sc_trace(o_vcd, i_resp_snoop_data, i_resp_snoop_data.name());
        sc_trace(o_vcd, i_resp_snoop_flags, i_resp_snoop_flags.name());
        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.req_addr, pn + ".r.req_addr");
        sc_trace(o_vcd, r.req_path, pn + ".r.req_path");
        sc_trace(o_vcd, r.req_cached, pn + ".r.req_cached");
        sc_trace(o_vcd, r.req_wdata, pn + ".r.req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r.req_wstrb");
        sc_trace(o_vcd, r.req_size, pn + ".r.req_size");
        sc_trace(o_vcd, r.req_prot, pn + ".r.req_prot");
        sc_trace(o_vcd, r.req_ar_snoop, pn + ".r.req_ar_snoop");
        sc_trace(o_vcd, r.req_aw_snoop, pn + ".r.req_aw_snoop");
        sc_trace(o_vcd, r.snoop_state, pn + ".r.snoop_state");
        sc_trace(o_vcd, r.ac_addr, pn + ".r.ac_addr");
        sc_trace(o_vcd, r.ac_snoop, pn + ".r.ac_snoop");
        sc_trace(o_vcd, r.cr_resp, pn + ".r.cr_resp");
        sc_trace(o_vcd, r.req_snoop_type, pn + ".r.req_snoop_type");
        sc_trace(o_vcd, r.resp_snoop_data, pn + ".r.resp_snoop_data");
        sc_trace(o_vcd, r.cache_access, pn + ".r.cache_access");
    }

}

template<int abits>
sc_uint<4> l1_dma_snoop<abits>::reqtype2arsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype) {
    sc_uint<4> ret;

    ret = 0;
    if (reqtype[REQ_MEM_TYPE_CACHED] == 0) {
        ret = ARSNOOP_READ_NO_SNOOP;
    } else {
        if (reqtype[REQ_MEM_TYPE_UNIQUE] == 0) {
            ret = ARSNOOP_READ_SHARED;
        } else {
            ret = ARSNOOP_READ_MAKE_UNIQUE;
        }
    }
    return ret;
}

template<int abits>
sc_uint<4> l1_dma_snoop<abits>::reqtype2awsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype) {
    sc_uint<4> ret;

    ret = 0;
    if (reqtype[REQ_MEM_TYPE_CACHED] == 0) {
        ret = AWSNOOP_WRITE_NO_SNOOP;
    } else {
        if (reqtype[REQ_MEM_TYPE_UNIQUE] == 0) {
            ret = AWSNOOP_WRITE_BACK;
        } else {
            ret = AWSNOOP_WRITE_LINE_UNIQUE;
        }
    }
    return ret;
}

template<int abits>
void l1_dma_snoop<abits>::comb() {
    bool v_resp_mem_valid;
    bool v_mem_er_load_fault;
    bool v_mem_er_store_fault;
    bool v_next_ready;
    axi4_l1_out_type vmsto;
    bool v_snoop_next_ready;
    bool req_snoop_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_req_snoop_addr;
    sc_uint<SNOOP_REQ_TYPE_BITS> vb_req_snoop_type;
    bool v_cr_valid;
    sc_uint<5> vb_cr_resp;
    bool v_cd_valid;
    sc_biguint<L1CACHE_LINE_BITS> vb_cd_data;

    v = r;
    v_resp_mem_valid = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = 0;
    v_next_ready = 0;
    vmsto = axi4_l1_out_none;
    v_snoop_next_ready = 0;
    req_snoop_valid = 0;
    vb_req_snoop_addr = 0;
    vb_req_snoop_type = 0;
    v_cr_valid = 0;
    vb_cr_resp = 0;
    v_cd_valid = 0;
    vb_cd_data = 0;

    vmsto.ar_bits.burst = AXI_BURST_INCR;                   // INCR (possible any value actually)
    vmsto.aw_bits.burst = AXI_BURST_INCR;                   // INCR (possible any value actually)
    switch (r.state.read()) {
    case state_idle:
        v_next_ready = 1;
        if (i_req_mem_valid.read() == 1) {
            v.req_path = i_req_mem_path.read();
            v.req_addr = i_req_mem_addr.read();
            v.req_size = i_req_mem_size.read();
            // [0] 0=Unpriv/1=Priv;
            // [1] 0=Secure/1=Non-secure;
            // [2] 0=Data/1=Instruction
            v.req_prot = (i_req_mem_path.read() << 2);
            if (i_req_mem_type.read()[REQ_MEM_TYPE_WRITE] == 0) {
                v.state = state_ar;
                v.req_wdata = 0;
                v.req_wstrb = 0;
                if (i_req_mem_type.read()[REQ_MEM_TYPE_CACHED] == 1) {
                    v.req_cached = ARCACHE_WRBACK_READ_ALLOCATE;
                } else {
                    v.req_cached = ARCACHE_DEVICE_NON_BUFFERABLE;
                }
                if (coherence_ena_ == 1) {
                    v.req_ar_snoop = reqtype2arsnoop(i_req_mem_type.read());
                }
            } else {
                v.state = state_aw;
                v.req_wdata = i_req_mem_data.read();
                v.req_wstrb = i_req_mem_strob.read();
                if (i_req_mem_type.read()[REQ_MEM_TYPE_CACHED] == 1) {
                    v.req_cached = AWCACHE_WRBACK_WRITE_ALLOCATE;
                } else {
                    v.req_cached = AWCACHE_DEVICE_NON_BUFFERABLE;
                }
                if (coherence_ena_ == 1) {
                    v.req_aw_snoop = reqtype2awsnoop(i_req_mem_type.read());
                }
            }
        }
        break;
    case state_ar:
        vmsto.ar_valid = 1;
        vmsto.ar_bits.addr = r.req_addr.read();
        vmsto.ar_bits.cache = r.req_cached.read();
        vmsto.ar_bits.size = r.req_size.read();
        vmsto.ar_bits.prot = r.req_prot.read();
        vmsto.ar_snoop = r.req_ar_snoop.read();
        if (i_msti.read().ar_ready == 1) {
            v.state = state_r;
        }
        break;
    case state_r:
        vmsto.r_ready = 1;
        v_mem_er_load_fault = i_msti.read().r_resp[1];
        v_resp_mem_valid = i_msti.read().r_valid;
        // r_valid and r_last always should be in the same time
        if ((i_msti.read().r_valid == 1) && (i_msti.read().r_last == 1)) {
            v.state = state_idle;
        }
        break;
    case state_aw:
        vmsto.aw_valid = 1;
        vmsto.aw_bits.addr = r.req_addr.read();
        vmsto.aw_bits.cache = r.req_cached.read();
        vmsto.aw_bits.size = r.req_size.read();
        vmsto.aw_bits.prot = r.req_prot.read();
        vmsto.aw_snoop = r.req_aw_snoop.read();
        // axi lite to simplify L2-cache
        vmsto.w_valid = 1;
        vmsto.w_last = 1;
        vmsto.w_data = r.req_wdata.read();
        vmsto.w_strb = r.req_wstrb.read();
        if (i_msti.read().aw_ready == 1) {
            if (i_msti.read().w_ready == 1) {
                v.state = state_b;
            } else {
                v.state = state_w;
            }
        }
        break;
    case state_w:
        // Shoudln't get here because of Lite interface:
        vmsto.w_valid = 1;
        vmsto.w_last = 1;
        vmsto.w_data = r.req_wdata.read();
        vmsto.w_strb = r.req_wstrb.read();
        if (i_msti.read().w_ready == 1) {
            v.state = state_b;
        }
        break;
    case state_b:
        vmsto.b_ready = 1;
        v_resp_mem_valid = i_msti.read().b_valid;
        v_mem_er_store_fault = i_msti.read().b_resp[1];
        if (i_msti.read().b_valid == 1) {
            v.state = state_idle;
        }
        break;
    default:
        break;
    }

    // Snoop processing:
    switch (r.snoop_state.read()) {
    case snoop_idle:
        v_snoop_next_ready = 1;
        break;
    case snoop_ac_wait_accept:
        req_snoop_valid = 1;
        vb_req_snoop_addr = r.ac_addr.read();
        vb_req_snoop_type = r.req_snoop_type.read();
        if (i_req_snoop_ready.read() == 1) {
            if (r.cache_access.read() == 0) {
                v.snoop_state = snoop_cr;
            } else {
                v.snoop_state = snoop_cd;
            }
        }
        break;
    case snoop_cr:
        if (i_resp_snoop_valid.read() == 1) {
            v_cr_valid = 1;
            if ((i_resp_snoop_flags.read()[TAG_FL_VALID] == 1)
                    && ((i_resp_snoop_flags.read()[DTAG_FL_SHARED] == 0)
                            || (r.ac_snoop.read() == AC_SNOOP_READ_UNIQUE))) {
                // Need second request with cache access
                v.cache_access = 1;
                // see table C3-21 "Snoop response bit allocation"
                vb_cr_resp[0] = 1;                          // will be Data transfer
                vb_cr_resp[4] = 1;                          // WasUnique
                if (r.ac_snoop.read() == AC_SNOOP_READ_UNIQUE) {
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READCLEAN] = 1;
                } else {
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READDATA] = 1;
                }
                v.req_snoop_type = vb_req_snoop_type;
                v.snoop_state = snoop_ac_wait_accept;
                if (i_msti.read().cr_ready == 1) {
                    v.snoop_state = snoop_ac_wait_accept;
                } else {
                    v.snoop_state = snoop_cr_wait_accept;
                }
            } else {
                vb_cr_resp = 0;
                if (i_msti.read().cr_ready == 1) {
                    v.snoop_state = snoop_idle;
                } else {
                    v.snoop_state = snoop_cr_wait_accept;
                }
            }
            v.cr_resp = vb_cr_resp;
        }
        break;
    case snoop_cr_wait_accept:
        v_cr_valid = 1;
        vb_cr_resp = r.cr_resp.read();
        if (i_msti.read().cr_ready == 1) {
            if (r.cache_access.read() == 1) {
                v.snoop_state = snoop_ac_wait_accept;
            } else {
                v.snoop_state = snoop_idle;
            }
        }
        break;
    case snoop_cd:
        if (i_resp_snoop_valid.read() == 1) {
            v_cd_valid = 1;
            vb_cd_data = i_resp_snoop_data.read();
            v.resp_snoop_data = i_resp_snoop_data.read();
            if (i_msti.read().cd_ready == 1) {
                v.snoop_state = snoop_idle;
            } else {
                v.snoop_state = snoop_cd_wait_accept;
            }
        }
        break;
    case snoop_cd_wait_accept:
        v_cd_valid = 1;
        vb_cd_data = r.resp_snoop_data.read();
        if (i_msti.read().cd_ready == 1) {
            v.snoop_state = snoop_idle;
        }
        break;
    default:
        break;
    }

    if ((coherence_ena_ == 1)
            && (v_snoop_next_ready == 1)
            && (i_msti.read().ac_valid == 1)) {
        req_snoop_valid = 1;
        v.cache_access = 0;
        vb_req_snoop_type = 0;                              // First snoop operation always just to read flags
        v.req_snoop_type = 0;
        vb_req_snoop_addr = i_msti.read().ac_addr;
        v.ac_addr = i_msti.read().ac_addr;
        v.ac_snoop = i_msti.read().ac_snoop;
        if (i_req_snoop_ready.read() == 1) {
            v.snoop_state = snoop_cr;
        } else {
            v.snoop_state = snoop_ac_wait_accept;
        }
    } else {
        v_snoop_next_ready = 1;
        v_cr_valid = 1;
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        l1_dma_snoop_r_reset(v);
    }

    vmsto.ac_ready = v_snoop_next_ready;
    vmsto.cr_valid = v_cr_valid;
    vmsto.cr_resp = vb_cr_resp;
    vmsto.cd_valid = v_cd_valid;
    vmsto.cd_data = vb_cd_data;
    vmsto.cd_last = v_cd_valid;
    vmsto.rack = 0;
    vmsto.wack = 0;

    o_req_mem_ready = v_next_ready;
    o_resp_mem_path = r.req_path.read();
    o_resp_mem_valid = v_resp_mem_valid;
    o_resp_mem_data = i_msti.read().r_data;
    o_resp_mem_load_fault = v_mem_er_load_fault;
    o_resp_mem_store_fault = v_mem_er_store_fault;
    // AXI Snoop IOs:
    o_req_snoop_valid = req_snoop_valid;
    o_req_snoop_type = vb_req_snoop_type;
    o_req_snoop_addr = vb_req_snoop_addr;
    o_resp_snoop_ready = 1;

    o_msto = vmsto;
}

template<int abits>
void l1_dma_snoop<abits>::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        l1_dma_snoop_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

