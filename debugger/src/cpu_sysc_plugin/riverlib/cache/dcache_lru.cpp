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

DCacheLru::DCacheLru(sc_module_name name_, bool async_reset)
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
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    o_req_mem_len("o_req_mem_len"),
    o_req_mem_burst("o_req_mem_burst"),
    o_req_mem_last("o_req_mem_last"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data("i_mem_data"),
    i_mem_load_fault("i_mem_load_fault"),
    i_mem_store_fault("i_mem_store_fault"),
    o_mpu_addr("o_mpu_addr"),
    i_mpu_cachable("i_mpu_cachable"),
    i_mpu_writable("i_mpu_writable"),
    i_mpu_readable("i_mpu_readable"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    o_state("o_state") {
    async_reset_ = async_reset;

    mem = new TagMemNWay<BUS_ADDR_WIDTH,
                         CFG_DLOG2_NWAYS,
                         CFG_DLOG2_LINES_PER_WAY,
                         CFG_DLOG2_BYTES_PER_LINE,
                         DTAG_FL_TOTAL>("mem0", async_reset);
    mem->i_clk(i_clk);
    mem->i_nrst(i_nrst);
    mem->i_cs(line_cs_i);
    mem->i_flush(line_flush_i);
    mem->i_addr(line_addr_i);
    mem->i_wdata(line_wdata_i);
    mem->i_wstrb(line_wstrb_i);
    mem->i_wflags(line_wflags_i);
    mem->o_raddr(line_raddr_o);
    mem->o_rdata(line_rdata_o);
    mem->o_rflags(line_rflags_o);
    mem->o_hit(line_hit_o);


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
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << i_req_mem_ready;
    sensitive << i_mpu_cachable;
    sensitive << i_mpu_writable;
    sensitive << i_mpu_readable;
    sensitive << line_raddr_o;
    sensitive << line_rdata_o;
    sensitive << line_hit_o;
    sensitive << line_rflags_o;
    sensitive << r.requested;
    sensitive << r.req_addr;
    sensitive << r.req_addr_b_resp;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.mem_write;
    sensitive << r.mem_addr;
    sensitive << r.burst_cnt;
    sensitive << r.burst_rstrb;
    sensitive << r.cached;
    sensitive << r.load_fault;
    sensitive << r.write_first;
    sensitive << r.write_flush;
    sensitive << r.mem_wstrb;
    sensitive << r.req_flush;
    sensitive << r.req_flush_addr;
    sensitive << r.req_flush_cnt;
    sensitive << r.flush_cnt;
    sensitive << r.cache_line_i;
    sensitive << r.cache_line_o;
    sensitive << r.init;

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
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, o_req_mem_len, o_req_mem_len.name());
        sc_trace(o_vcd, o_req_mem_burst, o_req_mem_burst.name());
        sc_trace(o_vcd, o_req_mem_last, o_req_mem_last.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_load_fault, i_mem_load_fault.name());
        sc_trace(o_vcd, i_mem_store_fault, i_mem_store_fault.name());

        sc_trace(o_vcd, i_mpu_cachable, i_mpu_cachable.name());
        sc_trace(o_vcd, i_flush_address, i_flush_address.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, o_state, o_state.name());

        std::string pn(name());
        sc_trace(o_vcd, r.requested, pn + ".r_requested");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, line_addr_i, pn + ".linei_addr_i");
        sc_trace(o_vcd, line_wstrb_i, pn + ".linei_wstrb_i");
        sc_trace(o_vcd, line_raddr_o, pn + ".line_raddr_o");
        sc_trace(o_vcd, line_rdata_o, pn + ".line_rdata_o");
        sc_trace(o_vcd, r.load_fault, pn + ".r_load_fault");
        sc_trace(o_vcd, r.cached, pn + ".r_cached");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
        sc_trace(o_vcd, r.cache_line_o, pn + ".r_cache_line_o");
        sc_trace(o_vcd, r.write_first, pn + ".r_write_first");
        sc_trace(o_vcd, r.write_flush, pn + ".r_write_flush");
        sc_trace(o_vcd, r.burst_cnt, pn + ".r_burst_cnt");
        sc_trace(o_vcd, r.mem_wstrb, pn + ".r_mem_wstrb");
    }
    mem->generateVCD(i_vcd, o_vcd);
}

