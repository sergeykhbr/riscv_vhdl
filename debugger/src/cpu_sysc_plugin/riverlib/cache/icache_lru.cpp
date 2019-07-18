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

#include "icache_lru.h"

namespace debugger {

ICacheLru::ICacheLru(sc_module_name name_, bool async_reset, int isize) :
    sc_module(name_) {
    async_reset_ = async_reset;
    isize_ = isize;

    char tstr1[32] = "wayeven0";
    char tstr2[32] = "wayodd0";
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        tstr1[7] = '0' + static_cast<char>(i);
        wayevenx[i] = new IWayMem(tstr1, async_reset, 2*i);
        wayevenx[i]->i_clk(i_clk);
        wayevenx[i]->i_nrst(i_nrst);
        wayevenx[i]->i_radr(swapin[WAY_EVEN].radr);
        wayevenx[i]->i_wadr(swapin[WAY_EVEN].wadr);
        wayevenx[i]->i_wena(wb_ena_even[i]);
        wayevenx[i]->i_wstrb(swapin[WAY_EVEN].wstrb);
        wayevenx[i]->i_wvalid(swapin[WAY_EVEN].wvalid);
        wayevenx[i]->i_wdata(swapin[WAY_EVEN].wdata);
        wayevenx[i]->i_load_fault(swapin[WAY_EVEN].load_fault);
        wayevenx[i]->o_rtag(memeven[i].rtag);
        wayevenx[i]->o_rdata(memeven[i].rdata);
        wayevenx[i]->o_valid(memeven[i].valid);
        wayevenx[i]->o_load_fault(memeven[i].load_fault);

        tstr2[6] = '0' + static_cast<char>(i);
        wayoddx[i] = new IWayMem(tstr2, async_reset, 2*i + 1);
        wayoddx[i]->i_clk(i_clk);
        wayoddx[i]->i_nrst(i_nrst);
        wayoddx[i]->i_radr(swapin[WAY_ODD].radr);
        wayoddx[i]->i_wadr(swapin[WAY_ODD].wadr);
        wayoddx[i]->i_wena(wb_ena_odd[i]);
        wayoddx[i]->i_wstrb(swapin[WAY_ODD].wstrb);
        wayoddx[i]->i_wvalid(swapin[WAY_ODD].wvalid);
        wayoddx[i]->i_wdata(swapin[WAY_ODD].wdata);
        wayoddx[i]->i_load_fault(swapin[WAY_ODD].load_fault);
        wayoddx[i]->o_rtag(memodd[i].rtag);
        wayoddx[i]->o_rdata(memodd[i].rdata);
        wayoddx[i]->o_valid(memodd[i].valid);
        wayoddx[i]->o_load_fault(memodd[i].load_fault);
    }

    lrueven = new ILru("lrueven0", async_reset);
    lrueven->i_nrst(i_nrst);
    lrueven->i_clk(i_clk);
    lrueven->i_adr(lrui[WAY_EVEN].adr);
    lrueven->i_we(lrui[WAY_EVEN].we);
    lrueven->i_lru(lrui[WAY_EVEN].lru);
    lrueven->o_lru(wb_lru_even);

    lruodd = new ILru("lruodd0", async_reset);
    lrueven->i_nrst(i_nrst);
    lrueven->i_clk(i_clk);
    lrueven->i_adr(lrui[WAY_ODD].adr);
    lrueven->i_we(lrui[WAY_ODD].we);
    lrueven->i_lru(lrui[WAY_ODD].lru);
    lrueven->o_lru(wb_lru_odd);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_valid;
    sensitive << i_req_ctrl_addr;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << i_resp_ctrl_ready;
    sensitive << i_flush_address;
    sensitive << i_flush_valid;
    sensitive << i_req_mem_ready;
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        sensitive << memeven[i].rtag;
        sensitive << memeven[i].rdata;
        sensitive << memeven[i].valid;
        sensitive << memeven[i].load_fault;
        sensitive << memodd[i].rtag;
        sensitive << memodd[i].rdata;
        sensitive << memodd[i].valid;
        sensitive << memodd[i].load_fault;
    }
    sensitive << wb_lru_even;
    sensitive << wb_lru_odd;
    sensitive << r.req_addr;
    sensitive << r.req_addr_overlay;
    sensitive << r.use_overlay;
    sensitive << r.x_removed;
    sensitive << r.state;
    sensitive << r.mem_addr;
    sensitive << r.burst_cnt;
    sensitive << r.burst_valid;
    sensitive << r.lru_even_wr;
    sensitive << r.lru_odd_wr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

