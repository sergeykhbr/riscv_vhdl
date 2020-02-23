/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "dcache_lru.h"

namespace debugger {

DCacheLru::DCacheLru(sc_module_name name_, bool async_reset, bool coherence_ena)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_valid("i_req_valid"),
    i_req_write("i_req_write"),
    i_req_addr("i_req_addr"),
    i_req_wdata("i_req_wdata"),
    i_req_wstrb("i_req_wstrb"),
    o_req_ready("o_req_ready"),
    o_resp_valid("o_resp_valid"),
    o_resp_addr("o_resp_addr"),
    o_resp_data("o_resp_data"),
    o_resp_er_addr("o_resp_er_addr"),
    o_resp_er_load_fault("o_resp_er_load_fault"),
    o_resp_er_store_fault("o_resp_er_store_fault"),
    o_resp_er_mpu_load("o_resp_er_mpu_load"),
    o_resp_er_mpu_store("o_resp_er_mpu_store"),
    i_resp_ready("i_resp_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_type("o_req_mem_type"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data("i_mem_data"),
    i_mem_load_fault("i_mem_load_fault"),
    i_mem_store_fault("i_mem_store_fault"),
    o_mpu_addr("o_mpu_addr"),
    i_mpu_flags("i_mpu_flags"),
    i_req_snoop_valid("i_req_snoop_valid"),
    i_req_snoop_type("i_req_snoop_type"),
    o_req_snoop_ready("o_req_snoop_ready"),
    i_req_snoop_addr("i_req_snoop_addr"),
    i_resp_snoop_ready("i_resp_snoop_ready"),
    o_resp_snoop_valid("o_resp_snoop_valid"),
    o_resp_snoop_data("o_resp_snoop_data"),
    o_resp_snoop_flags("o_resp_snoop_flags"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    o_flush_end("o_flush_end"),
    o_state("o_state") {
    async_reset_ = async_reset;
    coherence_ena_ = coherence_ena;

    mem = new TagMemNWay<CFG_CPU_ADDR_BITS,
                         CFG_DLOG2_NWAYS,
                         CFG_DLOG2_LINES_PER_WAY,
                         CFG_DLOG2_BYTES_PER_LINE,
                         DTAG_FL_TOTAL,
                         CFG_SNOOP_ENA>("mem0", async_reset);
    mem->i_clk(i_clk);
    mem->i_nrst(i_nrst);
    mem->i_direct_access(line_direct_access_i);
    mem->i_invalidate(line_invalidate_i);
    mem->i_re(line_re_i);
    mem->i_we(line_we_i);
    mem->i_addr(line_addr_i);
    mem->i_wdata(line_wdata_i);
    mem->i_wstrb(line_wstrb_i);
    mem->i_wflags(line_wflags_i);
    mem->o_raddr(line_raddr_o);
    mem->o_rdata(line_rdata_o);
    mem->o_rflags(line_rflags_o);
    mem->i_snoop_addr(line_snoop_addr_i);
    mem->o_snoop_ready(line_snoop_ready_o);
    mem->o_snoop_flags(line_snoop_flags_o);


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_valid;
    sensitive << i_req_write;
    sensitive << i_req_addr;
    sensitive << i_req_wdata;
    sensitive << i_req_wstrb;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_mem_load_fault;
    sensitive << i_mem_store_fault;
    sensitive << i_resp_ready;
    sensitive << i_req_snoop_valid;
    sensitive << i_req_snoop_type;
    sensitive << o_req_snoop_ready;
    sensitive << i_req_snoop_addr;
    sensitive << i_resp_snoop_ready;
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << i_req_mem_ready;
    sensitive << i_mpu_flags;
    sensitive << line_raddr_o;
    sensitive << line_rdata_o;
    sensitive << line_rflags_o;
    sensitive << r.req_addr;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.req_mem_type;
    sensitive << r.mem_addr;
    sensitive << r.load_fault;
    sensitive << r.write_first;
    sensitive << r.write_flush;
    sensitive << r.mem_wstrb;
    sensitive << r.req_flush;
    sensitive << r.req_flush_all;
    sensitive << r.req_flush_addr;
    sensitive << r.req_flush_cnt;
    sensitive << r.flush_cnt;
    sensitive << r.cache_line_i;
    sensitive << r.cache_line_o;
    sensitive << r.req_snoop_type;
    sensitive << r.snoop_flags_valid;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

DCacheLru::~DCacheLru() {
    delete mem;
}

void DCacheLru::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, i_nrst.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_write, i_req_write.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_wdata, i_req_wdata.name());
        sc_trace(o_vcd, i_req_wstrb, i_req_wstrb.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_addr, o_resp_addr.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_resp_er_load_fault, o_resp_er_load_fault.name());
        sc_trace(o_vcd, o_resp_er_store_fault, o_resp_er_store_fault.name());
        sc_trace(o_vcd, o_resp_er_mpu_load, o_resp_er_mpu_load.name());
        sc_trace(o_vcd, o_resp_er_mpu_store, o_resp_er_mpu_store.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());

        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_type, o_req_mem_type.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_load_fault, i_mem_load_fault.name());
        sc_trace(o_vcd, i_mem_store_fault, i_mem_store_fault.name());

        sc_trace(o_vcd, i_mpu_flags, i_mpu_flags.name());
        sc_trace(o_vcd, i_flush_address, i_flush_address.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, o_state, o_state.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, line_addr_i, pn + ".linei_addr_i");
        sc_trace(o_vcd, line_wstrb_i, pn + ".linei_wstrb_i");
        sc_trace(o_vcd, line_raddr_o, pn + ".line_raddr_o");
        sc_trace(o_vcd, line_rdata_o, pn + ".line_rdata_o");
        sc_trace(o_vcd, r.load_fault, pn + ".r_load_fault");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
        sc_trace(o_vcd, r.cache_line_o, pn + ".r_cache_line_o");
        sc_trace(o_vcd, r.write_first, pn + ".r_write_first");
        sc_trace(o_vcd, r.write_flush, pn + ".r_write_flush");
        sc_trace(o_vcd, r.mem_wstrb, pn + ".r_mem_wstrb");
        sc_trace(o_vcd, r.flush_cnt, pn + ".r_flush_cnt");
        sc_trace(o_vcd, r.req_flush, pn + ".r_req_flush");
        sc_trace(o_vcd, r.req_flush_addr, pn + ".r_req_flush_addr");
        sc_trace(o_vcd, r.req_flush_cnt, pn + ".r_req_flush_cnt");
    }
    mem->generateVCD(i_vcd, o_vcd);
}

