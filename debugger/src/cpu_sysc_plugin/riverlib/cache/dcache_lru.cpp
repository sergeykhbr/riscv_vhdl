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

DCacheLru::DCacheLru(sc_module_name name_, bool async_reset,
    int index_width) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_valid("i_req_valid"),
    i_req_write("i_req_write"),
    i_req_addr("i_req_addr"),
    i_req_wdata("i_req_wdata"),
    i_req_wstrb("i_req_wstrb"),
    o_req_ctrl_ready("o_req_ctrl_ready"),
    o_resp_ctrl_valid("o_resp_ctrl_valid"),
    o_resp_ctrl_addr("o_resp_ctrl_addr"),
    o_resp_ctrl_data("o_resp_ctrl_data"),
    o_resp_ctrl_load_fault("o_resp_ctrl_load_fault"),
    o_resp_ctrl_executable("o_resp_ctrl_executable"),
    o_resp_ctrl_writable("o_resp_ctrl_writable"),
    o_resp_ctrl_readable("o_resp_ctrl_readable"),
    i_resp_ctrl_ready("i_resp_ctrl_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    o_req_mem_len("o_req_mem_len"),
    o_req_mem_burst("o_req_mem_burst"),
    o_req_mem_last("o_req_mem_last"),
    i_resp_mem_data_valid("i_resp_mem_data_valid"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    o_mpu_addr("o_mpu_addr"),
    i_mpu_cachable("i_mpu_cachable"),
    i_mpu_executable("i_mpu_executable"),
    i_mpu_writable("i_mpu_writable"),
    i_mpu_readable("i_mpu_readable"),
    i_flush_address("i_flush_address"),
    i_flush_valid("i_flush_valid"),
    o_istate("o_istate") {
    async_reset_ = async_reset;
    index_width_ = index_width;

    char tstr1[32] = "way0";
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        tstr1[3] = '0' + static_cast<char>(i);
        wayx[i] = new DWayMem(tstr1, async_reset, i);
        wayx[i]->i_clk(i_clk);
        wayx[i]->i_nrst(i_nrst);
        wayx[i]->i_addr(way_i.addr);
        //wayx[i]->i_wena(wb_ena[i]);
        wayx[i]->i_wstrb(way_i.wstrb[i]);
        wayx[i]->i_wvalid(way_i.wvalid);
        wayx[i]->i_wdata(way_i.wdata);
        wayx[i]->i_load_fault(way_i.load_fault);
        wayx[i]->i_executable(way_i.executable);
        wayx[i]->i_readable(way_i.readable);
        wayx[i]->i_writable(way_i.writable);
        wayx[i]->o_rtag(way_o[i].rtag);
        wayx[i]->o_rdata(way_o[i].rdata);
        wayx[i]->o_valid(way_o[i].valid);
        wayx[i]->o_load_fault(way_o[i].load_fault);
        wayx[i]->o_executable(way_o[i].executable);
        wayx[i]->o_readable(way_o[i].readable);
        wayx[i]->o_writable(way_o[i].writable);
    }

    lru = new ILru("lru0");
    lru->i_clk(i_clk);
    lru->i_init(lrui.init);
    lru->i_radr(lrui.radr);
    lru->i_wadr(lrui.wadr);
    lru->i_we(lrui.we);
    lru->i_lru(lrui.lru);
    lru->o_lru(wb_lru);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_valid;
    sensitive << i_req_write;
    sensitive << i_req_addr;
    sensitive << i_req_wdata;
    sensitive << i_req_wstrb;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << i_resp_ctrl_ready;
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << i_req_mem_ready;
    sensitive << i_mpu_cachable;
    sensitive << i_mpu_executable;
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        sensitive << way_o[i].rtag;
        sensitive << way_o[i].rdata;
        sensitive << way_o[i].valid;
        sensitive << way_o[i].load_fault;
        sensitive << way_o[i].executable;
        sensitive << way_o[i].readable;
        sensitive << way_o[i].writable;
    }
    sensitive << wb_lru;
    sensitive << r.requested;
    sensitive << r.req_addr;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.mem_addr;
    sensitive << r.burst_cnt;
    sensitive << r.burst_rstrb;
    sensitive << r.lru_wr;
    sensitive << r.cached;
    sensitive << r.load_fault;
    sensitive << r.write_first;
    sensitive << r.req_flush;
    sensitive << r.req_flush_addr;
    sensitive << r.req_flush_cnt;
    sensitive << r.flush_cnt;
    sensitive << r.cache_line_i;
    sensitive << r.cache_line_o;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

DCacheLru::~DCacheLru() {
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        delete wayx[i];
    }
    delete lru;
}

