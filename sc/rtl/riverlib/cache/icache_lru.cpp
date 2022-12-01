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

#include "icache_lru.h"
#include "api_core.h"

namespace debugger {

ICacheLru::ICacheLru(sc_module_name name,
                     bool async_reset,
                     uint32_t waybits,
                     uint32_t ibits)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_valid("i_req_valid"),
    i_req_addr("i_req_addr"),
    o_req_ready("o_req_ready"),
    o_resp_valid("o_resp_valid"),
    o_resp_addr("o_resp_addr"),
    o_resp_data("o_resp_data"),
    o_resp_load_fault("o_resp_load_fault"),
    i_resp_ready("i_resp_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_type("o_req_mem_type"),
    o_req_mem_size("o_req_mem_size"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data("i_mem_data"),
    i_mem_load_fault("i_mem_load_fault"),
    o_mpu_addr("o_mpu_addr"),
    i_pma_cached("i_pma_cached"),
    i_pmp_x("i_pmp_x"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid") {

    async_reset_ = async_reset;
    waybits_ = waybits;
    ibits_ = ibits;
    ways = (1 << waybits);
    FLUSH_ALL_VALUE = ((1 << (ibits + waybits)) - 1);
    mem0 = 0;

    mem0 = new TagMemCoupled<abus,
                             2,
                             7,
                             lnbits,
                             flbits>("mem0", async_reset);
    mem0->i_clk(i_clk);
    mem0->i_nrst(i_nrst);
    mem0->i_direct_access(line_direct_access_i);
    mem0->i_invalidate(line_invalidate_i);
    mem0->i_re(line_re_i);
    mem0->i_we(line_we_i);
    mem0->i_addr(line_addr_i);
    mem0->i_wdata(line_wdata_i);
    mem0->i_wstrb(line_wstrb_i);
    mem0->i_wflags(line_wflags_i);
    mem0->o_raddr(line_raddr_o);
    mem0->o_rdata(line_rdata_o);
    mem0->o_rflags(line_rflags_o);
    mem0->o_hit(line_hit_o);
    mem0->o_hit_next(line_hit_next_o);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_valid;
    sensitive << i_req_addr;
    sensitive << i_resp_ready;
    sensitive << i_req_mem_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_mem_load_fault;
    sensitive << i_pma_cached;
    sensitive << i_pmp_x;
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << line_direct_access_i;
    sensitive << line_invalidate_i;
    sensitive << line_re_i;
    sensitive << line_we_i;
    sensitive << line_addr_i;
    sensitive << line_wdata_i;
    sensitive << line_wstrb_i;
    sensitive << line_wflags_i;
    sensitive << line_raddr_o;
    sensitive << line_rdata_o;
    sensitive << line_rflags_o;
    sensitive << line_hit_o;
    sensitive << line_hit_next_o;
    sensitive << r.req_addr;
    sensitive << r.req_addr_next;
    sensitive << r.write_addr;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.mem_addr;
    sensitive << r.req_mem_type;
    sensitive << r.req_mem_size;
    sensitive << r.load_fault;
    sensitive << r.req_flush;
    sensitive << r.req_flush_all;
    sensitive << r.req_flush_addr;
    sensitive << r.req_flush_cnt;
    sensitive << r.flush_cnt;
    sensitive << r.cache_line_i;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

ICacheLru::~ICacheLru() {
    if (mem0) {
        delete mem0;
    }
}

void ICacheLru::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_addr, o_resp_addr.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_resp_load_fault, o_resp_load_fault.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());
        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_type, o_req_mem_type.name());
        sc_trace(o_vcd, o_req_mem_size, o_req_mem_size.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_load_fault, i_mem_load_fault.name());
        sc_trace(o_vcd, o_mpu_addr, o_mpu_addr.name());
        sc_trace(o_vcd, i_pma_cached, i_pma_cached.name());
        sc_trace(o_vcd, i_pmp_x, i_pmp_x.name());
        sc_trace(o_vcd, i_flush_address, i_flush_address.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_addr_next, pn + ".r_req_addr_next");
        sc_trace(o_vcd, r.write_addr, pn + ".r_write_addr");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_mem_valid, pn + ".r_req_mem_valid");
        sc_trace(o_vcd, r.mem_addr, pn + ".r_mem_addr");
        sc_trace(o_vcd, r.req_mem_type, pn + ".r_req_mem_type");
        sc_trace(o_vcd, r.req_mem_size, pn + ".r_req_mem_size");
        sc_trace(o_vcd, r.load_fault, pn + ".r_load_fault");
        sc_trace(o_vcd, r.req_flush, pn + ".r_req_flush");
        sc_trace(o_vcd, r.req_flush_all, pn + ".r_req_flush_all");
        sc_trace(o_vcd, r.req_flush_addr, pn + ".r_req_flush_addr");
        sc_trace(o_vcd, r.req_flush_cnt, pn + ".r_req_flush_cnt");
        sc_trace(o_vcd, r.flush_cnt, pn + ".r_flush_cnt");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
    }

}