ICacheLru::~ICacheLru() {
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        delete wayevenx[i];
        delete wayoddx[i];
    }
    delete lrueven;
    delete lruodd;
}

void ICacheLru::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, "/top/cache0/i0/i_nrst");
        sc_trace(o_vcd, i_req_ctrl_valid, "/top/cache0/i0/i_req_ctrl_valid");
        sc_trace(o_vcd, i_req_ctrl_addr, "/top/cache0/i0/i_req_ctrl_addr");
        sc_trace(o_vcd, o_req_ctrl_ready, "/top/cache0/i0/o_req_ctrl_ready");
        sc_trace(o_vcd, o_req_mem_valid, "/top/cache0/i0/o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_addr, "/top/cache0/i0/o_req_mem_addr");
        sc_trace(o_vcd, i_req_mem_ready, "/top/cache0/i0/i_req_mem_ready");
        sc_trace(o_vcd, i_resp_mem_data_valid, "/top/cache0/i0/i_resp_mem_data_valid");
        sc_trace(o_vcd, i_resp_mem_data, "/top/cache0/i0/i_resp_mem_data");
        sc_trace(o_vcd, i_flush_address, "/top/cache0/i0/i_flush_address");
        sc_trace(o_vcd, i_flush_valid, "/top/cache0/i0/i_flush_valid");
        sc_trace(o_vcd, o_resp_ctrl_valid, "/top/cache0/i0/o_resp_ctrl_valid");
        sc_trace(o_vcd, i_resp_ctrl_ready, "/top/cache0/i0/i_resp_ctrl_ready");
        sc_trace(o_vcd, o_resp_ctrl_addr, "/top/cache0/i0/o_resp_ctrl_addr");
        sc_trace(o_vcd, o_resp_ctrl_data, "/top/cache0/i0/o_resp_ctrl_data");
    }
}