void DCacheLru::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, i_nrst.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_write, i_req_write.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_wdata, i_req_wdata.name());
        sc_trace(o_vcd, i_req_wstrb, i_req_wstrb.name());
        sc_trace(o_vcd, o_req_ctrl_ready, o_req_ctrl_ready.name());
        sc_trace(o_vcd, o_resp_ctrl_valid, o_resp_ctrl_valid.name());
        sc_trace(o_vcd, o_resp_ctrl_addr, o_resp_ctrl_addr.name());
        sc_trace(o_vcd, o_resp_ctrl_data, o_resp_ctrl_data.name());
        sc_trace(o_vcd, o_resp_ctrl_load_fault, o_resp_ctrl_load_fault.name());
        sc_trace(o_vcd, i_resp_ctrl_ready, i_resp_ctrl_ready.name());
        sc_trace(o_vcd, o_resp_ctrl_executable, o_resp_ctrl_executable.name());

        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, o_req_mem_len, o_req_mem_len.name());
        sc_trace(o_vcd, o_req_mem_burst, o_req_mem_burst.name());
        sc_trace(o_vcd, o_req_mem_last, o_req_mem_last.name());
        sc_trace(o_vcd, i_resp_mem_data_valid, i_resp_mem_data_valid.name());
        sc_trace(o_vcd, i_resp_mem_data, i_resp_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_load_fault, i_resp_mem_load_fault.name());

        sc_trace(o_vcd, i_flush_address, i_flush_address.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, o_istate, o_istate.name());

        std::string pn(name());
        sc_trace(o_vcd, r.requested, pn + ".r_requested");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.lru_wr, pn + ".r_lru_wr");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        //sc_trace(o_vcd, wb_ena[0], pn + ".wb_ena0");
        //sc_trace(o_vcd, wb_ena[1], pn + ".wb_ena1");
        //sc_trace(o_vcd, wb_ena[2], pn + ".wb_ena2");
        //sc_trace(o_vcd, wb_ena[3], pn + ".wb_ena3");
        sc_trace(o_vcd, way_i.addr, pn + ".way_i.addr");
        sc_trace(o_vcd, way_i.wstrb[0], pn + ".way_i.wstrb0");
        sc_trace(o_vcd, way_i.wstrb[1], pn + ".way_i.wstrb1");
        sc_trace(o_vcd, way_i.wstrb[2], pn + ".way_i.wstrb2");
        sc_trace(o_vcd, way_i.wstrb[3], pn + ".way_i.wstrb3");
        sc_trace(o_vcd, way_i.wvalid, pn + ".way_i.wvalid");
        sc_trace(o_vcd, way_i.wdata, pn + ".way_i.wdata");
        sc_trace(o_vcd, way_i.load_fault, pn + ".way_i.load_fault");
        sc_trace(o_vcd, waysel.hit, pn + ".waysel.hit");
        sc_trace(o_vcd, waysel.rdata, pn + ".waysel.rdata");
        sc_trace(o_vcd, waysel.valid, pn + ".waysel.valid");
        sc_trace(o_vcd, waysel.load_fault, pn + ".waysel.load_fault");
        sc_trace(o_vcd, waysel.executable, pn + ".waysel.executable");
        sc_trace(o_vcd, r.cached, pn + ".r_cached");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
        sc_trace(o_vcd, r.cache_line_o, pn + ".r_cache_line_o");
        sc_trace(o_vcd, r.executable, pn + ".r_executable");
        sc_trace(o_vcd, r.write_first, pn + ".r_write_first");
        sc_trace(o_vcd, r.burst_cnt, pn + ".r_burst_cnt");
    }
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        wayx[i]->generateVCD(i_vcd, o_vcd);
    }
    lru->generateVCD(i_vcd, o_vcd);
}