void DCacheLru::comb() {
    sc_biguint<DCACHE_LINE_BITS> vb_cache_line_i_modified;
    sc_biguint<DCACHE_LINE_BITS> vb_line_rdata_o_modified;
    sc_uint<DCACHE_BYTES_PER_LINE> vb_line_rdata_o_wstrb;
    
    bool v_req_ready;
    sc_biguint<DCACHE_LINE_BITS> t_cache_line_i;
    sc_uint<64> vb_cached_data;
    sc_uint<64> vb_uncached_data;
    bool v_resp_valid;
    sc_uint<64> vb_resp_data;
    bool v_resp_er_load_fault;
    bool v_resp_er_store_fault;
    bool v_direct_access;
    bool v_invalidate;
    bool v_flush_end;
    bool v_line_cs_read;
    bool v_line_cs_write;
    sc_uint<CFG_CPU_ADDR_BITS> vb_line_addr;
    sc_biguint<DCACHE_LINE_BITS> vb_line_wdata;
    sc_uint<DCACHE_BYTES_PER_LINE> vb_line_wstrb;
    sc_biguint<64> vb_req_mask;
    sc_uint<DTAG_FL_TOTAL> v_line_wflags;
    sc_uint<CFG_DLOG2_BYTES_PER_LINE-3> ridx;
    bool v_req_same_line;
    bool v_ready_next;
    bool v_req_snoop_ready;
    bool v_resp_snoop_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_addr_direct_next;
   
    v = r;

    v_ready_next = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    vb_resp_data = 0;
    v_resp_er_load_fault = 0;
    v_resp_er_store_fault = 0;
    v_direct_access = 0;
    v_invalidate = 0;
    v_flush_end = 0;
    v_req_snoop_ready = 0;
    v_resp_snoop_valid = r.snoop_flags_valid;
    ridx = r.req_addr.read()(CFG_DLOG2_BYTES_PER_LINE-1, 3);

    vb_cached_data = line_rdata_o.read()((ridx.to_int()+1)*64 - 1,
                                         ridx.to_int()*64);
    vb_uncached_data = r.cache_line_i.read()(63, 0);

    v_req_same_line = 0;
    if (r.req_addr.read()(CFG_CPU_ADDR_BITS-1, CFG_DLOG2_BYTES_PER_LINE)
        == i_req_addr.read()(CFG_CPU_ADDR_BITS-1, CFG_DLOG2_BYTES_PER_LINE)) {
        v_req_same_line = 1;
    }


    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
        v.req_flush_all = i_flush_address.read()[0];
        if (i_flush_address.read()[0] == 1) {
            v.req_flush_cnt = ~0u;
            v.req_flush_addr = 0;
        } else {
            v.req_flush_cnt = 0;
            v.req_flush_addr = i_flush_address.read();
        }
    }

    vb_req_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (r.req_wstrb.read()[i] == 1) {
            vb_req_mask(8*i+7, 8*i) = ~0u;
        }
    }

    vb_line_rdata_o_modified = line_rdata_o.read();
    vb_cache_line_i_modified = r.cache_line_i.read();
    vb_line_rdata_o_wstrb = 0;
    for (int i = 0; i < DCACHE_BYTES_PER_LINE/8; i++) {
        if (i != ridx.to_int()) {
            continue;
        }
        vb_line_rdata_o_modified(64*(i+1)-1, 64*i) =
            (vb_line_rdata_o_modified(64*(i+1)-1, 64*i)
             & ~vb_req_mask) | (r.req_wdata.read() & vb_req_mask);

        vb_cache_line_i_modified(64*(i+1)-1, 64*i) =
            (vb_cache_line_i_modified(64*(i+1)-1, 64*i)
             & ~vb_req_mask) | (r.req_wdata.read() & vb_req_mask);

        vb_line_rdata_o_wstrb(8*(i+1)-1, 8*i) =
            r.req_wstrb.read();
    }

    // Flush counter when direct access
    if (r.req_addr.read()(CFG_DLOG2_NWAYS-1, 0) == DCACHE_WAYS-1) {
        vb_addr_direct_next = (r.req_addr.read() + DCACHE_BYTES_PER_LINE) 
                    & ~((1<<CFG_DLOG2_BYTES_PER_LINE)-1);
    } else {
        vb_addr_direct_next = r.req_addr.read() + 1;
    }


    v_line_cs_read = 0;
    v_line_cs_write = 0;
    vb_line_addr = r.req_addr.read();
    vb_line_wdata = r.cache_line_i.read();
    vb_line_wstrb = 0;
    v_line_wflags = 0;


    // System Bus access state machine
    switch (r.state.read()) {
    case State_Idle:
        v.mpu_er_store = 0;
        v.mpu_er_load = 0;
        v_ready_next = 1;
        break;
    case State_CheckHit:
        vb_resp_data = vb_cached_data;
        if (line_rflags_o.read()[TAG_FL_VALID] == 1) {
            // Hit
            v_resp_valid = 1;
            if (i_resp_ready.read() == 1) {
                if (r.req_write.read() == 1) {
                    // Modify tagged mem output with request and write back
                    v_line_cs_write = 1;
                    v_line_wflags[TAG_FL_VALID] = 1;
                    v_line_wflags[DTAG_FL_DIRTY] = 1;
                    v.req_write = 0;
                    vb_line_wstrb = vb_line_rdata_o_wstrb;
                    vb_line_wdata = vb_line_rdata_o_modified;
                    if (coherence_ena_ && line_rflags_o.read()[DTAG_FL_SHARED] == 1) {
                        // Make line: 'shared' -> 'unique' using write request
                        v_resp_valid = 0;
                        v.req_mem_valid = 1;
                        v.req_mem_type = WriteLineUnique();
                        v.mem_addr = line_raddr_o.read()(CFG_CPU_ADDR_BITS-1,
                                    CFG_DLOG2_BYTES_PER_LINE) << CFG_DLOG2_BYTES_PER_LINE;
                        v.mem_wstrb = vb_line_rdata_o_wstrb;
                        v.cache_line_o = vb_line_rdata_o_modified;
                        v.state = State_WaitGrantMakeUniqueL2;
                    } else {
                        if (v_req_same_line == 1) {
                            // Write address is the same as the next requested, so use it to write
                            // value and update state machine
                            v_ready_next = 1;
                        }
                        v.state = State_Idle;
                    }
                } else {
                    v_ready_next = 1;
                    v.state = State_Idle;
                }
            }
        } else {
            // Miss
            v.state = State_TranslateAddress;
        }
        break;
    case State_TranslateAddress:
        if (r.req_write.read() == 1
            && i_mpu_flags.read()[CFG_MPU_FL_WR] == 0) {
            v.mpu_er_store = 1;
            t_cache_line_i = 0;
            v.cache_line_i = ~t_cache_line_i;
            v.state = State_CheckResp;
        } else if (r.req_write.read() == 0
                    && i_mpu_flags.read()[CFG_MPU_FL_RD] == 0) {
            v.mpu_er_load = 1;
            t_cache_line_i = 0;
            v.cache_line_i = ~t_cache_line_i;
            v.state = State_CheckResp;
        } else {
            v.req_mem_valid = 1;
            v.state = State_WaitGrant;
            if (i_mpu_flags.read()[CFG_MPU_FL_CACHABLE] == 1) {
                // Cached:
                if (line_rflags_o.read()[TAG_FL_VALID] == 1 &&
                    line_rflags_o.read()[DTAG_FL_DIRTY] == 1) {
                    v.write_first = 1;
                    v.req_mem_type = WriteBack();
                    v.mem_addr = line_raddr_o.read()(CFG_CPU_ADDR_BITS-1,
                                CFG_DLOG2_BYTES_PER_LINE) << CFG_DLOG2_BYTES_PER_LINE;
                } else {
                    // 1. Read -> Save cache
                    // 2. Read -> Modify -> Save cache
                    v.mem_addr = r.req_addr.read()(CFG_CPU_ADDR_BITS-1,
                                CFG_DLOG2_BYTES_PER_LINE) << CFG_DLOG2_BYTES_PER_LINE;
                    if (r.req_write.read() == 1) {
                        v.req_mem_type = ReadMakeUnique();
                    } else {
                        v.req_mem_type = ReadShared();
                    }
                }
                v.mem_wstrb = ~0ul;
                v.cache_line_o = line_rdata_o;
            } else {
                // Uncached read/write
                v.mem_addr = r.req_addr.read()(CFG_CPU_ADDR_BITS-1, 3) << 3;
                v.mem_wstrb = (0, r.req_wstrb.read());
                if (r.req_write.read() == 1) {
                    v.req_mem_type = WriteNoSnoop();
                } else {
                    v.req_mem_type = ReadNoSnoop();
                }
                t_cache_line_i = 0;
                t_cache_line_i(63, 0) = r.req_wdata.read();
                v.cache_line_o = t_cache_line_i;
            }
        }

        v.cache_line_i = 0;
        v.load_fault = 0;
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            if (r.write_flush.read() == 1 ||
                r.write_first.read() == 1 ||
                (r.req_write.read() == 1 &&
                    r.req_mem_type.read()[REQ_MEM_TYPE_CACHED] == 0)) {
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
        if (i_mem_data_valid.read()) {
            v.cache_line_i = i_mem_data.read();
            v.state = State_CheckResp;
            if (i_mem_load_fault.read() == 1) {
                v.load_fault = 1;
            }
        }
        break;
    case State_CheckResp:
        if (r.req_mem_type.read()[REQ_MEM_TYPE_CACHED] == 0 ||
            r.load_fault.read() == 1) {
            // uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid = 1;
            vb_resp_data = vb_uncached_data;
            v_resp_er_store_fault = r.load_fault && r.req_write;
            v_resp_er_load_fault = r.load_fault && !r.req_write;
            if (i_resp_ready.read() == 1) {
                v.state = State_Idle;
            }
        } else {
            v.state = State_SetupReadAdr;
            v_line_cs_write = 1;
            v_line_wflags[TAG_FL_VALID] = 1;
            v_line_wflags[DTAG_FL_SHARED] = 1;
            vb_line_wstrb = ~0ul;  // write full line
            if (r.req_write.read() == 1) {
                // Modify tagged mem output with request before write
                v.req_write = 0;
                v_line_wflags[DTAG_FL_DIRTY] = 1;
                v_line_wflags[DTAG_FL_SHARED] = 0;
                vb_line_wdata = vb_cache_line_i_modified;
                v_resp_valid = 1;
                v.state = State_Idle;
            }
        }
        break;
    case State_SetupReadAdr:
        v.state = State_CheckHit;
        break;
    case State_WriteBus:
        if (i_mem_data_valid.read()) {
            if (r.write_flush.read() == 1) {
                // Offloading Cache line on flush request
                v.state = State_FlushAddr;
            } else if (r.write_first.read() == 1) {
                // Obsolete line was offloaded, now read new line
                v.mem_addr = r.req_addr.read()(CFG_CPU_ADDR_BITS-1, CFG_DLOG2_BYTES_PER_LINE)
                            << CFG_DLOG2_BYTES_PER_LINE;
                v.req_mem_valid = 1;
                v.write_first = 0;
                if (r.req_write.read() == 1) {
                    // read request: read-modify-save cache line
                    v.req_mem_type = ReadMakeUnique();
                } else {
                    v.req_mem_type = ReadShared();
                }
                v.state = State_WaitGrant;
            } else {
                // Non-cached write
                v.state = State_Idle;
                v_resp_valid = 1;
                v_resp_er_store_fault = i_mem_store_fault.read();
            }
        }
        break;
    case State_FlushAddr:
        v.state = State_FlushCheck;
        v_direct_access = r.req_flush_all;      // 0=only if hit; 1=will be applied ignoring hit
        v_invalidate = 1;                       // generate: wstrb='1; wflags='0
        v.write_flush = 0;
        v.cache_line_i = 0;
        break;
    case State_FlushCheck:
        v.cache_line_o = line_rdata_o.read();
        v_direct_access = r.req_flush_all;
        v_line_cs_write = r.req_flush_all;
        if (line_rflags_o.read()[TAG_FL_VALID] == 1 &&
            line_rflags_o.read()[DTAG_FL_DIRTY] == 1) {
            /** Off-load valid line */
            v.write_flush = 1;
            v.mem_addr = line_raddr_o.read();
            v.req_mem_valid = 1;
            v.req_mem_type = WriteBack();
            v.mem_wstrb = ~0ul;
            v.state = State_WaitGrant;
        } else {
            /** Write clean line */
            v.state = State_FlushAddr;
            if (r.flush_cnt.read().or_reduce() == 0) {
                v.state = State_Idle;
                v_flush_end = 1;
            }
        }
        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = r.flush_cnt.read() - 1;
            if (r.req_flush_all == 1) {
                v.req_addr = vb_addr_direct_next;
            } else {
                v.req_addr = r.req_addr.read() + DCACHE_BYTES_PER_LINE;
            }
        }
        break;
    case State_Reset:
        /** Write clean line */
        v_direct_access = 1;
        v_invalidate = 1;                       // generate: wstrb='1; wflags='0
        v.state = State_ResetWrite;
        break;
    case State_ResetWrite:
        v_direct_access = 1;
        v_line_cs_write = 1;
        v.state = State_Reset;
        if (r.flush_cnt.read().or_reduce() == 1) {
            v.flush_cnt = r.flush_cnt.read() - 1;
            v.req_addr = vb_addr_direct_next;
        } else {
            v.state = State_Idle;
        }
        break;

    case State_WaitGrantMakeUniqueL2:
        if (i_req_mem_ready.read()) {
            v.req_mem_valid = 0;
            v.state = State_WaitRespMakeUniqueL2;
        }
        break;
    case State_WaitRespMakeUniqueL2:
        if (i_mem_data_valid.read()) {
            v_resp_valid = 1;
            v.state = State_Idle;
        }
        break;
    case State_SnoopSetupAddr:
        if (r.req_snoop_type.read()[SNOOP_REQ_TYPE_READDATA] == 1) {
            v.state = State_SnoopReadData;
            v_invalidate = 1; 
        } else if (r.req_snoop_type.read()[SNOOP_REQ_TYPE_MAKEINVALID] == 1) {
            v_resp_snoop_valid = 1;
            v_invalidate = 1; 
            v.flush_cnt = 0;
            v.req_flush_all = 0;            // invalid only 'hit' line
            v.state = State_FlushCheck;     // address already setup
        } else {
            v_resp_snoop_valid = 1;
            v.state = State_Idle;
        }
        break;
    case State_SnoopReadData:
        v_resp_snoop_valid = 1;
        v.state = State_Idle;
        break;
    default:;
    }

    v_req_snoop_ready =
        (line_snoop_ready_o.read() && !i_req_snoop_type.read().or_reduce()) ||
        (coherence_ena_ && v_ready_next && i_req_snoop_type.read().or_reduce());

    v.snoop_flags_valid = i_req_snoop_valid.read() &&
        line_snoop_ready_o.read() && !i_req_snoop_type.read().or_reduce();

    if (v_ready_next == 1) {
        if (coherence_ena_ &&
            i_req_snoop_valid.read() == 1 && i_req_snoop_type.read() != 0x0) {
            // Access cache data
            vb_line_addr = i_req_snoop_addr.read();
            v.req_addr = i_req_snoop_addr.read();
            v.req_snoop_type = i_req_snoop_type.read();
            v.state = State_SnoopSetupAddr;
        } else if (r.req_flush.read() == 1) {
            v.state = State_FlushAddr;
            v.req_flush = 0;
            v.cache_line_i = 0;
            v.req_addr = r.req_flush_addr.read() & ~((1<<CFG_DLOG2_BYTES_PER_LINE)-1);
            v.flush_cnt = r.req_flush_cnt.read();
        } else {
            v_req_ready = 1;
            v_line_cs_read = i_req_valid.read();
            vb_line_addr = i_req_addr.read();
            if (i_req_valid.read() == 1) {
                v.req_addr = i_req_addr.read();
                v.req_wstrb = i_req_wstrb.read();
                v.req_wdata = i_req_wdata.read();
                v.req_write = i_req_write.read();
                v.state = State_CheckHit;
            }
        }
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }


    line_direct_access_i = v_direct_access;
    line_invalidate_i = v_invalidate;
    line_re_i = v_line_cs_read;
    line_we_i = v_line_cs_write;
    line_addr_i = vb_line_addr;
    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = v_line_wflags;
    line_snoop_addr_i = i_req_snoop_addr;

    o_req_ready = v_req_ready;

    o_req_mem_valid = r.req_mem_valid.read();
    o_req_mem_addr = r.mem_addr.read();
    o_req_mem_type = r.req_mem_type.read();
    o_req_mem_strob = r.mem_wstrb.read();
    o_req_mem_data = r.cache_line_o.read();

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_addr = r.req_addr.read();
    o_resp_er_addr = r.req_addr.read();
    o_resp_er_load_fault = v_resp_er_load_fault;
    o_resp_er_store_fault = v_resp_er_store_fault;
    o_resp_er_mpu_load = r.mpu_er_load;
    o_resp_er_mpu_store = r.mpu_er_store;
    o_mpu_addr = r.req_addr.read();

    o_req_snoop_ready = v_req_snoop_ready;
    o_resp_snoop_valid = v_resp_snoop_valid;
    o_resp_snoop_data = line_rdata_o;
    o_resp_snoop_flags = line_snoop_flags_o;

    o_flush_end = v_flush_end;
    o_state = r.state;
}