void ICacheLru::comb() {
    bool w_raddr5;
    bool w_use_overlay;
    sc_uint<BUS_ADDR_WIDTH> wb_radr_overlay;
    sc_uint<CFG_ITAG_WIDTH> wb_rtag;
    sc_uint<3> wb_hit0;
    sc_uint<3> wb_hit1;
    bool w_hit0_valid;
    bool w_hit1_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<32> wb_o_resp_data;
    bool w_ena;
    bool w_o_resp_valid;
    bool w_o_resp_load_fault;
    bool w_o_req_ctrl_ready;
    bool w_o_req_mem_valid;
   
    v = r;

    w_raddr5 = i_req_ctrl_addr.read()[CFG_IOFFSET_WIDTH];

    w_use_overlay = 0;
    if (i_req_ctrl_addr.read()(CFG_IOFFSET_WIDTH-1, 1) == 0xF) {
        w_use_overlay = 1;
    }

    wb_radr_overlay = i_req_ctrl_addr.read() + (1 << CFG_IOFFSET_WIDTH);
    wb_radr_overlay >>= CFG_IOFFSET_WIDTH;
    wb_radr_overlay <<= CFG_IOFFSET_WIDTH;

    if (w_raddr5 == 0) {
        swapin[WAY_EVEN].radr = i_req_ctrl_addr.read();
        swapin[WAY_ODD].radr = wb_radr_overlay;
    } else {
        swapin[WAY_EVEN].radr = wb_radr_overlay;
        swapin[WAY_ODD].radr = i_req_ctrl_addr.read();
    }

    // Check read tag and select hit way
    wb_rtag = r.req_addr.read()(ITAG_END, ITAG_START);
    waysel[WAY_EVEN].hit = MISS;
    waysel[WAY_EVEN].rdata = 0;
    waysel[WAY_EVEN].valid = 0;
    waysel[WAY_EVEN].load_fault = 0;
    waysel[WAY_ODD].hit = MISS;
    waysel[WAY_ODD].rdata = 0;
    waysel[WAY_ODD].valid = 0;
    waysel[WAY_ODD].load_fault = 0;
    for (uint8_t n = 0; n < static_cast<uint8_t>(CFG_ICACHE_WAYS); n++) {
        if (memeven[n].rtag == wb_rtag) {
            waysel[WAY_EVEN].hit = n;
            waysel[WAY_EVEN].rdata = memeven[n].rdata;
            waysel[WAY_EVEN].valid = memeven[n].valid;
            waysel[WAY_EVEN].load_fault = memeven[n].load_fault;
        }

        if (memodd[n].rtag == wb_rtag) {
            waysel[WAY_ODD].hit = n;
            waysel[WAY_ODD].rdata = memodd[n].rdata;
            waysel[WAY_ODD].valid = memodd[n].valid;
            waysel[WAY_ODD].load_fault = memodd[n].load_fault;
        }
    }

    // swap back rdata
    w_o_resp_load_fault = 0;
    if (r.req_addr.read()[CFG_IOFFSET_WIDTH] == 0) {
        if (r.use_overlay.read() == 0) {
            wb_hit0 = waysel[WAY_EVEN].hit;
            wb_hit1 = waysel[WAY_EVEN].hit;
            w_hit0_valid = waysel[WAY_EVEN].valid;
            w_hit1_valid = waysel[WAY_EVEN].valid;
            wb_o_resp_data = waysel[WAY_EVEN].rdata;
            w_o_resp_load_fault = waysel[WAY_EVEN].load_fault;
        } else {
            wb_hit0 = waysel[WAY_EVEN].hit;
            wb_hit1 = waysel[WAY_ODD].hit;
            w_hit0_valid = waysel[WAY_EVEN].valid;
            w_hit1_valid = waysel[WAY_ODD].valid;
            wb_o_resp_data(15, 0) = waysel[WAY_EVEN].rdata(15, 0);
            wb_o_resp_data(31, 16) = waysel[WAY_ODD].rdata(15, 0);
            w_o_resp_load_fault =
                waysel[WAY_EVEN].load_fault | waysel[WAY_ODD].load_fault;
        }
    } else {
        if (r.use_overlay.read() == 0) {
            wb_hit0 = waysel[WAY_ODD].hit;
            wb_hit1 = waysel[WAY_ODD].hit;
            w_hit0_valid = waysel[WAY_ODD].valid;
            w_hit1_valid = waysel[WAY_ODD].valid;
            wb_o_resp_data = waysel[WAY_ODD].rdata;
            w_o_resp_load_fault = waysel[WAY_ODD].load_fault;
        } else {
            wb_hit0 = waysel[WAY_ODD].hit;
            wb_hit1 = waysel[WAY_EVEN].hit;
            w_hit0_valid = waysel[WAY_ODD].valid;
            w_hit1_valid = waysel[WAY_EVEN].valid;
            wb_o_resp_data(15, 0) = waysel[WAY_ODD].rdata(15, 0);
            wb_o_resp_data(31, 16) = waysel[WAY_EVEN].rdata(15, 0);
            w_o_resp_load_fault =
                waysel[WAY_ODD].load_fault | waysel[WAY_EVEN].load_fault;
        }
    }

    lrui[WAY_EVEN].adr = 0;
    lrui[WAY_EVEN].we = 0;
    lrui[WAY_EVEN].lru = 0;
    lrui[WAY_ODD].adr = 0;
    lrui[WAY_ODD].we = 0;
    lrui[WAY_ODD].lru = 0;
    w_o_resp_valid = 0;
    if (r.x_removed.read() == 1 && (w_hit0_valid || w_hit1_valid) == 1
        && wb_hit0 != MISS && wb_hit1 != MISS) {
        w_o_resp_valid = 1;

        // Update LRU table
        if (r.req_addr.read()[CFG_IOFFSET_WIDTH] == 0) {
            lrui[WAY_EVEN].we = 1;
            lrui[WAY_EVEN].lru = wb_hit0(1, 0);
            lrui[WAY_EVEN].adr =
                r.req_addr.read()(IINDEX_END, IINDEX_START);
            if (r.use_overlay.read() == 1) {
                lrui[WAY_ODD].we = 1;
                lrui[WAY_ODD].lru = wb_hit1(1, 0);
                lrui[WAY_ODD].adr =
                    r.req_addr_overlay.read()(IINDEX_END, IINDEX_START);
            }
        } else {
            lrui[WAY_ODD].we = 1;
            lrui[WAY_ODD].lru = wb_hit0(1, 0);
            lrui[WAY_ODD].adr =
                r.req_addr.read()(IINDEX_END, IINDEX_START);
            if (r.use_overlay.read() == 1) {
                lrui[WAY_EVEN].we = 1;
                lrui[WAY_EVEN].lru = wb_hit1(1, 0);
                lrui[WAY_EVEN].adr =
                    r.req_addr_overlay.read()(IINDEX_END, IINDEX_START);
            }
        }
    }

    w_o_req_ctrl_ready = 1;
    if (i_req_ctrl_valid.read() && w_o_req_ctrl_ready) {
        v.req_addr = i_req_ctrl_addr.read();
        v.req_addr_overlay = wb_radr_overlay;
        v.use_overlay = w_use_overlay;
    }

    // System Bus access state machine
    w_o_req_mem_valid = 0;
    w_ena = 0;
    switch (r.state.read()) {
    case State_Idle:
        if (i_req_ctrl_valid.read() == 1) {
            v.state = State_CheckHit;
        }
        break;
    case State_CheckHit:
        if (w_o_resp_valid == 1) {
            // Hit
            if (i_req_ctrl_valid.read() == 1) {
                v.state = State_CheckHit;
            } else {
                v.state = State_Idle;
            }

        } else {
            // Miss
            w_o_req_mem_valid = 1;
            if (r.x_removed.read() == 0 || !w_hit0_valid || wb_hit0 == MISS) {
                wb_mem_addr = r.req_addr;
            } else {
                wb_mem_addr = r.req_addr_overlay.read();
            }
            if (i_req_mem_ready.read() == 1) {
                v.state = State_WaitResp;
            } else {
                v.state = State_WaitGrant;
            }

            v.mem_addr = wb_mem_addr(BUS_ADDR_WIDTH-1, 3) << 3;
            v.burst_cnt = 3;
            switch (wb_mem_addr(CFG_IOFFSET_WIDTH-1, 3)) {
            case 0:
                v.burst_wstrb = 0x1;
                break;
            case 1:
                v.burst_wstrb = 0x2;
                break;
            case 2:
                v.burst_wstrb = 0x4;
                break;
            case 3:
                v.burst_wstrb = 0x8;
                break;
            }
            v.burst_valid = v.burst_wstrb.read();
            v.lru_even_wr = wb_lru_even;
            v.lru_odd_wr = wb_lru_odd;
        }
        break;
    case State_WaitGrant:
        w_o_req_mem_valid = 1;
        if (i_req_mem_ready.read()) {
            v.state = State_WaitResp;
        }
        break;
    case State_WaitResp:
        if (i_resp_mem_data_valid.read()) {
            v.x_removed = 1;        // first write after reset
            w_ena = 1;
            if (r.burst_cnt.read() == 0) {
                v.state = State_CheckHit;
            } else {
                v.burst_cnt = r.burst_cnt.read() - 1;
            }
            /** Suppose using WRAP burst transaction */
            v.burst_wstrb =
                (r.burst_wstrb.read() << 1) | r.burst_wstrb.read()[3];
            v.burst_valid = r.burst_valid.read() | v.burst_wstrb.read();
        }
        break;
    default:;
    }

    // Write signals:
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        wb_ena_even[i] = 0;
        wb_ena_odd[i] = 0;
    }

    if (r.mem_addr.read()[CFG_IOFFSET_WIDTH] == 0) {
        wb_ena_even[r.lru_even_wr.read()] = w_ena;
        swapin[WAY_EVEN].wadr = r.mem_addr.read();
        swapin[WAY_EVEN].wstrb = r.burst_wstrb.read();
        swapin[WAY_EVEN].wvalid = r.burst_valid.read();
        swapin[WAY_EVEN].load_fault = i_resp_mem_load_fault.read();
        swapin[WAY_ODD].wadr = 0;
        swapin[WAY_ODD].wstrb = 0;
        swapin[WAY_ODD].wvalid = 0;
        swapin[WAY_ODD].load_fault = 0;
    } else {
        swapin[WAY_EVEN].wadr = 0;
        swapin[WAY_EVEN].wstrb = 0;
        swapin[WAY_EVEN].wvalid = 0;
        swapin[WAY_EVEN].load_fault = 0;
        wb_ena_odd[r.lru_odd_wr.read()] = w_ena;
        swapin[WAY_ODD].wadr = r.mem_addr.read();
        swapin[WAY_ODD].wstrb = r.burst_wstrb.read();
        swapin[WAY_ODD].wvalid = r.burst_valid.read();
        swapin[WAY_ODD].load_fault = i_resp_mem_load_fault.read();
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_req_ctrl_ready = w_o_req_ctrl_ready;

    o_req_mem_valid = w_o_req_mem_valid;
    o_req_mem_addr = r.mem_addr.read();
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;

    o_resp_ctrl_valid = w_o_resp_valid;
    o_resp_ctrl_data = wb_o_resp_data;
    o_resp_ctrl_addr = r.req_addr.read();
    o_resp_ctrl_load_fault = w_o_resp_load_fault;
    o_istate = r.state;
}