void DCacheLru::comb() {
    bool v_req_write;
    sc_uint<BUS_ADDR_WIDTH> vb_req_adr;
    sc_uint<BUS_DATA_WIDTH> vb_req_wdata;
    sc_uint<BUS_DATA_BYTES> vb_req_wstrb;
    sc_biguint<4*BUS_DATA_WIDTH> vb_req_wdata_line;
    sc_biguint<4*BUS_DATA_BYTES> vb_req_wstrb_line;
    sc_uint<CFG_DTAG_WIDTH> wb_rtag;
    
    sc_uint<3> wb_hit0;
    bool w_hit0_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_mpu_addr;
    bool v_line_valid;
    bool w_last;
    bool w_o_resp_load_fault;
    bool w_o_resp_executable;
    bool w_o_resp_writable;
    bool w_o_resp_readable;
    bool v_cached_load_fault;
    bool v_cached_executable;
    bool v_cached_readable;
    bool v_cached_writable;
    bool w_o_req_ctrl_ready;
    //bool vb_ena[CFG_DCACHE_WAYS];
    LruInTypeVariable v_lrui;
    TagMemInTypeVariable v_way_i;
    sc_biguint<4*BUS_DATA_WIDTH> t_cache_line_i;
    sc_uint<RISCV_ARCH> vb_cached_data;
    bool v_cached_valid;
    bool v_uncached_valid;
    sc_uint<RISCV_ARCH> vb_uncached_data;
    bool w_o_resp_valid;
    sc_uint<RISCV_ARCH> wb_o_resp_data;
    sc_uint<8> v_req_mem_len;
   
    v = r;

    if (i_req_valid.read() == 1) {
        v_req_write = i_req_write.read();
        vb_req_adr = i_req_addr.read();
        vb_req_wdata = i_req_wdata.read();
        vb_req_wstrb = i_req_wstrb.read();
    } else {
        v_req_write = r.req_write.read();
        vb_req_adr = r.req_addr.read();
        vb_req_wdata = r.req_wdata.read();
        vb_req_wstrb = r.req_wstrb.read();
    }

    int tdx = vb_req_adr(5, 4);
    vb_req_wdata_line = 0;
    vb_req_wdata_line(BUS_DATA_WIDTH*(tdx+1)-1, BUS_DATA_WIDTH*tdx) = r.req_wdata.read();
    vb_req_wstrb_line = 0;
    vb_req_wstrb_line(BUS_DATA_BYTES*(tdx+1)-1, BUS_DATA_BYTES*tdx) = r.req_wstrb.read();


    // flush request via debug interface
    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
        if (i_flush_address.read()[0] == 1) {
            v.req_flush_cnt = ~0u;
            v.req_flush_addr = FLUSH_ALL_ADDR;
        } else if (i_flush_address.read()(CFG_IOFFSET_WIDTH-1, 1) == 0xF) {
            v.req_flush_cnt = 1;
            v.req_flush_addr = i_flush_address.read();
        } else {
            v.req_flush_cnt = 0;
            v.req_flush_addr = i_flush_address.read();
        }
    }

    // Check read tag and select hit way
    wb_rtag = r.req_addr.read()(DTAG_END, DTAG_START);
    waysel.hit = MISS;
    waysel.rdata = 0;
    waysel.valid = 0;
    waysel.load_fault = 0;
    waysel.executable = 0;
    waysel.readable = 0;
    waysel.writable = 0;
    for (uint8_t n = 0; n < static_cast<uint8_t>(CFG_DCACHE_WAYS); n++) {
        if (waysel.hit == MISS && way_o[n].rtag == wb_rtag) {
            waysel.hit = n;
            waysel.rdata = way_o[n].rdata;
            waysel.valid = way_o[n].valid;
            waysel.load_fault = way_o[n].load_fault;
            waysel.executable = way_o[n].executable;
            waysel.readable = way_o[n].readable;
            waysel.writable = way_o[n].writable;
        }
    }
    waydisplace.rdata = way_o[r.lru_wr.read().to_int()].rdata;
    waydisplace.rtag = way_o[r.lru_wr.read().to_int()].rtag;
    waydisplace.valid = way_o[r.lru_wr.read().to_int()].valid;

    // swap back rdata
    v_cached_load_fault = 0;
    wb_hit0 = waysel.hit;
    w_hit0_valid = waysel.valid;
    vb_cached_data = waysel.rdata(RISCV_ARCH-1, 0);
    for (unsigned i = 1; i < 4; i++) {
        if (r.req_addr.read()(4, 3).to_uint() == i) {
            vb_cached_data = waysel.rdata((i+1)*RISCV_ARCH-1, i*RISCV_ARCH);
        }
    }
    v_cached_load_fault = waysel.load_fault;
    v_cached_executable = waysel.executable;
    v_cached_writable = waysel.writable;
    v_cached_readable = waysel.readable;

    v_lrui.init = 0;
    v_lrui.wadr = 0;
    v_lrui.we = 0;
    v_lrui.lru = 0;
    v_cached_valid = 0;
    if (r.state.read() == State_Flush) {
        v_lrui.init = 1;
        v_lrui.wadr = r.req_addr.read()(DINDEX_END, DINDEX_START);
    } else if (r.state.read() == State_WaitGrant
            || r.state.read() == State_WaitResp || r.state.read() == State_CheckResp
            || r.state.read() == State_WriteLine
            || r.state.read() == State_SetupReadAdr) {
            // Do nothing while memory writing
    } else if (w_hit0_valid && wb_hit0 != MISS && r.requested.read() == 1) {
        v_cached_valid = 1;

        // Update LRU table
        v_lrui.we = 1;
        v_lrui.lru = wb_hit0(1, 0);
        v_lrui.wadr = r.req_addr.read()(DINDEX_END, DINDEX_START);
    }

    w_o_req_ctrl_ready = !r.req_flush.read()
                       && (!r.requested.read() || v_cached_valid);
    if (i_req_valid.read() && w_o_req_ctrl_ready) {
        v.req_addr = i_req_addr.read();
        v.requested = 1;
    } else if (v_cached_valid && i_resp_ctrl_ready.read()) {
        v.requested = 0;
    }

    // System Bus access state machine
    w_last = 0;
    v_line_valid = 0;
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        //vb_ena[i] = 0;
        v_way_i.wstrb[i] = 0;
    }
    t_cache_line_i = r.cache_line_i.read();
    v_uncached_valid = 0;
    vb_uncached_data = 0;
    v_req_mem_len = 3;

    switch (r.state.read()) {
    case State_Idle:
        if (r.req_flush.read() == 1) {
            v.state = State_Flush;
            t_cache_line_i = 0;
            v.cache_line_i = ~t_cache_line_i;
            if (r.req_flush_addr.read()[0] == 1) {
                v.req_addr = FLUSH_ALL_ADDR;
                v.flush_cnt = ~0u;
            } else {
                v.req_addr = r.req_flush_addr.read();
                v.flush_cnt = r.req_flush_cnt.read();
            }
        } else if ((i_req_valid.read() == 1 && w_o_req_ctrl_ready == 1)
                 || r.requested.read() == 1) {
            /** Check hit even there's no new request only the previous one.
                This must be done in a case of CPU is halted and cache was flushed
            */
            v.state = State_CheckHit;
        }
        break;
    case State_CheckHit:
        if (v_cached_valid == 1) {
            // Hit
            v.cache_line_i = waysel.rdata | vb_req_wdata_line;
            if (i_req_valid.read() == 1 && w_o_req_ctrl_ready == 1) {
                v.state = State_CheckHit;
            } else {
                v.state = State_Idle;
            }

        } else {
            // Miss
            wb_mpu_addr = r.req_addr.read();
            v.mpu_addr = wb_mpu_addr;
            v.lru_wr = wb_lru;
            v.state = State_CheckMPU;
        }
        break;
    case State_CheckMPU:
        v.req_mem_valid = 1;
        v.state = State_WaitGrant;

        v.cache_line_o = waydisplace.rdata;

        if (i_mpu_cachable.read() == 1) {
            if (waydisplace.valid == 1) {
                v.write_first = 1;
                v.mem_addr = ((waydisplace.rtag << DTAG_START) |
                             r.req_addr.read()(DINDEX_END, DINDEX_START))
                            << CFG_DOFFSET_WIDTH;
            } else {
                v.mem_addr = r.mpu_addr.read()(BUS_ADDR_WIDTH-1, CFG_DOFFSET_WIDTH)
                            << CFG_DOFFSET_WIDTH;
            }
            v.burst_cnt = 3;
            v.cached = 1;
        } else {
            v.mem_addr = r.mpu_addr.read()(BUS_ADDR_WIDTH-1, 3) << 3;
            v.burst_cnt = 0;
            v.cached = 0;
            v_req_mem_len = 0;  // default cached = 3
        }
        v.burst_rstrb = 0x1;
        v.cache_line_i = 0;
        v.load_fault = 0;
        v.executable = i_mpu_executable.read();
        v.writable = i_mpu_writable.read();
        v.readable = i_mpu_readable.read();
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            if (r.write_first.read() == 1) {
                v.state = State_WriteLine;
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
            w_last = 1;
        }
        if (i_resp_mem_data_valid.read()) {
            for (int k = 0; k < 4; k++) {
                if (r.burst_rstrb.read()[k] == 1) {
                    t_cache_line_i((k+1)*BUS_DATA_WIDTH-1,
                                    k*BUS_DATA_WIDTH) = i_resp_mem_data.read();
                }
            }
            v.cache_line_i = t_cache_line_i;
            if (r.burst_cnt.read() == 0) {
                v.state = State_CheckResp;
            } else {
                v.burst_cnt = r.burst_cnt.read() - 1;
            }
            v.burst_rstrb = r.burst_rstrb.read() << 1;
            if (i_resp_mem_load_fault.read() == 1) {
                v.load_fault = 1;
            }
        }
        break;
    case State_CheckResp:
        if (r.cached.read() == 1) {
            v.state = State_SetupReadAdr;
            v_line_valid = 1;
            v_way_i.wstrb[r.lru_wr.read().to_int()] = ~0ul;  // write full line
            //vb_ena[r.lru_wr.read().to_int()] = 1;
        } else {
            v_uncached_valid = 1;
            vb_uncached_data = r.cache_line_i.read()(RISCV_ARCH-1, 0);
            if (i_resp_ctrl_ready.read() == 1) {
                v.state = State_Idle;
                v.requested = 0;
            }
        }
        break;
    case State_WriteLine:
        if (r.burst_cnt.read() == 0) {
            w_last = 1;
        }
        if (i_resp_mem_data_valid.read()) {
            v.cache_line_o = (0, r.cache_line_o.read()(4*BUS_DATA_WIDTH-1, BUS_DATA_WIDTH));
            if (r.burst_cnt.read() == 0) {
                v.mem_addr = r.mpu_addr.read()(BUS_ADDR_WIDTH-1, CFG_DOFFSET_WIDTH)
                            << CFG_DOFFSET_WIDTH;
                v.req_mem_valid = 1;
                v.burst_cnt = 3;
                v.write_first = 0;
                v.state = State_WaitGrant;
            } else {
                v.burst_cnt = r.burst_cnt.read() - 1;
            }
            //if (i_resp_mem_store_fault.read() == 1) {
            //    v.store_fault = 1;
            //}
        }
        break;
    case State_SetupReadAdr:
        v.state = State_CheckHit;
        break;
    case State_Flush:
        if (r.flush_cnt.read() == 0) {
            v.req_flush = 0;
            v.state = State_Idle;
        } else {
            v.flush_cnt = r.flush_cnt.read() - 1;
            v.req_addr = r.req_addr.read() + (1 << CFG_DOFFSET_WIDTH);
        }
        for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
            v_way_i.wstrb[i] = ~0u;
            //vb_ena[i] = 1;
        }
        break;
    default:;
    }

    // Write signals:
    //v_way_i.wadr = r.mem_addr.read();
    //v_way_i.wstrb = ~0u;
    v_way_i.wvalid = v_line_valid;
    v_way_i.wdata = r.cache_line_i.read();
    v_way_i.load_fault = r.load_fault.read();
    v_way_i.executable = r.executable.read();
    v_way_i.writable = r.writable.read();
    v_way_i.readable = r.readable.read();

    if (r.state.read() == State_Idle || v_cached_valid == 1) {
        v_way_i.addr = vb_req_adr;
        v_lrui.radr = vb_req_adr(DINDEX_END, DINDEX_START);
    } else {
        v_way_i.addr = r.req_addr.read();
        v_lrui.radr = r.req_addr.read()(DINDEX_END, DINDEX_START);
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    lrui.init = v_lrui.init;
    lrui.radr = v_lrui.radr;
    lrui.wadr = v_lrui.wadr;
    lrui.we = v_lrui.we;
    lrui.lru = v_lrui.lru;

    way_i.addr = v_way_i.addr;
    //way_i.wadr = v_way_i.wadr;
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        way_i.wstrb[i] = v_way_i.wstrb[i];
    }
    way_i.wvalid = v_way_i.wvalid;
    way_i.wdata = v_way_i.wdata;
    way_i.load_fault = v_way_i.load_fault;
    way_i.executable = v_way_i.executable;
    way_i.writable = v_way_i.writable;
    way_i.readable = v_way_i.readable;

    //for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
    //    wb_ena[i] = vb_ena[i];
    //}

    o_req_ctrl_ready = w_o_req_ctrl_ready;

    o_req_mem_valid = r.req_mem_valid.read();
    o_req_mem_addr = r.mem_addr.read();
    o_req_mem_write = r.write_first.read();
    o_req_mem_strob = 0;
    o_req_mem_data = r.cache_line_o.read()(BUS_DATA_WIDTH-1, 0).to_uint64();
    o_req_mem_len = v_req_mem_len;
    o_req_mem_burst = 1;    // 00=FIX; 01=INCR; 10=WRAP
    o_req_mem_last = w_last;

    w_o_resp_valid = v_cached_valid || v_uncached_valid;
    if (v_uncached_valid == 1) {
        wb_o_resp_data = vb_uncached_data;
        w_o_resp_load_fault = r.load_fault;
        w_o_resp_executable = r.executable;
        w_o_resp_writable = r.writable;
        w_o_resp_readable = r.readable;
    } else {
        wb_o_resp_data = vb_cached_data;
        w_o_resp_load_fault = v_cached_load_fault;
        w_o_resp_executable = v_cached_executable;
        w_o_resp_writable = v_cached_writable;
        w_o_resp_readable = v_cached_readable;
    }

    o_resp_ctrl_valid = w_o_resp_valid;
    o_resp_ctrl_data = wb_o_resp_data;
    o_resp_ctrl_addr = r.req_addr.read();
    o_resp_ctrl_load_fault = w_o_resp_load_fault;
    o_resp_ctrl_executable = w_o_resp_executable;
    o_resp_ctrl_writable = w_o_resp_writable;
    o_resp_ctrl_readable = w_o_resp_readable;
    o_mpu_addr = r.mpu_addr.read();
    o_istate = r.state.read()(1, 0);
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
    sensitive << w_req_ctrl_ready;
    sensitive << w_resp_ctrl_valid;
    sensitive << wb_resp_ctrl_addr;
    sensitive << wb_resp_ctrl_data;
    sensitive << w_resp_ctrl_load_fault;
    sensitive << r.clk_cnt;

    SC_METHOD(comb_bus);
    sensitive << w_nrst;
    sensitive << w_req_mem_valid;
    sensitive << w_req_mem_write;
    sensitive << wb_req_mem_addr;
    sensitive << wb_req_mem_strob;
    sensitive << wb_req_mem_data;
    sensitive << rbus.state;
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
    tt->o_req_ctrl_ready(w_req_ctrl_ready);
    tt->o_resp_ctrl_valid(w_resp_ctrl_valid);
    tt->o_resp_ctrl_addr(wb_resp_ctrl_addr);
    tt->o_resp_ctrl_data(wb_resp_ctrl_data);
    tt->o_resp_ctrl_load_fault(w_resp_ctrl_load_fault);
    tt->o_resp_ctrl_executable(w_resp_ctrl_executable);
    tt->o_resp_ctrl_writable(w_resp_ctrl_writable);
    tt->o_resp_ctrl_readable(w_resp_ctrl_readable);
    tt->i_resp_ctrl_ready(w_resp_ctrl_ready);
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
    tt->i_resp_mem_data_valid(w_resp_mem_data_valid);
    tt->i_resp_mem_data(wb_resp_mem_data);
    tt->i_resp_mem_load_fault(w_resp_mem_load_fault);
    // MPU interface
    tt->o_mpu_addr(wb_mpu_addr);
    tt->i_mpu_cachable(w_mpu_cachable);
    tt->i_mpu_executable(w_mpu_executable);
    tt->i_mpu_writable(w_mpu_writable);
    tt->i_mpu_readable(w_mpu_readable);
    // Debug interface
    tt->i_flush_address(wb_flush_address);
    tt->i_flush_valid(w_flush_valid);
    tt->o_istate(wb_istate);

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
    sc_trace(tb_vcd, w_req_ctrl_ready, "w_req_ctrl_ready");
    sc_trace(tb_vcd, w_resp_ctrl_valid, "w_resp_ctrl_valid");
    sc_trace(tb_vcd, wb_resp_ctrl_addr, "wb_resp_ctrl_addr");
    sc_trace(tb_vcd, wb_resp_ctrl_data, "wb_resp_ctrl_data");
    sc_trace(tb_vcd, w_resp_ctrl_ready, "w_resp_ctrl_ready");
    sc_trace(tb_vcd, w_req_mem_ready, "w_req_mem_ready");
    sc_trace(tb_vcd, w_req_mem_valid, "w_req_mem_valid");
    sc_trace(tb_vcd, w_req_mem_write, "w_req_mem_write");
    sc_trace(tb_vcd, wb_req_mem_addr, "wb_req_mem_addr");
    sc_trace(tb_vcd, wb_req_mem_strob, "wb_req_mem_strob");
    sc_trace(tb_vcd, wb_req_mem_data, "wb_req_mem_data");
    sc_trace(tb_vcd, w_resp_mem_data_valid, "w_resp_mem_data_valid");
    sc_trace(tb_vcd, wb_resp_mem_data, "wb_resp_mem_data");
    sc_trace(tb_vcd, wb_istate, "wb_istate");
    sc_trace(tb_vcd, rbus.burst_addr, "rbus_burst_addr");
    sc_trace(tb_vcd, rbus.burst_cnt, "rbus_burst_cnt");

    tt->generateVCD(tb_vcd, tb_vcd);
}