void DCacheLru::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

#ifdef DBG_DCACHE_LRU_TB
DCacheLru_tb::DCacheLru_tb(sc_module_name name_) : sc_module(name_),
    w_clk("clk0", 10, SC_NS) {
    SC_METHOD(comb0);
    sensitive << w_nrst;
    sensitive << r.clk_cnt;

    SC_METHOD(comb_fetch);
    sensitive << w_nrst;
    sensitive << w_req_ready;
    sensitive << w_resp_valid;
    sensitive << wb_resp_addr;
    sensitive << wb_resp_data;
    sensitive << w_resp_er_load_fault;
    sensitive << w_resp_er_store_fault;
    sensitive << w_resp_er_mpu_load;
    sensitive << w_resp_er_mpu_store;
    sensitive << r.clk_cnt;

    SC_METHOD(comb_bus);
    sensitive << w_nrst;
    sensitive << w_req_mem_valid;
    sensitive << w_req_mem_write;
    sensitive << wb_req_mem_addr;
    sensitive << wb_req_mem_strob;
    sensitive << wb_req_mem_data;
    sensitive << wb_mpu_addr;
    sensitive << rbus.state;
    sensitive << rbus.mpu_addr;
    sensitive << rbus.burst_addr;
    sensitive << rbus.burst_cnt;

    SC_METHOD(registers);
    sensitive << w_clk.posedge_event();

    tt = new DCacheLru("tt", 0);
    tt->i_clk(w_clk);
    tt->i_nrst(w_nrst);
    tt->i_req_valid(w_req_valid);
    tt->i_req_write(w_req_write);
    tt->i_req_addr(wb_req_addr);
    tt->i_req_wdata(wb_req_wdata);
    tt->i_req_wstrb(wb_req_wstrb);
    tt->o_req_ready(w_req_ready);
    tt->o_resp_valid(w_resp_valid);
    tt->o_resp_addr(wb_resp_addr);
    tt->o_resp_data(wb_resp_data);
    tt->o_resp_er_addr(wb_resp_er_addr);
    tt->o_resp_er_load_fault(w_resp_er_load_fault);
    tt->o_resp_er_store_fault(w_resp_er_store_fault);
    tt->o_resp_er_mpu_load(w_resp_er_mpu_load);
    tt->o_resp_er_mpu_store(w_resp_er_mpu_store);
    tt->i_resp_ready(w_resp_ready);
    // memory interface
    tt->i_req_mem_ready(w_req_mem_ready);
    tt->o_req_mem_valid(w_req_mem_valid);
    tt->o_req_mem_write(w_req_mem_write);
    tt->o_req_mem_addr(wb_req_mem_addr);
    tt->o_req_mem_strob(wb_req_mem_strob);
    tt->o_req_mem_data(wb_req_mem_data);
    tt->o_req_mem_len(wb_req_mem_len);
    tt->o_req_mem_burst(wb_req_mem_burst);
    tt->o_req_mem_last(w_req_mem_last);
    tt->i_mem_data_valid(w_mem_data_valid);
    tt->i_mem_data(wb_mem_data);
    tt->i_mem_load_fault(w_mem_load_fault);
    tt->i_mem_store_fault(w_mem_store_fault);
    // MPU interface
    tt->o_mpu_addr(wb_mpu_addr);
    tt->i_mpu_flags(w_mpu_flags);
    // Debug interface
    tt->i_flush_address(wb_flush_address);
    tt->i_flush_valid(w_flush_valid);
    tt->o_state(wb_state);

    tb_vcd = sc_create_vcd_trace_file("DCacheLru_tb");
    tb_vcd->set_time_unit(1, SC_PS);
    sc_trace(tb_vcd, w_nrst, "w_nrst");
    sc_trace(tb_vcd, w_clk, "w_clk");
    sc_trace(tb_vcd, r.clk_cnt, "clk_cnt");
    sc_trace(tb_vcd, w_req_valid, "w_req_valid");
    sc_trace(tb_vcd, w_req_write, "w_req_write");
    sc_trace(tb_vcd, wb_req_addr, "wb_req_addr");
    sc_trace(tb_vcd, wb_req_wdata, "wb_req_wdata");
    sc_trace(tb_vcd, wb_req_wstrb, "wb_req_wstrb");
    sc_trace(tb_vcd, w_req_ready, "w_req_ready");
    sc_trace(tb_vcd, w_resp_valid, "w_resp_valid");
    sc_trace(tb_vcd, wb_resp_addr, "wb_resp_addr");
    sc_trace(tb_vcd, wb_resp_data, "wb_resp_data");
    sc_trace(tb_vcd, w_resp_ready, "w_resp_ready");
    sc_trace(tb_vcd, w_req_mem_ready, "w_req_mem_ready");
    sc_trace(tb_vcd, w_req_mem_valid, "w_req_mem_valid");
    sc_trace(tb_vcd, w_req_mem_write, "w_req_mem_write");
    sc_trace(tb_vcd, wb_req_mem_addr, "wb_req_mem_addr");
    sc_trace(tb_vcd, wb_req_mem_strob, "wb_req_mem_strob");
    sc_trace(tb_vcd, wb_req_mem_data, "wb_req_mem_data");
    sc_trace(tb_vcd, w_mem_data_valid, "w_resp_mem_data_valid");
    sc_trace(tb_vcd, wb_mem_data, "wb_resp_mem_data");
    sc_trace(tb_vcd, wb_state, "wb_state");
    sc_trace(tb_vcd, rbus.burst_addr, "rbus_burst_addr");
    sc_trace(tb_vcd, rbus.burst_cnt, "rbus_burst_cnt");

    tt->generateVCD(tb_vcd, tb_vcd);
}


