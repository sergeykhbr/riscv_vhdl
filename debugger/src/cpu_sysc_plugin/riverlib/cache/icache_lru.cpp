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

ICacheLru::ICacheLru(sc_module_name name_, bool async_reset,
    int index_width) : sc_module(name_) {
    async_reset_ = async_reset;
    index_width_ = index_width;

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
    lruodd->i_nrst(i_nrst);
    lruodd->i_clk(i_clk);
    lruodd->i_adr(lrui[WAY_ODD].adr);
    lruodd->i_we(lrui[WAY_ODD].we);
    lruodd->i_lru(lrui[WAY_ODD].lru);
    lruodd->o_lru(wb_lru_odd);

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
    sensitive << r.requested;
    sensitive << r.req_addr;
    sensitive << r.req_addr_overlay;
    sensitive << r.use_overlay;
    sensitive << r.state;
    sensitive << r.mem_addr;
    sensitive << r.burst_cnt;
    sensitive << r.burst_wstrb;
    sensitive << r.burst_valid;
    sensitive << r.lru_even_wr;
    sensitive << r.lru_odd_wr;
    sensitive << r.req_flush;
    sensitive << r.req_flush_addr;
    sensitive << r.req_flush_cnt;
    sensitive << r.flush_cnt;

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
        sc_trace(o_vcd, r.requested, "/top/cache0/i0/r_requested");
        sc_trace(o_vcd, r.state, "/top/cache0/i0/r_state");
        sc_trace(o_vcd, r.lru_even_wr, "/top/cache0/i0/r_lru_even_wr");
        sc_trace(o_vcd, r.lru_odd_wr, "/top/cache0/i0/r_lru_odd_wr");
        sc_trace(o_vcd, r.req_addr, "/top/cache0/i0/r_req_addr");
        sc_trace(o_vcd, r.req_addr_overlay, "/top/cache0/i0/r_req_addr_overlay");

        sc_trace(o_vcd, wb_ena_even[0], "/top/cache0/i0/wb_ena_even0");
        sc_trace(o_vcd, wb_ena_even[1], "/top/cache0/i0/wb_ena_even1");
        sc_trace(o_vcd, wb_ena_even[2], "/top/cache0/i0/wb_ena_even2");
        sc_trace(o_vcd, wb_ena_even[3], "/top/cache0/i0/wb_ena_even3");
        sc_trace(o_vcd, swapin[0].radr, "/top/cache0/i0/swapin(0).radr");
        sc_trace(o_vcd, swapin[0].wadr, "/top/cache0/i0/swapin(0).wadr");
        sc_trace(o_vcd, swapin[0].wstrb, "/top/cache0/i0/swapin(0).wstrb");
        sc_trace(o_vcd, swapin[0].wvalid, "/top/cache0/i0/swapin(0).wvalid");
        sc_trace(o_vcd, swapin[0].wdata, "/top/cache0/i0/swapin(0).wdata");
        sc_trace(o_vcd, swapin[0].load_fault, "/top/cache0/i0/swapin(0).load_fault");
        sc_trace(o_vcd, swapin[1].radr, "/top/cache0/i0/swapin(1).radr");
        sc_trace(o_vcd, swapin[1].wadr, "/top/cache0/i0/swapin(1).wadr");
        sc_trace(o_vcd, swapin[1].wstrb, "/top/cache0/i0/swapin(1).wstrb");
        sc_trace(o_vcd, swapin[1].wvalid, "/top/cache0/i0/swapin(1).wvalid");
        sc_trace(o_vcd, swapin[1].wdata, "/top/cache0/i0/swapin(1).wdata");
        sc_trace(o_vcd, swapin[1].load_fault, "/top/cache0/i0/swapin(1).load_fault");
        sc_trace(o_vcd, waysel[0].hit, "/top/cache0/i0/waysel(0).hit");
        sc_trace(o_vcd, waysel[0].rdata, "/top/cache0/i0/waysel(0).rdata");
        sc_trace(o_vcd, waysel[0].valid, "/top/cache0/i0/waysel(0).valid");
        sc_trace(o_vcd, waysel[0].load_fault, "/top/cache0/i0/waysel(0).load_fault");
        sc_trace(o_vcd, waysel[1].hit, "/top/cache0/i0/waysel(1).hit");
        sc_trace(o_vcd, waysel[1].rdata, "/top/cache0/i0/waysel(1).rdata");
        sc_trace(o_vcd, waysel[1].valid, "/top/cache0/i0/waysel(1).valid");
        sc_trace(o_vcd, waysel[1].load_fault, "/top/cache0/i0/waysel(1).load_fault");
        sc_trace(o_vcd, r.use_overlay, "/top/cache0/i0/r_use_overlay");

        sc_trace(o_vcd, memeven[0].valid, "/top/cache0/i0/memeven0_valid");
        sc_trace(o_vcd, memeven[0].rtag, "/top/cache0/i0/memeven0_rtag");
        sc_trace(o_vcd, memeven[0].rdata, "/top/cache0/i0/memeven0_rdata");
        sc_trace(o_vcd, lrui[0].we, "/top/cache0/i0/lruieven/we");
        sc_trace(o_vcd, lrui[0].lru, "/top/cache0/i0/lruieven/lru");
        sc_trace(o_vcd, lrui[1].we, "/top/cache0/i0/lruiodd/we");
        sc_trace(o_vcd, lrui[1].lru, "/top/cache0/i0/lruiodd/lru");
    }
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        wayevenx[i]->generateVCD(i_vcd, o_vcd);
        wayoddx[i]->generateVCD(i_vcd, o_vcd);
    }
}

