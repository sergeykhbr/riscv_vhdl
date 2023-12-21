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

#include "sdctrl_cache.h"
#include "api_core.h"

namespace debugger {

sdctrl_cache::sdctrl_cache(sc_module_name name,
                           bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_valid("i_req_valid"),
    i_req_write("i_req_write"),
    i_req_addr("i_req_addr"),
    i_req_wdata("i_req_wdata"),
    i_req_wstrb("i_req_wstrb"),
    o_req_ready("o_req_ready"),
    o_resp_valid("o_resp_valid"),
    o_resp_data("o_resp_data"),
    o_resp_err("o_resp_err"),
    i_resp_ready("i_resp_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_data("o_req_mem_data"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data("i_mem_data"),
    i_mem_fault("i_mem_fault"),
    i_flush_valid("i_flush_valid"),
    o_flush_end("o_flush_end") {

    async_reset_ = async_reset;
    mem0 = 0;

    mem0 = new TagMem<abus,
                      ibits,
                      lnbits,
                      flbits,
                      0>("mem0", async_reset);
    mem0->i_clk(i_clk);
    mem0->i_nrst(i_nrst);
    mem0->i_addr(r.line_addr_i);
    mem0->i_wstrb(line_wstrb_i);
    mem0->i_wdata(line_wdata_i);
    mem0->i_wflags(line_wflags_i);
    mem0->o_raddr(line_raddr_o);
    mem0->o_rdata(line_rdata_o);
    mem0->o_rflags(line_rflags_o);
    mem0->o_hit(line_hit_o);
    mem0->i_snoop_addr(line_snoop_addr_i);
    mem0->o_snoop_flags(line_snoop_flags_o);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_valid;
    sensitive << i_req_write;
    sensitive << i_req_addr;
    sensitive << i_req_wdata;
    sensitive << i_req_wstrb;
    sensitive << i_resp_ready;
    sensitive << i_req_mem_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_mem_fault;
    sensitive << i_flush_valid;
    sensitive << line_wdata_i;
    sensitive << line_wstrb_i;
    sensitive << line_wflags_i;
    sensitive << line_raddr_o;
    sensitive << line_rdata_o;
    sensitive << line_rflags_o;
    sensitive << line_hit_o;
    sensitive << line_snoop_addr_i;
    sensitive << line_snoop_flags_o;
    sensitive << r.req_write;
    sensitive << r.req_addr;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.req_mem_write;
    sensitive << r.mem_addr;
    sensitive << r.mem_fault;
    sensitive << r.write_first;
    sensitive << r.write_flush;
    sensitive << r.req_flush;
    sensitive << r.flush_cnt;
    sensitive << r.line_addr_i;
    sensitive << r.cache_line_i;
    sensitive << r.cache_line_o;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

sdctrl_cache::~sdctrl_cache() {
    if (mem0) {
        delete mem0;
    }
}

void sdctrl_cache::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_write, i_req_write.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_wdata, i_req_wdata.name());
        sc_trace(o_vcd, i_req_wstrb, i_req_wstrb.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_resp_err, o_resp_err.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());
        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_fault, i_mem_fault.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, o_flush_end, o_flush_end.name());
        sc_trace(o_vcd, r.req_write, pn + ".r_req_write");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_mem_valid, pn + ".r_req_mem_valid");
        sc_trace(o_vcd, r.req_mem_write, pn + ".r_req_mem_write");
        sc_trace(o_vcd, r.mem_addr, pn + ".r_mem_addr");
        sc_trace(o_vcd, r.mem_fault, pn + ".r_mem_fault");
        sc_trace(o_vcd, r.write_first, pn + ".r_write_first");
        sc_trace(o_vcd, r.write_flush, pn + ".r_write_flush");
        sc_trace(o_vcd, r.req_flush, pn + ".r_req_flush");
        sc_trace(o_vcd, r.flush_cnt, pn + ".r_flush_cnt");
        sc_trace(o_vcd, r.line_addr_i, pn + ".r_line_addr_i");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
        sc_trace(o_vcd, r.cache_line_o, pn + ".r_cache_line_o");
    }

    if (mem0) {
        mem0->generateVCD(i_vcd, o_vcd);
    }
}