void DCacheLru_tb::comb0() {
    v = r;
    v.clk_cnt = r.clk_cnt.read() + 1;

    if (r.clk_cnt.read() < 10) {
        w_nrst = 0;
    } else {
        w_nrst = 1;
    }

}


void DCacheLru_tb::comb_fetch() {
    sc_uint<CFG_MPU_FL_TOTAL> v_mpu_flags;
    w_req_valid = 0;
    w_req_write = 0;
    wb_req_addr = 0;
    wb_req_wdata = 0;
    wb_req_wstrb = 0;
    w_resp_ready = 1;

    v_mpu_flags = ~0u;

    w_flush_valid = 0;
    wb_flush_address = 0;


    if (rbus.mpu_addr.read()[31] == 1) {
        v_mpu_flags[CFG_MPU_FL_CACHABLE] = 0;
    }
    w_mpu_flags = v_mpu_flags;

    // flush duration in clocks
    // 1 clock to setup adr + 1 clock to write line for each line
    // Total number of lines = ways + line_per_way
    const unsigned START_POINT = 10 + 1 + (1 << (1+CFG_DLOG2_NWAYS+CFG_DLOG2_LINES_PER_WAY));
    const unsigned START_POINT2 = START_POINT + 500;

    switch (r.clk_cnt.read()) {
    case START_POINT:
        w_req_valid = 1;
        wb_req_addr = 0x00000008;
        break;

    case START_POINT + 10:
        w_req_valid = 1;
        wb_req_addr = 0x00010008;
        break;

    case START_POINT + 25:
        w_req_valid = 1;
        wb_req_addr = 0x00011008;
        break;

    case START_POINT + 40:
        w_req_valid = 1;
        wb_req_addr = 0x00012008;
        break;

    case START_POINT + 55:
        w_req_valid = 1;
        wb_req_addr = 0x00013010;
        break;


    case START_POINT2:
        /** Cached Write to loaded cache line without bus access */
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x00012008;
        wb_req_wdata = 0x000000000000CC00ull;
        wb_req_wstrb = 0x02;
        break;

    case START_POINT2 + 10:
        /** Uncached write directly on bus */
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x80012008;
        wb_req_wdata = 0x000000000000CCBBull;
        wb_req_wstrb = 0x0F;
        break;

    case START_POINT2 + 20:
        /** Uncached read directly on bus */
        w_req_valid = 1;
        w_req_write = 0;
        wb_req_addr = 0x80012010;
        break;

    case START_POINT2 + 30:
        /** Cached Write to notloaded cache line without displacement:
               - load line
               - modify line
               - store to cache
         */
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x00000028;
        wb_req_wdata = 0x00000000BBAA0000ull;
        wb_req_wstrb = 0x0C;
        break;

    case START_POINT2 + 50:
        w_req_valid = 1;
        w_req_write = 0;
        wb_req_addr = 0x00000020;
        break;
    case START_POINT2 + 51:
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x00000028;
        wb_req_wdata = 0x0000DDCC00000000ull;
        wb_req_wstrb = 0x30;
        break;
    case START_POINT2 + 52:
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x00000028;
        wb_req_wdata = 0xFF00DDCC00000000ull;
        wb_req_wstrb = 0x80;
        break;
    case START_POINT2 + 53:
        w_req_valid = 1;
        w_req_write = 0;
        wb_req_addr = 0x00000028;
        break;
    case START_POINT2 + 54:
        w_req_valid = 1;
        w_req_write = 0;
        wb_req_addr = 0x00000020;
        break;

#if 0
    case START_POINT2 + 50:
        /** Cached Write to notloaded cache line with displacement:
               - store to memory displaced line
               - load line
               - modify line
               - store to cache
         */
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x00021008;   // tag 0x11 should be replaced with 0x21
        wb_req_wdata = 0xFFEEDDCC00000000ull;
        wb_req_wstrb = 0xF0;
        break;
#endif
    case START_POINT2 + 500:
        /** Flush cache (must dirty bits to offload)
         */
        w_flush_valid = 1;
        wb_flush_address = 0x1; // flush all
        break;

    default:;
    }
}

