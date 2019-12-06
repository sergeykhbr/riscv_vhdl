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
    int index_width) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_ctrl_valid("i_req_ctrl_valid"),
    i_req_ctrl_addr("i_req_ctrl_addr"),
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
        wayevenx[i]->i_executable(swapin[WAY_EVEN].executable);
        wayevenx[i]->i_readable(swapin[WAY_EVEN].readable);
        wayevenx[i]->i_writable(swapin[WAY_EVEN].writable);
        wayevenx[i]->o_rtag(wayeven_o[i].rtag);
        wayevenx[i]->o_rdata(wayeven_o[i].rdata);
        wayevenx[i]->o_valid(wayeven_o[i].valid);
        wayevenx[i]->o_load_fault(wayeven_o[i].load_fault);
        wayevenx[i]->o_executable(wayeven_o[i].executable);
        wayevenx[i]->o_readable(wayeven_o[i].readable);
        wayevenx[i]->o_writable(wayeven_o[i].writable);

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
        wayoddx[i]->i_executable(swapin[WAY_ODD].executable);
        wayoddx[i]->i_readable(swapin[WAY_ODD].readable);
        wayoddx[i]->i_writable(swapin[WAY_ODD].writable);
        wayoddx[i]->o_rtag(wayodd_o[i].rtag);
        wayoddx[i]->o_rdata(wayodd_o[i].rdata);
        wayoddx[i]->o_valid(wayodd_o[i].valid);
        wayoddx[i]->o_load_fault(wayodd_o[i].load_fault);
        wayoddx[i]->o_executable(wayodd_o[i].executable);
        wayoddx[i]->o_readable(wayodd_o[i].readable);
        wayoddx[i]->o_writable(wayodd_o[i].writable);
    }

    lrueven = new ILru("lrueven0");
    lrueven->i_clk(i_clk);
    lrueven->i_init(lrui[WAY_EVEN].init);
    lrueven->i_radr(lrui[WAY_EVEN].radr);
    lrueven->i_wadr(lrui[WAY_EVEN].wadr);
    lrueven->i_we(lrui[WAY_EVEN].we);
    lrueven->i_lru(lrui[WAY_EVEN].lru);
    lrueven->o_lru(wb_lru_even);

    lruodd = new ILru("lruodd0");
    lruodd->i_clk(i_clk);
    lruodd->i_init(lrui[WAY_ODD].init);
    lruodd->i_radr(lrui[WAY_ODD].radr);
    lruodd->i_wadr(lrui[WAY_ODD].wadr);
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
    sensitive << i_mpu_cachable;
    sensitive << i_mpu_executable;
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        sensitive << wayeven_o[i].rtag;
        sensitive << wayeven_o[i].rdata;
        sensitive << wayeven_o[i].valid;
        sensitive << wayeven_o[i].load_fault;
        sensitive << wayeven_o[i].executable;
        sensitive << wayeven_o[i].readable;
        sensitive << wayeven_o[i].writable;
        sensitive << wayodd_o[i].rtag;
        sensitive << wayodd_o[i].rdata;
        sensitive << wayodd_o[i].valid;
        sensitive << wayodd_o[i].load_fault;
        sensitive << wayodd_o[i].executable;
        sensitive << wayodd_o[i].readable;
        sensitive << wayodd_o[i].writable;
    }
    sensitive << wb_lru_even;
    sensitive << wb_lru_odd;
    sensitive << r.requested;
    sensitive << r.req_addr;
    sensitive << r.req_addr_overlay;
    sensitive << r.use_overlay;
    sensitive << r.state;
    sensitive << r.req_mem_valid;
    sensitive << r.mem_addr;
    sensitive << r.burst_cnt;
    sensitive << r.burst_wstrb;
    sensitive << r.lru_even_wr;
    sensitive << r.lru_odd_wr;
    sensitive << r.cached;
    sensitive << r.load_fault;
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
        sc_trace(o_vcd, i_nrst, i_nrst.name());
        sc_trace(o_vcd, i_req_ctrl_valid, i_req_ctrl_valid.name());
        sc_trace(o_vcd, i_req_ctrl_addr, i_req_ctrl_addr.name());
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
        sc_trace(o_vcd, r.lru_even_wr, pn + ".r_lru_even_wr");
        sc_trace(o_vcd, r.lru_odd_wr, pn + ".r_lru_odd_wr");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_addr_overlay, pn + ".r_req_addr_overlay");
        sc_trace(o_vcd, wb_ena_even[0], pn + ".wb_ena_even0");
        sc_trace(o_vcd, wb_ena_even[1], pn + ".wb_ena_even1");
        sc_trace(o_vcd, wb_ena_even[2], pn + ".wb_ena_even2");
        sc_trace(o_vcd, wb_ena_even[3], pn + ".wb_ena_even3");
        sc_trace(o_vcd, wb_ena_odd[0], pn + ".wb_ena_odd0");
        sc_trace(o_vcd, wb_ena_odd[1], pn + ".wb_ena_odd1");
        sc_trace(o_vcd, wb_ena_odd[2], pn + ".wb_ena_odd2");
        sc_trace(o_vcd, wb_ena_odd[3], pn + ".wb_ena_odd3");
        sc_trace(o_vcd, swapin[0].radr, pn + ".swapin(0).radr");
        sc_trace(o_vcd, swapin[0].wadr, pn + ".swapin(0).wadr");
        sc_trace(o_vcd, swapin[0].wstrb, pn + ".swapin(0).wstrb");
        sc_trace(o_vcd, swapin[0].wvalid, pn + ".swapin(0).wvalid");
        sc_trace(o_vcd, swapin[0].wdata, pn + ".swapin(0).wdata");
        sc_trace(o_vcd, swapin[0].load_fault, pn + ".swapin(0).load_fault");
        sc_trace(o_vcd, swapin[1].radr, pn + ".swapin(1).radr");
        sc_trace(o_vcd, swapin[1].wadr, pn + ".swapin(1).wadr");
        sc_trace(o_vcd, swapin[1].wstrb, pn + ".swapin(1).wstrb");
        sc_trace(o_vcd, swapin[1].wvalid, pn + ".swapin(1).wvalid");
        sc_trace(o_vcd, swapin[1].wdata, pn + ".swapin(1).wdata");
        sc_trace(o_vcd, swapin[1].load_fault, pn + ".swapin(1).load_fault");
        sc_trace(o_vcd, waysel[0].hit, pn + ".waysel(0).hit");
        sc_trace(o_vcd, waysel[0].rdata, pn + ".waysel(0).rdata");
        sc_trace(o_vcd, waysel[0].valid, pn + ".waysel(0).valid");
        sc_trace(o_vcd, waysel[0].load_fault, pn + ".waysel(0).load_fault");
        sc_trace(o_vcd, waysel[0].executable, pn + ".waysel(0).executable");
        sc_trace(o_vcd, waysel[1].hit, pn + ".waysel(1).hit");
        sc_trace(o_vcd, waysel[1].rdata, pn + ".waysel(1).rdata");
        sc_trace(o_vcd, waysel[1].valid, pn + ".waysel(1).valid");
        sc_trace(o_vcd, waysel[1].load_fault, pn + ".waysel(1).load_fault");
        sc_trace(o_vcd, r.use_overlay, pn + ".r_use_overlay");
        sc_trace(o_vcd, r.cached, pn + ".r_cached");
        sc_trace(o_vcd, r.cache_line_i, pn + ".r_cache_line_i");
        sc_trace(o_vcd, r.executable, pn + ".r_executable");
    }
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        wayevenx[i]->generateVCD(i_vcd, o_vcd);
        wayoddx[i]->generateVCD(i_vcd, o_vcd);
    }
    lrueven->generateVCD(i_vcd, o_vcd);
    lruodd->generateVCD(i_vcd, o_vcd);
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
    sc_uint<4> wb_wstrb_next;
    bool vb_ena_even[CFG_ICACHE_WAYS];
    bool vb_ena_odd[CFG_ICACHE_WAYS];
    LruInTypeVariable v_lrui[WAY_SubNum];
    TagMemInTypeVariable v_swapin[WAY_SubNum];
    sc_biguint<4*BUS_DATA_WIDTH> t_cache_line_i;
    sc_uint<32> vb_cached_data;
    bool v_cached_valid;
    bool v_uncached_valid;
    sc_uint<32> vb_uncached_data;
    bool w_o_resp_valid;
    sc_uint<32> wb_o_resp_data;
    sc_uint<8> v_req_mem_len;
   
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
    wb_rtag = r.req_addr.read()(ITAG_END, ITAG_START);
    wb_rtag_overlay = r.req_addr_overlay.read()(ITAG_END, ITAG_START);
    waysel[WAY_EVEN].hit = MISS;
    waysel[WAY_EVEN].rdata = 0;
    waysel[WAY_EVEN].valid = 0;
    waysel[WAY_EVEN].load_fault = 0;
    waysel[WAY_EVEN].executable = 0;
    waysel[WAY_EVEN].readable = 0;
    waysel[WAY_EVEN].writable = 0;
    waysel[WAY_ODD].hit = MISS;
    waysel[WAY_ODD].rdata = 0;
    waysel[WAY_ODD].valid = 0;
    waysel[WAY_ODD].load_fault = 0;
    waysel[WAY_ODD].executable = 0;
    waysel[WAY_ODD].readable = 0;
    waysel[WAY_ODD].writable = 0;
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
        if (waysel[WAY_EVEN].hit == MISS && wayeven_o[n].rtag == wb_rtag_even) {
            waysel[WAY_EVEN].hit = n;
            waysel[WAY_EVEN].rdata = wayeven_o[n].rdata;
            waysel[WAY_EVEN].valid = wayeven_o[n].valid;
            waysel[WAY_EVEN].load_fault = wayeven_o[n].load_fault;
            waysel[WAY_EVEN].executable = wayeven_o[n].executable;
            waysel[WAY_EVEN].readable = wayeven_o[n].readable;
            waysel[WAY_EVEN].writable = wayeven_o[n].writable;
        }

        if (waysel[WAY_ODD].hit == MISS && wayodd_o[n].rtag == wb_rtag_odd) {
            waysel[WAY_ODD].hit = n;
            waysel[WAY_ODD].rdata = wayodd_o[n].rdata;
            waysel[WAY_ODD].valid = wayodd_o[n].valid;
            waysel[WAY_ODD].load_fault = wayodd_o[n].load_fault;
            waysel[WAY_ODD].executable = wayodd_o[n].executable;
            waysel[WAY_ODD].writable = wayodd_o[n].writable;
            waysel[WAY_ODD].readable= wayodd_o[n].readable;
        }
    }

    // swap back rdata
    v_cached_load_fault = 0;
    if (w_raddr5_r == 0) {
        if (r.use_overlay.read() == 0) {
            wb_hit0 = waysel[WAY_EVEN].hit;
            wb_hit1 = waysel[WAY_EVEN].hit;
            w_hit0_valid = waysel[WAY_EVEN].valid;
            w_hit1_valid = waysel[WAY_EVEN].valid;
            vb_cached_data = waysel[WAY_EVEN].rdata;
            v_cached_load_fault = waysel[WAY_EVEN].load_fault;
            v_cached_executable = waysel[WAY_EVEN].executable;
            v_cached_writable = waysel[WAY_EVEN].writable;
            v_cached_readable = waysel[WAY_EVEN].readable;
        } else {
            wb_hit0 = waysel[WAY_EVEN].hit;
            wb_hit1 = waysel[WAY_ODD].hit;
            w_hit0_valid = waysel[WAY_EVEN].valid;
            w_hit1_valid = waysel[WAY_ODD].valid;
            vb_cached_data(15, 0) = waysel[WAY_EVEN].rdata(15, 0);
            vb_cached_data(31, 16) = waysel[WAY_ODD].rdata(15, 0);
            v_cached_load_fault =
                waysel[WAY_EVEN].load_fault | waysel[WAY_ODD].load_fault;
            v_cached_executable =
                waysel[WAY_EVEN].executable & waysel[WAY_ODD].executable;
            v_cached_writable =
                waysel[WAY_EVEN].writable & waysel[WAY_ODD].writable;
            v_cached_readable =
                waysel[WAY_EVEN].readable & waysel[WAY_ODD].readable;
        }
    } else {
        if (r.use_overlay.read() == 0) {
            wb_hit0 = waysel[WAY_ODD].hit;
            wb_hit1 = waysel[WAY_ODD].hit;
            w_hit0_valid = waysel[WAY_ODD].valid;
            w_hit1_valid = waysel[WAY_ODD].valid;
            vb_cached_data = waysel[WAY_ODD].rdata;
            v_cached_load_fault = waysel[WAY_ODD].load_fault;
            v_cached_executable = waysel[WAY_ODD].executable;
            v_cached_writable = waysel[WAY_ODD].writable;
            v_cached_readable = waysel[WAY_ODD].readable;
        } else {
            wb_hit0 = waysel[WAY_ODD].hit;
            wb_hit1 = waysel[WAY_EVEN].hit;
            w_hit0_valid = waysel[WAY_ODD].valid;
            w_hit1_valid = waysel[WAY_EVEN].valid;
            vb_cached_data(15, 0) = waysel[WAY_ODD].rdata(15, 0);
            vb_cached_data(31, 16) = waysel[WAY_EVEN].rdata(15, 0);
            v_cached_load_fault =
                waysel[WAY_ODD].load_fault | waysel[WAY_EVEN].load_fault;
            v_cached_executable =
                waysel[WAY_ODD].executable & waysel[WAY_EVEN].executable;
            v_cached_writable =
                waysel[WAY_ODD].writable & waysel[WAY_EVEN].writable;
            v_cached_readable =
                waysel[WAY_ODD].readable & waysel[WAY_EVEN].readable;
        }
    }

    v_lrui[WAY_EVEN].init = 0;
    v_lrui[WAY_EVEN].radr = 0;
    v_lrui[WAY_EVEN].wadr = 0;
    v_lrui[WAY_EVEN].we = 0;
    v_lrui[WAY_EVEN].lru = 0;
    v_lrui[WAY_ODD].init = 0;
    v_lrui[WAY_ODD].radr = 0;
    v_lrui[WAY_ODD].wadr = 0;
    v_lrui[WAY_ODD].we = 0;
    v_lrui[WAY_ODD].lru = 0;
    v_cached_valid = 0;
    if (r.state.read() == State_Flush) {
        v_lrui[WAY_EVEN].init = !r.mem_addr.read()[CFG_IOFFSET_WIDTH];
        v_lrui[WAY_EVEN].wadr = r.mem_addr.read()(IINDEX_END, IINDEX_START);
        v_lrui[WAY_ODD].init = r.mem_addr.read()[CFG_IOFFSET_WIDTH];
        v_lrui[WAY_ODD].wadr = r.mem_addr.read()(IINDEX_END, IINDEX_START);
    } else if (r.state.read() == State_WaitGrant
            || r.state.read() == State_WaitResp || r.state.read() == State_CheckResp
            || r.state.read() == State_WriteLine
            || r.state.read() == State_SetupReadAdr) {
            // Do nothing while memory writing
    } else if (w_hit0_valid && w_hit1_valid
        && wb_hit0 != MISS && wb_hit1 != MISS && r.requested.read() == 1) {
        v_cached_valid = 1;

        // Update LRU table
        if (w_raddr5_r == 0) {
            v_lrui[WAY_EVEN].we = 1;
            v_lrui[WAY_EVEN].lru = wb_hit0(1, 0);
            v_lrui[WAY_EVEN].wadr =
                r.req_addr.read()(IINDEX_END, IINDEX_START);
            if (r.use_overlay.read() == 1) {
                v_lrui[WAY_ODD].we = 1;
                v_lrui[WAY_ODD].lru = wb_hit1(1, 0);
                v_lrui[WAY_ODD].wadr =
                    r.req_addr_overlay.read()(IINDEX_END, IINDEX_START);
            }
        } else {
            v_lrui[WAY_ODD].we = 1;
            v_lrui[WAY_ODD].lru = wb_hit0(1, 0);
            v_lrui[WAY_ODD].wadr =
                r.req_addr.read()(IINDEX_END, IINDEX_START);
            if (r.use_overlay.read() == 1) {
                v_lrui[WAY_EVEN].we = 1;
                v_lrui[WAY_EVEN].lru = wb_hit1(1, 0);
                v_lrui[WAY_EVEN].wadr =
                    r.req_addr_overlay.read()(IINDEX_END, IINDEX_START);
            }
        }
    }

    w_o_req_ctrl_ready = !r.req_flush.read()
                       && (!r.requested.read() || v_cached_valid);
    if (i_req_ctrl_valid.read() && w_o_req_ctrl_ready) {
        v.req_addr = i_req_ctrl_addr.read();
        v.req_addr_overlay = wb_radr_overlay;
        v.use_overlay = w_use_overlay;
        v.requested = 1;
    } else if (v_cached_valid && i_resp_ctrl_ready.read()) {
        v.requested = 0;
    }

    // System Bus access state machine
    w_last = 0;
    v_line_valid = 0;
    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        vb_ena_even[i] = 0;
        vb_ena_odd[i] = 0;
    }
    t_cache_line_i = r.cache_line_i.read();
    v_uncached_valid = 0;
    vb_uncached_data = 0;
    v_req_mem_len = 3;

    wb_wstrb_next = r.burst_wstrb.read() << 1;
    switch (r.state.read()) {
    case State_Idle:
        if (r.req_flush.read() == 1) {
            v.state = State_Flush;
            t_cache_line_i = 0;
            v.cache_line_i = ~t_cache_line_i;
            if (r.req_flush_addr.read()[0] == 1) {
                v.mem_addr = FLUSH_ALL_ADDR;
                v.flush_cnt = ~0u;
            } else {
                v.mem_addr = r.req_flush_addr.read();
                v.flush_cnt = r.req_flush_cnt.read();
            }
        } else if ((i_req_ctrl_valid.read() == 1 && w_o_req_ctrl_ready == 1)
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
            if (i_req_ctrl_valid.read() == 1 && w_o_req_ctrl_ready == 1) {
                v.state = State_CheckHit;
            } else {
                v.state = State_Idle;
            }

        } else {
            // Miss
            if (!w_hit0_valid || wb_hit0 == MISS) {
                wb_mpu_addr = r.req_addr.read();
            } else {
                wb_mpu_addr = r.req_addr_overlay.read();
            }
            v.state = State_CheckMPU;
            v.mpu_addr = wb_mpu_addr;
            v.lru_even_wr = wb_lru_even;
            v.lru_odd_wr = wb_lru_odd;
        }
        break;
    case State_CheckMPU:
        v.req_mem_valid = 1;
        if (i_req_mem_ready.read() == 1) {
            v.state = State_WaitResp;
        } else {
            v.state = State_WaitGrant;
        }

        if (i_mpu_cachable.read() == 1) {
            v.mem_addr = r.mpu_addr.read()(BUS_ADDR_WIDTH-1, CFG_IOFFSET_WIDTH)
                        << CFG_IOFFSET_WIDTH;
            v.burst_cnt = 3;
            v.cached = 1;
        } else {
            v.mem_addr = r.mpu_addr.read()(BUS_ADDR_WIDTH-1, 3) << 3;
            v.burst_cnt = 1;
            v.cached = 0;
            v_req_mem_len = 1;  // default cached = 3
        }
        wb_wstrb_next = 0x1;
        v.burst_wstrb = wb_wstrb_next;
        v.cache_line_i = 0;
        v.load_fault = 0;
        v.executable = i_mpu_executable.read();
        v.writable = i_mpu_writable.read();
        v.readable = i_mpu_readable.read();
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            v.state = State_WaitResp;
            v.req_mem_valid = 0;
        }
        if (r.cached.read() == 0) {
            v_req_mem_len = 1;
        }
        break;
    case State_WaitResp:
        if (r.burst_cnt.read() == 0) {
            w_last = 1;
        }
        if (i_resp_mem_data_valid.read()) {
            for (int k = 0; k < 4; k++) {
                if (r.burst_wstrb.read()[k] == 1) {
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
            v.burst_wstrb = wb_wstrb_next;
            if (i_resp_mem_load_fault.read() == 1) {
                v.load_fault = 1;
            }
        }
        break;
    case State_CheckResp:
        if (r.cached.read() == 1) {
            v.state = State_SetupReadAdr;
            v_line_valid = 1;
            if (r.mem_addr.read()[CFG_IOFFSET_WIDTH] == 0) {
                vb_ena_even[r.lru_even_wr.read().to_int()] = 1;
            } else {
                vb_ena_odd[r.lru_odd_wr.read().to_int()] = 1;
            }
        } else {
            v_uncached_valid = 1;
            for (unsigned i = 0; i < 4; i++) {
                if (r.mpu_addr.read()(2, 1) == i) {
                    vb_uncached_data = r.cache_line_i.read()(16*i+31, 16*i);
                }
            }
            if (i_resp_ctrl_ready.read() == 1) {
                v.state = State_Idle;
                v.requested = 0;
            }
        }
        break;
    case State_WriteLine:
        v.state = State_SetupReadAdr;
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
            v.mem_addr = r.mem_addr.read() + (1 << CFG_IOFFSET_WIDTH);
        }
        for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
            vb_ena_even[i] = 1;
            vb_ena_odd[i] = 1;
        }
        break;
    default:;
    }

    // Write signals:
    v_swapin[WAY_EVEN].wadr = r.mem_addr.read();
    v_swapin[WAY_EVEN].wstrb = ~0u;
    v_swapin[WAY_EVEN].wvalid = v_line_valid;
    v_swapin[WAY_EVEN].wdata = r.cache_line_i.read();
    v_swapin[WAY_EVEN].load_fault = r.load_fault.read();
    v_swapin[WAY_EVEN].executable = r.executable.read();
    v_swapin[WAY_EVEN].writable = r.writable.read();
    v_swapin[WAY_EVEN].readable = r.readable.read();
    v_swapin[WAY_ODD].wadr = r.mem_addr.read();
    v_swapin[WAY_ODD].wstrb = ~0u;
    v_swapin[WAY_ODD].wvalid = v_line_valid;
    v_swapin[WAY_ODD].wdata = r.cache_line_i.read();
    v_swapin[WAY_ODD].load_fault = r.load_fault.read();
    v_swapin[WAY_ODD].executable = r.executable.read();
    v_swapin[WAY_ODD].writable = r.writable.read();
    v_swapin[WAY_ODD].readable = r.readable.read();

    if (r.state.read() == State_WaitResp
        || r.state.read() == State_CheckResp
        || r.state.read() == State_Flush) {
        v_swapin[WAY_EVEN].radr = r.mem_addr.read();
        v_swapin[WAY_ODD].radr = r.mem_addr.read();
    } else if (r.state.read() == State_Idle || v_cached_valid == 1) {
        if (w_raddr5 == 0) {
            v_swapin[WAY_EVEN].radr = wb_req_adr;
            v_swapin[WAY_ODD].radr = wb_radr_overlay;
            v_lrui[WAY_EVEN].radr = wb_req_adr(IINDEX_END, IINDEX_START);
            v_lrui[WAY_ODD].radr = wb_radr_overlay(IINDEX_END, IINDEX_START);
        } else {
            v_swapin[WAY_EVEN].radr = wb_radr_overlay;
            v_swapin[WAY_ODD].radr = wb_req_adr;
            v_lrui[WAY_EVEN].radr = wb_radr_overlay(IINDEX_END, IINDEX_START);
            v_lrui[WAY_ODD].radr = wb_req_adr(IINDEX_END, IINDEX_START);
        }
    } else {
        if (w_raddr5_r == 0) {
            v_swapin[WAY_EVEN].radr = r.req_addr.read();
            v_swapin[WAY_ODD].radr = r.req_addr_overlay.read();
            v_lrui[WAY_EVEN].radr =
                r.req_addr.read()(IINDEX_END, IINDEX_START);
            v_lrui[WAY_ODD].radr =
                r.req_addr_overlay.read()(IINDEX_END, IINDEX_START);
        } else {
            v_swapin[WAY_EVEN].radr = r.req_addr_overlay.read();
            v_swapin[WAY_ODD].radr = r.req_addr.read();
            v_lrui[WAY_EVEN].radr =
                r.req_addr_overlay.read()(IINDEX_END, IINDEX_START);
            v_lrui[WAY_ODD].radr =
                r.req_addr.read()(IINDEX_END, IINDEX_START);
        }
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    for (int i = 0; i < WAY_SubNum; i++) {
        lrui[i].init = v_lrui[i].init;
        lrui[i].radr = v_lrui[i].radr;
        lrui[i].wadr = v_lrui[i].wadr;
        lrui[i].we = v_lrui[i].we;
        lrui[i].lru = v_lrui[i].lru;

        swapin[i].radr = v_swapin[i].radr;
        swapin[i].wadr = v_swapin[i].wadr;
        swapin[i].wstrb = v_swapin[i].wstrb;
        swapin[i].wvalid = v_swapin[i].wvalid;
        swapin[i].wdata = v_swapin[i].wdata;
        swapin[i].load_fault = v_swapin[i].load_fault;
        swapin[i].executable = v_swapin[i].executable;
        swapin[i].writable = v_swapin[i].writable;
        swapin[i].readable = v_swapin[i].readable;
    }

    for (int i = 0; i < CFG_ICACHE_WAYS; i++) {
        wb_ena_even[i] = vb_ena_even[i];
        wb_ena_odd[i] = vb_ena_odd[i];
    }

    o_req_ctrl_ready = w_o_req_ctrl_ready;

    o_req_mem_valid = r.req_mem_valid.read();
    o_req_mem_addr = r.mem_addr.read();
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;
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