void DCacheLru::comb() {
    sc_biguint<8*(1<<CFG_DLOG2_BYTES_PER_LINE)> vb_cache_line_i_modified;
    sc_biguint<8*(1<<CFG_DLOG2_BYTES_PER_LINE)> vb_line_rdata_o_modified;
    sc_uint<(1<<CFG_DLOG2_BYTES_PER_LINE)> vb_line_rdata_o_wstrb;
    
    bool v_last;
    bool v_req_ready;
    sc_uint<8> v_req_mem_len;
    sc_biguint<8*(1<<CFG_DLOG2_BYTES_PER_LINE)> t_cache_line_i;
    sc_uint<BUS_DATA_WIDTH> vb_cached_data;
    sc_uint<BUS_DATA_WIDTH> vb_uncached_data;
    bool v_resp_valid;
    sc_uint<BUS_DATA_WIDTH> vb_resp_data;
    bool v_resp_er_load_fault;
    bool v_resp_er_mpu_load;
    bool v_resp_er_mpu_store;
    bool v_flush;
    bool v_line_cs;
    sc_uint<BUS_ADDR_WIDTH> vb_line_addr;
    sc_biguint<8*(1<<CFG_DLOG2_BYTES_PER_LINE)> vb_line_wdata;
    sc_uint<(1<<CFG_DLOG2_BYTES_PER_LINE)> vb_line_wstrb;
    sc_biguint<BUS_DATA_WIDTH> vb_req_mask;
    sc_uint<DTAG_FL_TOTAL> v_line_wflags;
    sc_uint<BUS_ADDR_WIDTH> vb_err_addr;
    sc_uint<DCACHE_LOG2_BURST_LEN> ridx;
   
    v = r;

    v_req_ready = 0;
    v_resp_valid = 0;
    v_resp_er_load_fault = 0;
    v_resp_er_mpu_load = 0;
    v_resp_er_mpu_store = 0;
    v_flush = 0;
    v_last = 0;
    v_req_mem_len = DCACHE_BURST_LEN-1;
    ridx = r.req_addr.read()(CFG_DLOG2_BYTES_PER_LINE-1, CFG_LOG2_DATA_BYTES);

    vb_cached_data = line_rdata_o.read()((ridx+1)*BUS_DATA_WIDTH - 1,
                                         ridx*BUS_DATA_WIDTH);
    vb_uncached_data = r.cache_line_i.read()(BUS_DATA_WIDTH-1, 0);

    if (i_mem_store_fault.read() == 1) {
        vb_err_addr = r.req_addr_b_resp.read();
    } else {
        vb_err_addr = r.req_addr.read();
    }


    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
        if (i_flush_address.read()[0] == 1) {
            v.req_flush_cnt = ~0u;
            v.req_flush_addr = 0;
        } else {
            v.req_flush_cnt = 0;
            v.req_flush_addr = i_flush_address.read();
        }
    }

    vb_req_mask = 0;
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (r.req_wstrb.read()[i] == 1) {
            vb_req_mask(8*i+7, 8*i) = ~0u;
        }
    }

    vb_line_rdata_o_modified = line_rdata_o.read();
    vb_cache_line_i_modified = r.cache_line_i.read();
    vb_line_rdata_o_wstrb = 0;
    for (int i = 0; i < DCACHE_BURST_LEN; i++) {
        if (i != ridx.to_int()) {
            continue;
        }
        vb_line_rdata_o_modified(BUS_DATA_WIDTH*(i+1)-1, BUS_DATA_WIDTH*i) =
            (vb_line_rdata_o_modified(BUS_DATA_WIDTH*(i+1)-1, BUS_DATA_WIDTH*i)
             & ~vb_req_mask) | (r.req_wdata.read() & vb_req_mask);

        vb_cache_line_i_modified(BUS_DATA_WIDTH*(i+1)-1, BUS_DATA_WIDTH*i) =
            (vb_cache_line_i_modified(BUS_DATA_WIDTH*(i+1)-1, BUS_DATA_WIDTH*i)
             & ~vb_req_mask) | (r.req_wdata.read() & vb_req_mask);

        vb_line_rdata_o_wstrb(BUS_DATA_BYTES*(i+1)-1, BUS_DATA_BYTES*i) =
            r.req_wstrb.read();
    }
    
    v_line_cs = 0;
    vb_line_addr = r.req_addr.read();
    vb_line_wdata = r.cache_line_i.read();
    vb_line_wstrb = 0;
    v_line_wflags = 0;


    // System Bus access state machine
    switch (r.state.read()) {
    case State_Idle:
        if (r.req_flush.read() == 1) {
            v.state = State_FlushAddr;
            v.req_flush = 0;
            t_cache_line_i = 0;
            v.cache_line_i = ~t_cache_line_i;
            if (r.req_flush_addr.read()[0] == 1) {
                v.req_addr = 0;
                v.flush_cnt = ~0u;
            } else {
                v.req_addr = r.req_flush_addr.read() & ~LOG2_DATA_BYTES_MASK;
                v.flush_cnt = r.req_flush_cnt.read();
            }
        } else {
            v_line_cs = i_req_valid.read();
            v_req_ready = 1;
            vb_line_addr = i_req_addr.read();
            if (i_req_valid.read() == 1) {
                v.requested = 1;
                v.req_addr = i_req_addr.read();
                v.req_wstrb = i_req_wstrb.read();
                v.req_wdata = i_req_wdata.read();
                v.req_write = i_req_write.read();
                v.state = State_CheckHit;
            } else {
                v.requested = 0;
            }
        }
        break;
    case State_CheckHit:
        vb_resp_data = vb_cached_data;
        if (line_hit_o.read() == 1) {
            // Hit
            v_resp_valid = 1;
            if (r.req_write.read() == 1) {
                // Modify tagged mem output with request and write back
                v_line_cs = 1;
                v_line_wflags[TAG_FL_VALID] = 1;
                v_line_wflags[DTAG_FL_DIRTY] = 1;
                v.req_write = 0;
                vb_line_wstrb = vb_line_rdata_o_wstrb;
                vb_line_wdata = vb_line_rdata_o_modified;
                if (i_resp_ready.read() == 0) {
                    // Do nothing: wait accept
                } else {
                    v.state = State_Idle;
                    v.requested = 0;
                }
            } else {
                v_req_ready = 1;
                if (i_resp_ready.read() == 0) {
                    // Do nothing: wait accept
                } else if (i_req_valid.read() == 1) {
                    v.state = State_CheckHit;
                    v_line_cs = i_req_valid.read();
                    v.req_addr = i_req_addr.read();
                    v.req_wstrb = i_req_wstrb.read();
                    v.req_wdata = i_req_wdata.read();
                    v.req_write = i_req_write.read();
                    vb_line_addr = i_req_addr.read();
                } else {
                    v.state = State_Idle;
                    v.requested = 0;
                }
            }
        } else {
            // Miss
            v.state = State_CheckMPU;
        }
        break;
    case State_CheckMPU:
        v.req_mem_valid = 1;
        v.mem_write = 0;
        v.state = State_WaitGrant;

        if (i_mpu_cachable.read() == 1) {
            if (line_rflags_o.read()[TAG_FL_VALID] == 1 &&
                line_rflags_o.read()[DTAG_FL_DIRTY] == 1) {
                v.write_first = 1;
                v.mem_write = 1;
                v.mem_addr = line_raddr_o.read()(BUS_ADDR_WIDTH-1,
                            CFG_DLOG2_BYTES_PER_LINE) << CFG_DLOG2_BYTES_PER_LINE;
            } else {
                v.mem_addr = r.req_addr.read()(BUS_ADDR_WIDTH-1,
                            CFG_DLOG2_BYTES_PER_LINE) << CFG_DLOG2_BYTES_PER_LINE;
            }
            v.mem_wstrb = ~0ul;
            v.burst_cnt = DCACHE_BURST_LEN-1;
            v.cached = 1;
            v.cache_line_o = line_rdata_o;
        } else {
            v.mem_addr = r.req_addr.read()(BUS_ADDR_WIDTH-1, CFG_LOG2_DATA_BYTES)
                         << CFG_LOG2_DATA_BYTES;
            v.mem_wstrb = r.req_wstrb.read();
            v.mem_write = r.req_write.read();
            v.burst_cnt = 0;
            v.cached = 0;
            v_req_mem_len = 0;
            t_cache_line_i = 0;
            t_cache_line_i(BUS_DATA_WIDTH-1, 0) = r.req_wdata.read();
            v.cache_line_o = t_cache_line_i;
        }
        v.burst_rstrb = 0x1;
        v.cache_line_i = 0;
        v.load_fault = 0;
        v.writable = i_mpu_writable.read();
        v.readable = i_mpu_readable.read();
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            if (r.write_flush.read() == 1 ||
                r.write_first.read() == 1 ||
                (r.req_write.read() == 1 && r.cached.read() == 0)) {
                v.state = State_WriteBus;
            } else {
                v.state = State_WaitResp;
            }
            v.req_mem_valid = 0;
        }
        if (r.cached.read() == 0) {
            v_req_mem_len = 0;
        }
        break;
    case State_WaitResp:
        if (r.burst_cnt.read() == 0) {
            v_last = 1;
        }
        if (i_mem_data_valid.read()) {
            t_cache_line_i = r.cache_line_i.read();
            for (int k = 0; k < DCACHE_BURST_LEN; k++) {
                if (r.burst_rstrb.read()[k] == 1) {
                    t_cache_line_i((k+1)*BUS_DATA_WIDTH-1,
                                    k*BUS_DATA_WIDTH) = i_mem_data.read();
                }
            }
            v.cache_line_i = t_cache_line_i;
            if (r.burst_cnt.read() == 0) {
                v.state = State_CheckResp;
            } else {
                v.burst_cnt = r.burst_cnt.read() - 1;
            }
            v.burst_rstrb = r.burst_rstrb.read() << 1;
            if (i_mem_load_fault.read() == 1) {
                v.load_fault = 1;
            }
        }
        break;
    case State_CheckResp:
        if (r.cached.read() == 1) {
            v.state = State_SetupReadAdr;
            v_line_cs = 1;
            v_line_wflags[TAG_FL_VALID] = 1;
            vb_line_wstrb = ~0ul;  // write full line
            if (r.req_write.read() == 1) {
                // Modify tagged mem output with request before write
                v.req_write = 0;
                v_line_wflags[DTAG_FL_DIRTY] = 1;
                vb_line_wdata = vb_cache_line_i_modified;
            }
        } else {
            v_resp_valid = 1;
            vb_resp_data = vb_uncached_data;
            v_resp_er_load_fault = r.load_fault;
            if (i_resp_ready.read() == 1) {
                v.state = State_Idle;
                v.requested = 0;
            }
        }
        break;
    case State_SetupReadAdr:
        v.state = State_CheckHit;
        break;
    case State_WriteBus:
        if (r.burst_cnt.read() == 0) {
            v_last = 1;
        }
        if (i_mem_data_valid.read()) {
            // Shift right to send lower part on AXI
            v.cache_line_o =
                (0, r.cache_line_o.read()(DCACHE_LINE_BITS-1, BUS_DATA_WIDTH));
            if (r.burst_cnt.read() == 0) {
                v.req_addr_b_resp = r.req_addr;
                if (r.write_flush.read() == 1) {
                    // Offloading Cache line on flush request
                    v.state = State_FlushAddr;
                } else if (r.write_first.read() == 1) {
                    v.mem_addr = r.req_addr.read()(BUS_ADDR_WIDTH-1, CFG_DLOG2_BYTES_PER_LINE)
                                << CFG_DLOG2_BYTES_PER_LINE;
                    v.req_mem_valid = 1;
                    v.burst_cnt = DCACHE_BURST_LEN-1;
                    v.write_first = 0;
                    v.mem_write = 0;
                    v.state = State_WaitGrant;
                } else {
                    // Non-cached write
                    v_resp_valid = 1;
                    v.state = State_Idle;
                }
            } else {
                v.burst_cnt = r.burst_cnt.read() - 1;
            }
            //if (i_resp_mem_store_fault.read() == 1) {
            //    v.store_fault = 1;
            //}
        }
        break;
    case State_FlushAddr:
        v_line_cs = 1;
        v_flush = 1;
        v.state = State_FlushCheck;
        v.write_flush = 0;
        v.cache_line_i = 0;
        if (r.flush_cnt.read() == 0) {
            v.state = State_Idle;
            v.init = 0;
        }
        break;
    case State_FlushCheck:
        v.cache_line_o = line_rdata_o.read();
        v_line_wflags = 0;      // flag valid = 0
        vb_line_wstrb = ~0ul;   // write full line
        v_line_cs = 1;
        v_flush = 1;

        if (r.init.read() == 0 &&
            line_rflags_o.read()[TAG_FL_VALID] == 1) {// &&
//            line_rflags_o.read()[DTAG_FL_DIRTY] == 1) {
            /** Off-load valid line */
            v.write_flush = 1;
            v.mem_addr = line_raddr_o.read();
            v.req_mem_valid = 1;
            v.burst_cnt = DCACHE_BURST_LEN-1;
            v.mem_write = 1;
            v.state = State_WaitGrant;
        } else {
            /** Write clean line */
            v.state = State_FlushAddr;
            if (r.flush_cnt.read() == 0) {
                v.state = State_Idle;
                v.init = 0;
            }
        }

        if (r.flush_cnt.read() != 0) {
            v.flush_cnt = r.flush_cnt.read() - 1;
            /** Use lsb address bits to manually select memory WAY bank: */
            if (r.req_addr.read()(CFG_DLOG2_NWAYS-1, 0) == DCACHE_WAYS-1) {
                v.req_addr = (r.req_addr.read() + (1 << CFG_DLOG2_BYTES_PER_LINE)) 
                            & ~LOG2_DATA_BYTES_MASK;
            } else {
                v.req_addr = r.req_addr.read() + 1;
            }
        }
        break;
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }


    line_cs_i = v_line_cs;
    line_addr_i = vb_line_addr;
    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = v_line_wflags;
    line_flush_i = v_flush;

    o_req_ready = v_req_ready;

    o_req_mem_valid = r.req_mem_valid.read();
    o_req_mem_addr = r.mem_addr.read();
    o_req_mem_write = r.mem_write.read();
    o_req_mem_strob = r.mem_wstrb.read();
    o_req_mem_data = r.cache_line_o.read()(BUS_DATA_WIDTH-1, 0).to_uint64();
    o_req_mem_len = v_req_mem_len;
    o_req_mem_burst = 1;    // 00=FIX; 01=INCR; 10=WRAP
    o_req_mem_last = v_last;

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_addr = r.req_addr.read();
    o_resp_er_addr = vb_err_addr;
    o_resp_er_load_fault = v_resp_er_load_fault;
    o_resp_er_store_fault = i_mem_store_fault.read();
    o_resp_er_mpu_load = v_resp_er_mpu_load;
    o_resp_er_mpu_store = v_resp_er_mpu_store;
    o_mpu_addr = r.req_addr.read();
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

    tt = new DCacheLru("tt", 0, CFG_IINDEX_WIDTH);
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
    tt->i_mpu_cachable(w_mpu_cachable);
    tt->i_mpu_writable(w_mpu_writable);
    tt->i_mpu_readable(w_mpu_readable);
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
    w_req_valid = 0;
    w_req_write = 0;
    wb_req_addr = 0;
    wb_req_wdata = 0;
    wb_req_wstrb = 0;
    w_resp_ready = 1;

    w_mpu_cachable = 1;
    w_mpu_writable = 1;
    w_mpu_readable = 1;

    w_flush_valid = 0;
    wb_flush_address = 0;


    if (rbus.mpu_addr.read()[31] == 1) {
        w_mpu_cachable = 0;
    }

    const unsigned START_POINT = 10 + 1 + (1 << (CFG_IINDEX_WIDTH+4));
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
    case START_POINT2 + 150:
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