void DCacheLru_tb::comb_bus() {
    vbus = rbus;

    w_req_mem_ready = 0;
    w_mem_data_valid = 0;
    wb_mem_data = 0;
    w_mem_load_fault = 0;
    w_mem_store_fault = 0;

    switch (rbus.state.read()) {
    case BUS_Idle:
        w_req_mem_ready = 1;
        if (w_req_mem_valid.read() == 1) {
            if (wb_req_mem_len.read() == 0) {
                vbus.state = BUS_ReadLast;
            } else {
                vbus.state = BUS_Read;
            }
            vbus.burst_addr = wb_req_mem_addr.read();
            vbus.burst_cnt = wb_req_mem_len;
        }
        break;
    case BUS_Read:
        w_mem_data_valid = 1;
        wb_mem_data = 0x2000000010000000ull + rbus.burst_addr.read();
        vbus.burst_cnt = rbus.burst_cnt.read() - 1;
        vbus.burst_addr = rbus.burst_addr.read() + 8;
        if (rbus.burst_cnt.read() == 1) {
            vbus.state = BUS_ReadLast;
        }
        break;
    case BUS_ReadLast:
        w_req_mem_ready = 1;
        w_mem_data_valid = 1;
        wb_mem_data = 0x2000000010000000ull + rbus.burst_addr.read();
        if (w_req_mem_valid.read() == 1) {
            if (wb_req_mem_len.read() == 0) {
                vbus.state = BUS_ReadLast;
            } else {
                vbus.state = BUS_Read;
            }
            vbus.burst_addr = wb_req_mem_addr.read();
            vbus.burst_cnt = wb_req_mem_len;
        } else {
            vbus.state = BUS_Idle;
            vbus.burst_cnt = 0;
        }
        break;
    default:;
    }

    vbus.mpu_addr = wb_mpu_addr.read();

    if (w_nrst.read() == 0) {
        vbus.state = BUS_Idle;
        vbus.mpu_addr = 0;
        vbus.burst_addr = 0;
        vbus.burst_cnt = 0;
    }
}

#endif

}  // namespace debugger