void ICacheLru::comb() {
    sc_biguint<L1CACHE_LINE_BITS> t_cache_line_i;
    bool v_req_ready;
    bool v_resp_valid;
    sc_uint<64> vb_cached_data;
    sc_uint<64> vb_uncached_data;
    sc_uint<64> vb_resp_data;
    bool v_resp_er_load_fault;
    bool v_direct_access;
    bool v_invalidate;
    bool v_line_cs_read;
    bool v_line_cs_write;
    sc_uint<CFG_CPU_ADDR_BITS> vb_line_addr;
    sc_biguint<L1CACHE_LINE_BITS> vb_line_wdata;
    sc_uint<L1CACHE_BYTES_PER_LINE> vb_line_wstrb;
    sc_uint<ITAG_FL_TOTAL> v_line_wflags;
    int sel_cached;
    int sel_uncached;
    bool v_ready_next;
    sc_uint<CFG_CPU_ADDR_BITS> vb_addr_direct_next;

    t_cache_line_i = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    vb_cached_data = 0;
    vb_uncached_data = 0;
    vb_resp_data = 0;
    v_resp_er_load_fault = 0;
    v_direct_access = 0;
    v_invalidate = 0;
    v_line_cs_read = 0;
    v_line_cs_write = 0;
    vb_line_addr = 0;
    vb_line_wdata = 0;
    vb_line_wstrb = 0;
    v_line_wflags = 0;
    sel_cached = 0;
    sel_uncached = 0;
    v_ready_next = 0;
    vb_addr_direct_next = 0;

    v = r;

    sel_cached = r.req_addr.read()((CFG_LOG2_L1CACHE_BYTES_PER_LINE - 1), 2).to_int();
    sel_uncached = r.req_addr.read()(2, 2).to_int();
    vb_cached_data = line_rdata_o.read()((32 * sel_cached) + 64 - 1, (32 * sel_cached));
    vb_uncached_data = r.cache_line_i.read()((32 * sel_uncached) + 64 - 1, (32 * sel_uncached));

    // flush request via debug interface
    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
        v.req_flush_all = i_flush_address.read()[0];
        if (i_flush_address.read()[0] == 1) {
            v.req_flush_cnt = FLUSH_ALL_VALUE;
            v.req_flush_addr = 0;
        } else {
            v.req_flush_cnt = 0;
            v.req_flush_addr = i_flush_address;
        }
    }

    // Flush counter when direct access
    if (r.req_addr.read()((waybits_ - 1), 0) == (ways - 1)) {
        vb_addr_direct_next = ((r.req_addr.read() + L1CACHE_BYTES_PER_LINE) & (~LINE_BYTES_MASK));
    } else {
        vb_addr_direct_next = (r.req_addr.read() + 1);
    }

    vb_line_addr = r.req_addr;
    vb_line_wdata = r.cache_line_i;

    switch (r.state.read()) {
    case State_Idle:
        v_ready_next = 1;
        break;
    case State_CheckHit:
        vb_resp_data = vb_cached_data;
        if ((line_hit_o.read() == 1) && (line_hit_next_o.read() == 1)) {
            // Hit
            v_resp_valid = 1;
            if (i_resp_ready.read() == 1) {
                v_ready_next = 1;
                v.state = State_Idle;
            }
        } else {
            // Miss
            v.state = State_TranslateAddress;
        }
        break;
    case State_TranslateAddress:
        if (i_pmp_x.read() == 0) {
            t_cache_line_i = 0;
            v.cache_line_i = (~t_cache_line_i);
            v.state = State_CheckResp;
            v.load_fault = 1;
        } else {
            v.req_mem_valid = 1;
            v.state = State_WaitGrant;
            v.write_addr = r.req_addr;
            v.load_fault = 0;

            if (i_pma_cached.read() == 1) {
                if (line_hit_o.read() == 0) {
                    v.mem_addr = (r.req_addr.read()((CFG_CPU_ADDR_BITS - 1), CFG_LOG2_L1CACHE_BYTES_PER_LINE) << CFG_LOG2_L1CACHE_BYTES_PER_LINE);
                } else {
                    v.write_addr = r.req_addr_next;
                    v.mem_addr = (r.req_addr_next.read()((CFG_CPU_ADDR_BITS - 1), CFG_LOG2_L1CACHE_BYTES_PER_LINE) << CFG_LOG2_L1CACHE_BYTES_PER_LINE);
                }
                v.req_mem_type = ReadShared();
                v.req_mem_size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
            } else {
                v.mem_addr = (r.req_addr.read()((CFG_CPU_ADDR_BITS - 1), 3) << 3);
                v.req_mem_type = ReadNoSnoop();
                v.req_mem_size = 4;                         // uncached, 16 B
            }
        }
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read() == 1) {
            v.state = State_WaitResp;
            v.req_mem_valid = 0;
        }
        break;
    case State_WaitResp:
        if (i_mem_data_valid.read() == 1) {
            v.cache_line_i = i_mem_data;
            v.state = State_CheckResp;
            v.write_addr = r.req_addr;                      // Swap addres for 1 clock to write line
            v.req_addr = r.write_addr;
            if (i_mem_load_fault.read() == 1) {
                v.load_fault = 1;
            }
        }
        break;
    case State_CheckResp:
        v.req_addr = r.write_addr;                          // Restore req_addr after line write
        if ((r.req_mem_type.read()[REQ_MEM_TYPE_CACHED] == 0)
                || (r.load_fault.read() == 1)) {
            v_resp_valid = 1;
            vb_resp_data = vb_uncached_data;
            v_resp_er_load_fault = r.load_fault;
            if (i_resp_ready.read() == 1) {
                v.state = State_Idle;
            }
        } else {
            v.state = State_SetupReadAdr;
            v_line_cs_write = 1;
            v_line_wflags[TAG_FL_VALID] = 1;
            vb_line_wstrb = ~0ull;                          // write full line
        }
        break;
    case State_SetupReadAdr:
        v.state = State_CheckHit;
        break;
    case State_FlushAddr:
        v.state = State_FlushCheck;
        v_direct_access = r.req_flush_all;                  // 0=only if hit; 1=will be applied ignoring hit
        v_invalidate = 1;                                   // generate: wstrb='1; wflags='0
        v.cache_line_i = 0;
        break;
    case State_FlushCheck:
        v.state = State_FlushAddr;
        v_direct_access = r.req_flush_all;
        v_line_cs_write = r.req_flush_all;
        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = (r.flush_cnt.read() - 1);
            if (r.req_flush_all.read() == 1) {
                v.req_addr = vb_addr_direct_next;
            } else {
                v.req_addr = (r.req_addr.read() + L1CACHE_BYTES_PER_LINE);
            }
        } else {
            v.state = State_Idle;
        }
        break;
    case State_Reset:
        // Write clean line
        if (r.req_flush.read() == 1) {
            v.req_flush = 0;
            v.flush_cnt = FLUSH_ALL_VALUE;                  // Init after power-on-reset
        }
        v_direct_access = 1;
        v_invalidate = 1;                                   // generate: wstrb='1; wflags='0
        v.state = State_ResetWrite;
        break;
    case State_ResetWrite:
        v_direct_access = 1;
        v_line_cs_write = 1;
        v.state = State_Reset;

        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = (r.flush_cnt.read() - 1);
            v.req_addr = vb_addr_direct_next;
        } else {
            v.state = State_Idle;
        }
        break;
    default:
        break;
    }

    if (v_ready_next == 1) {
        if (r.req_flush.read() == 1) {
            v.state = State_FlushAddr;
            v.req_flush = 0;
            v.cache_line_i = 0;
            v.req_addr = (r.req_flush_addr.read() & (~LINE_BYTES_MASK));
            v.flush_cnt = r.req_flush_cnt;
        } else {
            v_req_ready = 1;
            v_line_cs_read = i_req_valid;
            vb_line_addr = i_req_addr;
            if (i_req_valid.read() == 1) {
                v.req_addr = i_req_addr;
                v.req_addr_next = (i_req_addr.read() + L1CACHE_BYTES_PER_LINE);
                v.state = State_CheckHit;
            }
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        ICacheLru_r_reset(v);
    }

    line_direct_access_i = v_direct_access;
    line_invalidate_i = v_invalidate;
    line_re_i = v_line_cs_read;
    line_we_i = v_line_cs_write;
    line_addr_i = vb_line_addr;
    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = v_line_wflags;

    o_req_ready = v_req_ready;
    o_req_mem_valid = r.req_mem_valid;
    o_req_mem_addr = r.mem_addr;
    o_req_mem_type = r.req_mem_type;
    o_req_mem_size = r.req_mem_size;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_addr = r.req_addr;
    o_resp_load_fault = v_resp_er_load_fault;
    o_mpu_addr = r.req_addr;
}

void ICacheLru::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        ICacheLru_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