void ICacheLru::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

#ifdef DBG_ICACHE_TB
void ICacheLru_tb::comb0() {
    v = r;
    v.clk_cnt = r.clk_cnt.read() + 1;

    if (r.clk_cnt.read() < 10) {
        w_nrst = 0;
        v.mem_state = 0;
        v.mem_cnt = 0;
        v.fetch_state = 0;
        v.fetch_cnt = 0;
        return;
    }
    w_nrst = 1;

    w_req_ctrl_valid = 0;
    wb_req_ctrl_addr = 0;
    w_resp_ctrl_ready = 0;

    struct FetchDelayType {
        uint32_t raddr;
        int req_wait;
        int accept_wait;
    };
    static const FetchDelayType RADDR[4] = {{0x100008f4, 0, 0}, {0x100007b0, 0, 0}, {0x100008f0, 2, 0}, {0x100007b4, 0, 0}};
    struct MemDelayType {
        unsigned rdy_wait;
        unsigned valid_wait;
    };
    //static const MemDelayType MEM_DELAY[4] = {{2,3}, {2,3}, {0,0}, {0,0}};
    static const MemDelayType MEM_DELAY[4] = {{0,0}, {0,0}, {5,0}, {5,0}};


    // fetch model:
    w_resp_ctrl_ready = 0;
    if (r.clk_cnt.read() >= 15) {
        switch (r.fetch_state.read()) {
        case 0:
            if (r.fetch_cnt.read() < 4) {
                if (RADDR[r.fetch_cnt.read()].req_wait == 0) {
                    w_req_ctrl_valid = 1;
                    wb_req_ctrl_addr = RADDR[r.fetch_cnt.read()].raddr;
                    if (w_req_ctrl_ready) {
                        v.fetch_state = 3;
                        v.fetch_cnt = r.fetch_cnt.read() + 1;
                        v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].accept_wait;
                    } else {
                        v.fetch_state = 2;
                    }
                } else {
                    if (RADDR[r.fetch_cnt.read()].req_wait == 1) {
                        v.fetch_state = 2;
                    } else {
                        v.fetch_state = 1;
                    }
                    v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].req_wait;
                }
            }
            break;
        case 1:
            // wait to request:
            v.fetch_wait_cnt = r.fetch_wait_cnt.read() - 1;
            if (r.fetch_wait_cnt.read() == 1) {
                v.fetch_state = 2;
            }
            break;
        case 2:// wait ready signal
            w_req_ctrl_valid = 1;
            wb_req_ctrl_addr = RADDR[r.fetch_cnt.read()].raddr;
            if (w_req_ctrl_ready) {
                v.fetch_state = 3;
                v.fetch_cnt = r.fetch_cnt.read() + 1;
                v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].accept_wait;
            }
            break;
        case 3: // wait valid signal:
            if (w_resp_ctrl_valid) {
                w_resp_ctrl_ready = 1;
                if (r.fetch_wait_cnt.read()) {
                    v.fetch_wait_cnt = r.fetch_wait_cnt.read() - 1;
                    w_resp_ctrl_ready = 0;
                } else if (r.fetch_cnt.read() < 4) {
                    if (RADDR[r.fetch_cnt.read()].req_wait == 0) {
                        w_req_ctrl_valid = 1;
                        wb_req_ctrl_addr = RADDR[r.fetch_cnt.read()].raddr;
                        if (w_req_ctrl_ready) {
                            v.fetch_state = 3;
                            v.fetch_cnt = r.fetch_cnt.read() + 1;
                            v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].accept_wait;
                        } else {
                            v.fetch_state = 2;
                        }
                    } else {
                        if (RADDR[r.fetch_cnt.read()].req_wait == 1) {
                            v.fetch_state = 2;
                        } else {
                            v.fetch_state = 1;
                        }
                        v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].req_wait;
                    }
                } else {
                    v.fetch_state = 0;
                }
            }
            break;
        default:;
        }
    }

    if (r.clk_cnt.read() == 21) {
        wb_req_ctrl_addr = 0x100008f8;
    } else if (r.clk_cnt.read() == 22) {
        wb_req_ctrl_addr = 0x100007b4;
    }


    // Memory model:
    w_req_mem_ready = 0;
    w_resp_mem_data_valid = 0;

    switch (r.mem_state.read()) {
    case 0: // MemIdle
        if (w_req_mem_valid && r.mem_cnt.read() < 4) {
            if (MEM_DELAY[r.mem_cnt.read()].rdy_wait == 0) {
                if (MEM_DELAY[r.mem_cnt.read()].valid_wait == 0) {
                    v.mem_state = 3;
                    v.mem_raddr = wb_req_mem_addr;
                    w_req_mem_ready = 1;
                    v.mem_cnt = r.mem_cnt.read() + 1;
                } else {
                    v.mem_state = 2;
                    v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].valid_wait;
                }
            } else {
                v.mem_state = 1;
                v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].rdy_wait;
            }
        }
        break;
    case 1: 
        v.mem_wait_cnt = r.mem_wait_cnt.read() - 1;
        if (r.mem_wait_cnt.read() == 1) {
            if (w_req_mem_valid) {
                v.mem_raddr = wb_req_mem_addr;
                w_req_mem_ready = 1;
                v.mem_cnt = r.mem_cnt.read() + 1;
                if (MEM_DELAY[r.mem_cnt.read()].valid_wait == 0) {
                    v.mem_state = 3;
                } else {
                    v.mem_state = 2;
                    v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].valid_wait;
                }
            } else {
                v.mem_state = 0;
            }
        }
        break;
    case 2:
        v.mem_wait_cnt = r.mem_wait_cnt.read() - 1;
        if (r.mem_wait_cnt.read() == 1) {
            v.mem_state = 3;
        }
        break;
    case 3:
        w_resp_mem_data_valid = 1;
        if (r.mem_raddr.read() == 0x100008f0) {
            wb_resp_mem_data = 0xffdff06fea9ff0efull;
        } else if (r.mem_raddr.read() == 0x100007b0) {
            wb_resp_mem_data = 0xfa0a0a1300004a17;
        }

        if (w_req_mem_valid && r.mem_cnt.read() < 4) {
            if (MEM_DELAY[r.mem_cnt.read()].rdy_wait == 0) {
                if (MEM_DELAY[r.mem_cnt.read()].valid_wait == 0) {
                    v.mem_state = 3;
                    v.mem_raddr = wb_req_mem_addr;
                    w_req_mem_ready = 1;
                    v.mem_cnt = r.mem_cnt.read() + 1;
                } else {
                    v.mem_state = 2;
                    v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].valid_wait;
                }
            } else {
                v.mem_state = 1;
                v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].rdy_wait;
            }
        } else {
            v.mem_state = 0;
        }
        break;
    default:;
    }


}
#endif

}  // namespace debugger