void sdctrl_cache::comb() {
    sc_biguint<SDCACHE_LINE_BITS> vb_cache_line_i_modified;
    sc_biguint<SDCACHE_LINE_BITS> vb_line_rdata_o_modified;
    sc_uint<SDCACHE_BYTES_PER_LINE> vb_line_rdata_o_wstrb;
    bool v_req_ready;
    sc_biguint<SDCACHE_LINE_BITS> t_cache_line_i;
    sc_uint<64> vb_cached_data;
    sc_uint<64> vb_uncached_data;
    bool v_resp_valid;
    sc_uint<64> vb_resp_data;
    bool v_flush_end;
    sc_biguint<SDCACHE_LINE_BITS> vb_line_wdata;
    sc_uint<SDCACHE_BYTES_PER_LINE> vb_line_wstrb;
    sc_uint<64> vb_req_mask;
    sc_uint<SDCACHE_FL_TOTAL> vb_line_wflags;
    sc_uint<(CFG_LOG2_SDCACHE_BYTES_PER_LINE - 3)> ridx;
    bool v_req_same_line;
    bool v_mem_addr_last;
    sc_uint<CFG_SDCACHE_ADDR_BITS> vb_addr_direct_next;

    vb_cache_line_i_modified = 0;
    vb_line_rdata_o_modified = 0;
    vb_line_rdata_o_wstrb = 0;
    v_req_ready = 0;
    t_cache_line_i = 0;
    vb_cached_data = 0;
    vb_uncached_data = 0;
    v_resp_valid = 0;
    vb_resp_data = 0;
    v_flush_end = 0;
    vb_line_wdata = 0;
    vb_line_wstrb = 0;
    vb_req_mask = 0;
    vb_line_wflags = 0;
    ridx = 0;
    v_req_same_line = 0;
    v_mem_addr_last = 0;
    vb_addr_direct_next = 0;

    v = r;

    ridx = r.req_addr.read()((CFG_LOG2_SDCACHE_BYTES_PER_LINE - 1), 3);
    v_mem_addr_last = r.mem_addr.read()(9, CFG_LOG2_SDCACHE_BYTES_PER_LINE).and_reduce();

    vb_cached_data = line_rdata_o.read()((64 * ridx.to_int()) + 64 - 1, (64 * ridx.to_int()));
    vb_uncached_data = r.cache_line_i.read()(63, 0).to_uint64();

    if (r.req_addr.read()((CFG_SDCACHE_ADDR_BITS - 1), CFG_LOG2_SDCACHE_BYTES_PER_LINE) == i_req_addr.read()((CFG_SDCACHE_ADDR_BITS - 1), CFG_LOG2_SDCACHE_BYTES_PER_LINE)) {
        v_req_same_line = 1;
    }

    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
    }

    for (int i = 0; i < 8; i++) {
        if (r.req_wstrb.read()[i] == 1) {
            vb_req_mask((8 * i) + 8 - 1, (8 * i)) = 0xFF;
        }
    }

    vb_line_rdata_o_modified = line_rdata_o;
    vb_cache_line_i_modified = r.cache_line_i;
    for (int i = 0; i < (SDCACHE_BYTES_PER_LINE / 8); i++) {
        if (i == ridx.to_int()) {
            vb_line_rdata_o_modified((64 * i) + 64 - 1, (64 * i)) = ((vb_line_rdata_o_modified((64 * i) + 64 - 1, (64 * i))
                            & (~vb_req_mask))
                    | (r.req_wdata.read() & vb_req_mask));
            vb_cache_line_i_modified((64 * i) + 64 - 1, (64 * i)) = ((vb_cache_line_i_modified((64 * i) + 64 - 1, (64 * i))
                            & (~vb_req_mask))
                    | (r.req_wdata.read() & vb_req_mask));
            vb_line_rdata_o_wstrb((8 * i) + 8 - 1, (8 * i)) = r.req_wstrb;
        }
    }

    // Flush counter when direct access
    vb_addr_direct_next = (r.req_addr.read() + 1);

    vb_line_wdata = r.cache_line_i;

    // System Bus access state machine
    switch (r.state.read()) {
    case State_Idle:
        v.mem_fault = 0;
        if (r.req_flush.read() == 1) {
            v.state = State_FlushAddr;
            v.cache_line_i = 0;
            v.flush_cnt = FLUSH_ALL_VALUE;
        } else {
            v_req_ready = 1;
            if (i_req_valid.read() == 1) {
                v.line_addr_i = i_req_addr;
                v.req_addr = i_req_addr;
                v.req_wstrb = i_req_wstrb;
                v.req_wdata = i_req_wdata;
                v.req_write = i_req_write;
                if (v_req_same_line == 1) {
                    // Write address is the same as the next requested, so use it to write
                    // value and update state machine
                    v.state = State_CheckHit;
                } else {
                    v.state = State_SetupReadAdr;
                }
            }
        }
        break;
    case State_SetupReadAdr:
        v.state = State_CheckHit;
        break;
    case State_CheckHit:
        vb_resp_data = vb_cached_data;
        if (line_hit_o.read() == 1) {
            // Hit
            v_resp_valid = 1;
            if (i_resp_ready.read() == 1) {
                v.state = State_Idle;
                if (r.req_write.read() == 1) {
                    // Modify tagged mem output with request and write back
                    vb_line_wflags[SDCACHE_FL_VALID] = 1;
                    vb_line_wflags[SDCACHE_FL_DIRTY] = 1;
                    v.req_write = 0;
                    vb_line_wstrb = vb_line_rdata_o_wstrb;
                    vb_line_wdata = vb_line_rdata_o_modified;
                    v.state = State_Idle;
                }
            }
        } else {
            // Miss
            v.state = State_TranslateAddress;
        }
        break;
    case State_TranslateAddress:
        v.req_mem_valid = 1;
        v.mem_fault = 0;
        v.state = State_WaitGrant;
        if ((line_rflags_o.read()[SDCACHE_FL_VALID] == 1)
                && (line_rflags_o.read()[SDCACHE_FL_DIRTY] == 1)) {
            v.write_first = 1;
            v.req_mem_write = 1;
            v.mem_addr = (line_raddr_o.read()((CFG_SDCACHE_ADDR_BITS - 1), 9) << 9);
        } else {
            // 1. Read -> Save cache
            // 2. Read -> Modify -> Save cache
            v.mem_addr = (r.req_addr.read()((CFG_SDCACHE_ADDR_BITS - 1), 9) << 9);
            v.req_mem_write = r.req_write;
        }
        v.cache_line_o = line_rdata_o;
        v.cache_line_i = 0;
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read() == 1) {
            if ((r.write_flush.read() == 1)
                    || (r.write_first.read() == 1)
                    || (r.req_write.read() == 1)) {
                v.state = State_WriteBus;
            } else {
                // 1. uncached read
                // 2. cached read or write
                v.state = State_WaitResp;
            }
            v.req_mem_valid = 0;
        }
        break;
    case State_WaitResp:
        if (i_mem_data_valid.read() == 1) {
            v.cache_line_i = i_mem_data;
            v.state = State_CheckResp;
            v.mem_fault = i_mem_fault;
        }
        break;
    case State_CheckResp:
        if (r.mem_fault.read() == 1) {
            // uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid = 1;
            vb_resp_data = vb_uncached_data;
            if (i_resp_ready.read() == 1) {
                v.state = State_Idle;
            }
        } else {
            vb_line_wflags[SDCACHE_FL_VALID] = 1;
            vb_line_wstrb = ~0ull;                          // write full line
            v.mem_addr = (r.mem_addr.read() + SDCACHE_BYTES_PER_LINE);
            if (v_mem_addr_last == 1) {
                v.state = State_SetupReadAdr;
                v.line_addr_i = r.req_addr;
            } else {
                v.state = State_WaitResp;
                v.line_addr_i = (r.line_addr_i.read() + SDCACHE_BYTES_PER_LINE);
            }
        }
        break;
    case State_WriteBus:
        if (i_mem_data_valid.read() == 1) {
            if (r.write_flush.read() == 1) {
                // Offloading Cache line on flush request
                v.state = State_FlushAddr;
            } else if (r.write_first.read() == 1) {
                // Obsolete line was offloaded, now read new line
                v.mem_addr = (r.req_addr.read()((CFG_SDCACHE_ADDR_BITS - 1), CFG_LOG2_SDCACHE_BYTES_PER_LINE) << CFG_LOG2_SDCACHE_BYTES_PER_LINE);
                v.req_mem_valid = 1;
                v.write_first = 0;
                v.req_mem_write = r.req_write;
                v.state = State_WaitGrant;
            } else {
                // Non-cached write
                v.state = State_Idle;
                v_resp_valid = 1;
                v.mem_fault = i_mem_fault;
            }
        }
        break;
    case State_FlushAddr:
        v.state = State_FlushCheck;
        vb_line_wstrb = ~0ull;
        vb_line_wflags = 0;
        v.write_flush = 0;
        v.cache_line_i = 0;
        break;
    case State_FlushCheck:
        v.cache_line_o = line_rdata_o;
        if ((line_rflags_o.read()[SDCACHE_FL_VALID] == 1)
                && (line_rflags_o.read()[SDCACHE_FL_DIRTY] == 1)) {
            // Off-load valid line
            v.write_flush = 1;
            v.mem_addr = line_raddr_o;
            v.req_mem_valid = 1;
            v.req_mem_write = 1;
            v.state = State_WaitGrant;
        } else {
            // Write clean line
            v.state = State_FlushAddr;
            if (r.flush_cnt.read().or_reduce() == 0) {
                v.state = State_Idle;
                v_flush_end = 1;
            }
        }
        v.line_addr_i = (r.line_addr_i.read() + SDCACHE_BYTES_PER_LINE);
        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = (r.flush_cnt.read() - 1);
        }
        break;
    case State_Reset:
        // Write clean line
        v.line_addr_i = 0;
        v.flush_cnt = FLUSH_ALL_VALUE;                      // Init after power-on-reset
        v.state = State_ResetWrite;
        break;
    case State_ResetWrite:
        vb_line_wstrb = ~0ull;
        vb_line_wflags = 0;
        v.line_addr_i = (r.line_addr_i.read() + SDCACHE_BYTES_PER_LINE);
        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = (r.flush_cnt.read() - 1);
        } else {
            v.state = State_Idle;
        }
        break;
    default:
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_cache_r_reset(v);
    }

    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = vb_line_wflags;
    line_snoop_addr_i = 0;

    o_req_ready = v_req_ready;
    o_req_mem_valid = r.req_mem_valid;
    o_req_mem_addr = r.mem_addr;
    o_req_mem_write = r.req_mem_write;
    o_req_mem_data = r.cache_line_o;

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_err = r.mem_fault;

    o_flush_end = v_flush_end;
}

void sdctrl_cache::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_cache_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