void ICacheLru::comb() {
    bool w_raddr5;
    bool w_raddr5_r;
    bool w_use_overlay;
    sc_uint<BUS_ADDR_WIDTH> wb_req_adr;
    sc_uint<BUS_ADDR_WIDTH> wb_radr_overlay;
    sc_uint<CFG_ITAG_WIDTH> wb_rtag;
    sc_uint<CFG_ITAG_WIDTH> wb_rtag_overlay;
    sc_uint<CFG_ITAG_WIDTH> wb_rtag_even;
    sc_uint<CFG_ITAG_WIDTH> wb_rtag_odd;
    sc_uint<3> wb_hit0;
    sc_uint<3> wb_hit1;
    bool w_hit0_valid;
    bool w_hit1_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<32> wb_o_resp_data;
    bool w_ena;
    bool w_dis;
    bool w_last;
    bool w_o_resp_valid;
    bool w_o_resp_load_fault;
    bool w_o_req_ctrl_ready;
    bool w_o_req_mem_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_req_mem_addr;
    sc_uint<4> wb_wstrb_next;
   
    v = r;

    if (i_req_ctrl_valid.read() == 1) {
        wb_req_adr = i_req_ctrl_addr.read();
    } else {
        wb_req_adr = r.req_addr.read();
    }

    w_raddr5 = wb_req_adr[CFG_IOFFSET_WIDTH];
    w_raddr5_r = r.req_addr.read()[CFG_IOFFSET_WIDTH];

    w_use_overlay = 0;
    if (wb_req_adr(CFG_IOFFSET_WIDTH-1, 1) == 0xF) {
        w_use_overlay = 1;
    }

    wb_radr_overlay = wb_req_adr + (1 << CFG_IOFFSET_WIDTH);
    wb_radr_overlay >>= CFG_IOFFSET_WIDTH;
    wb_radr_overlay <<= CFG_IOFFSET_WIDTH;

    // flush request via debug interface
    if (i_flush_valid.read() == 1) {
        v.req_flush = 1;
        if (i_flush_address.read()[0] == 1) {
            v.req_flush_cnt = ~0u;
            v.req_flush_addr = 0xFFFF0000;
        } else if (i_flush_address.read()(CFG_IOFFSET_WIDTH-1, 1) == 0xF) {
            v.req_flush_cnt = 1;
            v.req_flush_addr = i_flush_address.read();
        } else {
            v.req_flush_cnt = 0;
            v.req_flush_addr = i_flush_address.read();
        }
    }

    // Check read tag and select hit way
    wb_rtag = r.req_addr.read()(ITAG_END, ITAG_START);
    wb_rtag_overlay = r.req_addr_overlay.read()(ITAG_END, ITAG_START);
    waysel[WAY_EVEN].hit = MISS;
    waysel[WAY_EVEN].rdata = 0;
    waysel[WAY_EVEN].valid = 0;
    waysel[WAY_EVEN].load_fault = 0;
    waysel[WAY_ODD].hit = MISS;
    waysel[WAY_ODD].rdata = 0;
    waysel[WAY_ODD].valid = 0;
    waysel[WAY_ODD].load_fault = 0;
    if (r.use_overlay.read() == 0) {
        wb_rtag_even = wb_rtag;
        wb_rtag_odd = wb_rtag;
    } else if (w_raddr5_r == 0) {
        wb_rtag_even = wb_rtag;
        wb_rtag_odd = wb_rtag_overlay;
    } else {
        wb_rtag_even = wb_rtag_overlay;
        wb_rtag_odd = wb_rtag;
    }
    for (uint8_t n = 0; n < static_cast<uint8_t>(CFG_ICACHE_WAYS); n++) {
        if (waysel[WAY_EVEN].hit == MISS && memeven[n].rtag == wb_rtag_even) {
            waysel[WAY_EVEN].hit = n;
            waysel[WAY_EVEN].rdata = memeven[n].rdata;
            waysel[WAY_EVEN].valid = memeven[n].valid;
            waysel[WAY_EVEN].load_fault = memeven[n].load_fault;
        }

        if (waysel[WAY_ODD].hit == MISS && memodd[n].rtag == wb_rtag_odd) {
            waysel[WAY_ODD].hit = n;
            waysel[WAY_ODD].rdata = memodd[n].rdata;
            waysel[WAY_ODD].valid = memodd[n].valid;
            waysel[WAY_ODD].load_fault = memodd[n].load_fault;
        }
    }

    // swap back rdata
    w_o_resp_load_fault = 0;
    if (w_raddr5_r == 0) {
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
    if (r.state.read() != State_Flush && w_hit0_valid && w_hit1_valid
        && wb_hit0 != MISS && wb_hit1 != MISS && r.requested.read() == 1) {
        w_o_resp_valid = 1;

        // Update LRU table
        if (w_raddr5_r == 0) {
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

    if (r.state.read() == State_Idle || w_o_resp_valid == 1) {
        if (w_raddr5 == 0) {
            swapin[WAY_EVEN].radr = wb_req_adr;
            swapin[WAY_ODD].radr = wb_radr_overlay;
        } else {
            swapin[WAY_EVEN].radr = wb_radr_overlay;
            swapin[WAY_ODD].radr = wb_req_adr;
        }
    } else {
        if (w_raddr5_r == 0) {
            swapin[WAY_EVEN].radr = r.req_addr.read();
            swapin[WAY_ODD].radr = r.req_addr_overlay.read();
        } else {
            swapin[WAY_EVEN].radr = r.req_addr_overlay.read();
            swapin[WAY_ODD].radr = r.req_addr.read();
        }
    }

    w_o_req_ctrl_ready = !r.req_flush.read()
                       && (!r.requested.read() || w_o_resp_valid);
    if (i_req_ctrl_valid.read() && w_o_req_ctrl_ready) {
        v.req_addr = i_req_ctrl_addr.read();
        v.req_addr_overlay = wb_radr_overlay;
        v.use_overlay = w_use_overlay;
        v.requested = 1;
    } else if (w_o_resp_valid && i_resp_ctrl_ready.read()) {
        v.requested = 0;
    }

    // System Bus access state machine
    w_last = 0;
    w_o_req_mem_valid = 0;
    wb_o_req_mem_addr = 0;
    w_ena = 0;
    w_dis = 0;
    wb_wstrb_next = (r.burst_wstrb.read() << 1) | r.burst_wstrb.read()[3];
    switch (r.state.read()) {
    case State_Idle:
        if (r.req_flush.read() == 1) {
            v.state = State_Flush;
            if (r.req_flush_addr.read()[0] == 1) {
                v.mem_addr = 0xFFFF0000;
                v.flush_cnt = ~0u;
            } else {
                v.mem_addr = r.req_flush_addr.read();
                v.flush_cnt = r.req_flush_cnt.read();
            }
            v.burst_wstrb = ~0u;    // All qwords in line
        } else if (i_req_ctrl_valid.read() == 1 && w_o_req_ctrl_ready == 1) {
            v.state = State_CheckHit;
        }
        break;
    case State_CheckHit:
        if (w_o_resp_valid == 1) {
            // Hit
            if (i_req_ctrl_valid.read() == 1 && w_o_req_ctrl_ready == 1) {
                v.state = State_CheckHit;
            } else {
                v.state = State_Idle;
            }

        } else {
            // Miss
            w_o_req_mem_valid = 1;
            if (!w_hit0_valid || wb_hit0 == MISS) {
                wb_mem_addr = r.req_addr;
            } else {
                wb_mem_addr = r.req_addr_overlay.read();
            }
            if (i_req_mem_ready.read() == 1) {
                v.state = State_WaitResp;
            } else {
                v.state = State_WaitGrant;
            }

            wb_o_req_mem_addr = wb_mem_addr(BUS_ADDR_WIDTH-1, 3) << 3;
            v.mem_addr = wb_o_req_mem_addr;
            v.burst_cnt = 3;
            switch (wb_mem_addr(CFG_IOFFSET_WIDTH-1, 3)) {
            case 0:
                wb_wstrb_next = 0x1;
                break;
            case 1:
                wb_wstrb_next = 0x2;
                break;
            case 2:
                wb_wstrb_next = 0x4;
                break;
            case 3:
                wb_wstrb_next = 0x8;
                break;
            }
            v.burst_wstrb = wb_wstrb_next;
            v.burst_valid = wb_wstrb_next;
            v.lru_even_wr = wb_lru_even;
            v.lru_odd_wr = wb_lru_odd;
        }
        break;
    case State_WaitGrant:
        w_o_req_mem_valid = 1;
        wb_o_req_mem_addr = r.mem_addr;
        if (i_req_mem_ready.read()) {
            v.state = State_WaitResp;
        }
        break;
    case State_WaitResp:
        if (r.burst_cnt.read() == 0) {
            w_last = 1;
        }
        if (i_resp_mem_data_valid.read()) {
            w_ena = 1;
            if (r.burst_cnt.read() == 0) {
                v.state = State_CheckResp;
            } else {
                v.burst_cnt = r.burst_cnt.read() - 1;
            }
            /** Suppose using WRAP burst transaction */
            v.burst_wstrb = wb_wstrb_next;
            v.burst_valid = r.burst_valid.read() | wb_wstrb_next;
        }
        break;
    case State_CheckResp:
        if ((w_o_req_ctrl_ready == 1 && i_req_ctrl_valid.read() == 1)
            || (r.requested.read() == 1 && w_o_resp_valid == 0)) {
            v.state = State_CheckHit;
        } else {
            v.state = State_Idle;
        }
        break;
    case State_Flush:
        w_ena = 1;
        w_dis = 1;
        if (r.flush_cnt.read() == 0) {
            v.req_flush = 0;
            v.state = State_Idle;
        } else {
            v.flush_cnt = r.flush_cnt.read() - 1;
            v.mem_addr = r.mem_addr.read() + (1 << CFG_IOFFSET_WIDTH);
        }
        break;
    default:;
    }

    // Write signals:
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        wb_ena_even[i] = w_dis;
        wb_ena_odd[i] = w_dis;
    }

    if (r.mem_addr.read()[CFG_IOFFSET_WIDTH] == 0) {
        wb_ena_even[r.lru_even_wr.read()] = w_ena;
        swapin[WAY_EVEN].wadr = r.mem_addr.read();
        swapin[WAY_EVEN].wstrb = r.burst_wstrb.read();
        swapin[WAY_EVEN].wvalid = r.burst_valid.read();
        swapin[WAY_EVEN].wdata = i_resp_mem_data.read();
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
        swapin[WAY_ODD].wdata = i_resp_mem_data.read();
        swapin[WAY_ODD].load_fault = i_resp_mem_load_fault.read();
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_req_ctrl_ready = w_o_req_ctrl_ready;

    o_req_mem_valid = w_o_req_mem_valid;
    o_req_mem_addr = wb_o_req_mem_addr;
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;
    o_req_mem_len = 3;
    o_req_mem_burst = 2;
    o_req_mem_last = w_last;

    o_resp_ctrl_valid = w_o_resp_valid;
    o_resp_ctrl_data = wb_o_resp_data;
    o_resp_ctrl_addr = r.req_addr.read();
    o_resp_ctrl_load_fault = w_o_resp_load_fault;
    o_istate = r.state.read()(1, 0);
}

void ICacheLru::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

#ifdef DBG_ICACHE_LRU_TB
ICacheLru_tb::ICacheLru_tb(sc_module_name name_) : sc_module(name_),
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

    tt = new ICacheLru("tt", 0, CFG_IINDEX_WIDTH);
    tt->i_clk(w_clk);
    tt->i_nrst(w_nrst);
    tt->i_req_ctrl_valid(w_req_ctrl_valid);
    tt->i_req_ctrl_addr(wb_req_ctrl_addr);
    tt->o_req_ctrl_ready(w_req_ctrl_ready);
    tt->o_resp_ctrl_valid(w_resp_ctrl_valid);
    tt->o_resp_ctrl_addr(wb_resp_ctrl_addr);
    tt->o_resp_ctrl_data(wb_resp_ctrl_data);
    tt->o_resp_ctrl_load_fault(w_resp_ctrl_load_fault);
    tt->i_resp_ctrl_ready(w_resp_ctrl_ready);
    tt->i_req_mem_ready(w_req_mem_ready);
    tt->o_req_mem_valid(w_req_mem_valid);
    tt->o_req_mem_write(w_req_mem_write);
    tt->o_req_mem_addr(wb_req_mem_addr);
    tt->o_req_mem_strob(wb_req_mem_strob);
    tt->o_req_mem_data(wb_req_mem_data);
    tt->i_resp_mem_data_valid(w_resp_mem_data_valid);
    tt->i_resp_mem_data(wb_resp_mem_data);
    tt->i_resp_mem_load_fault(w_resp_mem_load_fault);
    tt->i_flush_address(wb_flush_address);
    tt->i_flush_valid(w_flush_valid);
    tt->o_istate(wb_istate);

    tb_vcd = sc_create_vcd_trace_file("ICacheLru_tb");
    tb_vcd->set_time_unit(1, SC_PS);
    sc_trace(tb_vcd, w_nrst, "w_nrst");
    sc_trace(tb_vcd, w_clk, "w_clk");
    sc_trace(tb_vcd, r.clk_cnt, "clk_cnt");
    sc_trace(tb_vcd, w_req_ctrl_valid, "w_req_ctrl_valid");
    sc_trace(tb_vcd, wb_req_ctrl_addr, "wb_req_ctrl_addr");
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


void ICacheLru_tb::comb0() {
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

void ICacheLru_tb::comb_fetch() {
    w_req_ctrl_valid = 0;
    wb_req_ctrl_addr = 0;

    switch (r.clk_cnt.read()) {
    case 10 + 1 + (1 << (2*(CFG_IINDEX_WIDTH+1))):
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x00000000;
        break;
    case 1021:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x100008f4;
        break;
    case 1024:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x100008f6;
        break;
    case 1025:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x100008fa;
        break;
    case 1026:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x100008fe;
        break;

    case 1050:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x0000001e;
        break;
    case 1060:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x0000201e;
        break;
    case 1070:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x0010001e;
        break;
    case 1081:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x0010201e;
        break;

    case 1100:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x00000004;
        break;
    case 1101:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x00000008;
        break;
    case 1102:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x00002008;
        break;
    case 1103:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x0000200C;
        break;
    case 1104:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x00102010;
        break;
    case 1105:
        w_req_ctrl_valid = 1;
        wb_req_ctrl_addr = 0x00102014;
        break;
    default:;
    }
}

void ICacheLru_tb::comb_bus() {
    sc_uint<CFG_IOFFSET_WIDTH> wrap_offset;
    sc_uint<32> wb_addr4;
    vbus = rbus;

    w_req_mem_ready = 0;
    w_resp_mem_data_valid = 0;
    wb_resp_mem_data = 0;
    wb_addr4 = rbus.burst_addr.read() + 4;

    switch (rbus.state.read()) {
    case BUS_Idle:
        w_req_mem_ready = 1;
        if (w_req_mem_valid.read() == 1) {
            vbus.state = BUS_Read;
            vbus.burst_addr = wb_req_mem_addr.read();
            vbus.burst_cnt = 3;
        }
        break;
    case BUS_Read:
        w_resp_mem_data_valid = 1;
        wb_resp_mem_data = (wb_addr4, rbus.burst_addr.read());
        vbus.burst_cnt = rbus.burst_cnt.read() - 1;
        wrap_offset = rbus.burst_addr.read()(CFG_IOFFSET_WIDTH-1, 0) + 8;
        // WRAP burst transaction type
        vbus.burst_addr =
            (rbus.burst_addr.read()(31, CFG_IOFFSET_WIDTH), wrap_offset);
        if (rbus.burst_cnt.read() == 1) {
            vbus.state = BUS_ReadLast;
        }
        break;
    case BUS_ReadLast:
        w_req_mem_ready = 1;
        w_resp_mem_data_valid = 1;
        wb_resp_mem_data = (wb_addr4, rbus.burst_addr.read());
        if (w_req_mem_valid.read() == 1) {
            vbus.state = BUS_Read;
            vbus.burst_addr = wb_req_mem_addr.read();
            vbus.burst_cnt = 3;
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