void DCacheLru_tb::comb0() {
    v = r;
    v.clk_cnt = r.clk_cnt.read() + 1;

    w_flush_valid = 0;
    wb_flush_address = 0;

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
    w_resp_ctrl_ready = 1;

    w_mpu_cachable = 1;
    w_mpu_executable = 1;
    w_mpu_writable = 1;
    w_mpu_readable = 1;

    const unsigned START_POINT = 10 + 1 + (1 << (CFG_IINDEX_WIDTH+1));
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
        w_req_valid = 1;
        w_req_write = 1;
        wb_req_addr = 0x00000008;
        wb_req_wdata = 0x000000000000CC00ull;
        wb_req_wstrb = 0x02;
        break;

    default:;
    }
}

void DCacheLru_tb::comb_bus() {
    vbus = rbus;

    w_req_mem_ready = 0;
    w_resp_mem_data_valid = 0;
    wb_resp_mem_data = 0;

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
        w_resp_mem_data_valid = 1;
        wb_resp_mem_data = 0x2000000010000000ull + rbus.burst_addr.read();
        vbus.burst_cnt = rbus.burst_cnt.read() - 1;
        vbus.burst_addr = rbus.burst_addr.read() + 8;
        if (rbus.burst_cnt.read() == 1) {
            vbus.state = BUS_ReadLast;
        }
        break;
    case BUS_ReadLast:
        w_req_mem_ready = 1;
        w_resp_mem_data_valid = 1;
        wb_resp_mem_data = 0x2000000010000000ull + rbus.burst_addr.read();
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

    if (w_nrst.read() == 0) {
        vbus.state = BUS_Idle;
        vbus.burst_addr = 0;
        vbus.burst_cnt = 0;
    }
}

#endif

}  // namespace debugger

