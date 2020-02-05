/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "rtl_wrapper.h"
#include "iservice.h"
#include "ihap.h"
#include <riscv-isa.h>
#include "coreservices/iserial.h"

namespace debugger {

RtlWrapper::RtlWrapper(IFace *parent, sc_module_name name) : sc_module(name),
    o_clk("clk", 10, SC_NS),
    o_nrst("o_nrst"),
    i_time("i_time"),
    o_msti_aw_ready("o_msti_aw_ready"),
    o_msti_w_ready("o_msti_w_ready"),
    o_msti_b_valid("o_msti_b_valid"),
    o_msti_b_resp("o_msti_b_resp"),
    o_msti_b_id("o_msti_b_id"),
    o_msti_b_user("o_msti_b_user"),
    o_msti_ar_ready("o_msti_ar_ready"),
    o_msti_r_valid("o_msti_r_valid"),
    o_msti_r_resp("o_msti_r_resp"),
    o_msti_r_data("o_msti_r_data"),
    o_msti_r_last("o_msti_r_last"),
    o_msti_r_id("o_msti_r_id"),
    o_msti_r_user("o_msti_r_user"),
    i_msto_aw_valid("i_msto_aw_valid"),
    i_msto_aw_bits_addr("i_msto_aw_bits_addr"),
    i_msto_aw_bits_len("i_msto_aw_bits_len"),
    i_msto_aw_bits_size("i_msto_aw_bits_size"),
    i_msto_aw_bits_burst("i_msto_aw_bits_burst"),
    i_msto_aw_bits_lock("i_msto_aw_bits_lock"),
    i_msto_aw_bits_cache("i_msto_aw_bits_cache"),
    i_msto_aw_bits_prot("i_msto_aw_bits_prot"),
    i_msto_aw_bits_qos("i_msto_aw_bits_qos"),
    i_msto_aw_bits_region("i_msto_aw_bits_region"),
    i_msto_aw_id("i_msto_aw_id"),
    i_msto_aw_user("i_msto_aw_user"),
    i_msto_w_valid("i_msto_w_valid"),
    i_msto_w_data("i_msto_w_data"),
    i_msto_w_last("i_msto_w_last"),
    i_msto_w_strb("i_msto_w_strb"),
    i_msto_w_user("i_msto_w_user"),
    i_msto_b_ready("i_msto_b_ready"),
    i_msto_ar_valid("i_msto_ar_valid"),
    i_msto_ar_bits_addr("i_msto_ar_bits_addr"),
    i_msto_ar_bits_len("i_msto_ar_bits_len"),
    i_msto_ar_bits_size("i_msto_ar_bits_size"),
    i_msto_ar_bits_burst("i_msto_ar_bits_burst"),
    i_msto_ar_bits_lock("i_msto_ar_bits_lock"),
    i_msto_ar_bits_cache("i_msto_ar_bits_cache"),
    i_msto_ar_bits_prot("i_msto_ar_bits_prot"),
    i_msto_ar_bits_qos("i_msto_ar_bits_qos"),
    i_msto_ar_bits_region("i_msto_ar_bits_region"),
    i_msto_ar_id("i_msto_ar_id"),
    i_msto_ar_user("i_msto_ar_user"),
    i_msto_r_ready("i_msto_r_ready"),
    o_msti_ac_valid("o_msti_ac_valid"),
    o_msti_ac_addr("o_msti_ac_addr"),
    o_msti_ac_snoop("o_msti_ac_snoop"),
    o_msti_ac_prot("o_msti_ac_prot"),
    o_msti_cr_ready("o_msti_cr_ready"),
    o_msti_cd_ready("o_msti_cd_ready"),
    i_msto_ar_domain("i_msto_ar_domain"),
    i_msto_ar_snoop("i_msto_ar_snoop"),
    i_msto_ar_bar("i_msto_ar_bar"),
    i_msto_aw_domain("i_msto_aw_domain"),
    i_msto_aw_snoop("i_msto_aw_snoop"),
    i_msto_aw_bar("i_msto_aw_bar"),
    i_msto_ac_ready("i_msto_ac_ready"),
    i_msto_cr_valid("i_msto_cr_valid"),
    i_msto_cr_resp("i_msto_cr_resp"),
    i_msto_cd_valid("i_msto_cd_valid"),
    i_msto_cd_data("i_msto_cd_data"),
    i_msto_cd_last("i_msto_cd_last"),
    i_msto_rack("i_msto_rack"),
    i_msto_wack("i_msto_wack"),
    o_interrupt("o_interrupt"),
    o_dport_valid("o_dport_valid"),
    o_dport_write("o_dport_write"),
    o_dport_region("o_dport_region"),
    o_dport_addr("o_dport_addr"),
    o_dport_wdata("o_dport_wdata"),
    i_dport_ready("i_dport_ready"),
    i_dport_rdata("i_dport_rdata"),
    i_halted("i_halted") {
    iparent_ = parent;
    clockCycles_ = 1000000; // 1 MHz when default resolution = 1 ps

    v.nrst = 1;//0;
    v.req_addr = 0;
    v.req_len = 0;
    v.req_burst = 0;
    v.b_valid = 0;
    v.b_resp = 0;
    v.interrupt = false;
    async_interrupt = 0;
    request_reset = false;
    w_interrupt = 0;
    v.halted = false;
    v.state = State_Idle;
    r.state = State_Idle;
    RISCV_event_create(&dport_.valid, "dport_valid");
    dport_.trans_idx_up = 0;
    dport_.trans_idx_down = 0;
    trans.source_idx = 0;//CFG_NASTI_MASTER_CACHED;

    SC_METHOD(comb);
    sensitive << w_interrupt;
    sensitive << i_halted;
    sensitive << i_msto_aw_valid;
    sensitive << i_msto_aw_bits_addr;
    sensitive << i_msto_aw_bits_len;
    sensitive << i_msto_aw_bits_size;
    sensitive << i_msto_aw_bits_burst;
    sensitive << i_msto_aw_bits_lock;
    sensitive << i_msto_aw_bits_cache;
    sensitive << i_msto_aw_bits_prot;
    sensitive << i_msto_aw_bits_qos;
    sensitive << i_msto_aw_bits_region;
    sensitive << i_msto_aw_id;
    sensitive << i_msto_aw_user;
    sensitive << i_msto_w_valid;
    sensitive << i_msto_w_data;
    sensitive << i_msto_w_last;
    sensitive << i_msto_w_strb;
    sensitive << i_msto_w_user;
    sensitive << i_msto_b_ready;
    sensitive << i_msto_ar_valid;
    sensitive << i_msto_ar_bits_addr;
    sensitive << i_msto_ar_bits_len;
    sensitive << i_msto_ar_bits_size;
    sensitive << i_msto_ar_bits_burst;
    sensitive << i_msto_ar_bits_lock;
    sensitive << i_msto_ar_bits_cache;
    sensitive << i_msto_ar_bits_prot;
    sensitive << i_msto_ar_bits_qos;
    sensitive << i_msto_ar_bits_region;
    sensitive << i_msto_ar_id;
    sensitive << i_msto_ar_user;
    sensitive << i_msto_r_ready;
    sensitive << i_halted;
    sensitive << r.req_addr;
    sensitive << r.req_len;
    sensitive << r.req_burst;
    sensitive << r.b_valid;
    sensitive << r.b_resp;
    sensitive << r.nrst;
    sensitive << r.interrupt;
    sensitive << r.state;
    sensitive << r.halted;
    sensitive << r.line;
    sensitive << r.r_error;
    sensitive << r.w_error;
    sensitive << w_resp_valid;
    sensitive << wb_resp_data;
    sensitive << w_r_error;
    sensitive << w_w_error;
    sensitive << w_dport_valid;
    sensitive << w_dport_write;
    sensitive << wb_dport_region;
    sensitive << wb_dport_addr;
    sensitive << wb_dport_wdata;

    SC_METHOD(registers);
    sensitive << o_clk.posedge_event();

    SC_METHOD(sys_bus_proc);
    sensitive << bus_event_;
}

RtlWrapper::~RtlWrapper() {
    RISCV_event_close(&dport_.valid);
}

void RtlWrapper::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, i_msto_aw_valid, i_msto_aw_valid.name());
        sc_trace(o_vcd, i_msto_aw_bits_addr, i_msto_aw_bits_addr.name());
        sc_trace(o_vcd, i_msto_aw_bits_len, i_msto_aw_bits_len.name());
        sc_trace(o_vcd, i_msto_aw_bits_size, i_msto_aw_bits_size.name());
        sc_trace(o_vcd, i_msto_aw_bits_burst, i_msto_aw_bits_burst.name());
        sc_trace(o_vcd, i_msto_aw_bits_lock, i_msto_aw_bits_lock.name());
        sc_trace(o_vcd, i_msto_aw_bits_cache, i_msto_aw_bits_cache.name());
        sc_trace(o_vcd, i_msto_aw_bits_prot, i_msto_aw_bits_prot.name());
        sc_trace(o_vcd, i_msto_aw_bits_qos, i_msto_aw_bits_qos.name());
        sc_trace(o_vcd, i_msto_aw_bits_region, i_msto_aw_bits_region.name());
        sc_trace(o_vcd, i_msto_aw_id, i_msto_aw_id.name());
        sc_trace(o_vcd, i_msto_aw_user, i_msto_aw_user.name());
        sc_trace(o_vcd, i_msto_w_valid, i_msto_w_valid.name());
        sc_trace(o_vcd, i_msto_w_data, i_msto_w_data.name());
        sc_trace(o_vcd, i_msto_w_last, i_msto_w_last.name());
        sc_trace(o_vcd, i_msto_w_strb, i_msto_w_strb.name());
        sc_trace(o_vcd, i_msto_w_user, i_msto_w_user.name());
        sc_trace(o_vcd, i_msto_b_ready, i_msto_b_ready.name());
        sc_trace(o_vcd, i_msto_ar_valid, i_msto_ar_valid.name());
        sc_trace(o_vcd, i_msto_ar_bits_addr, i_msto_ar_bits_addr.name());
        sc_trace(o_vcd, i_msto_ar_bits_len, i_msto_ar_bits_len.name());
        sc_trace(o_vcd, i_msto_ar_bits_size, i_msto_ar_bits_size.name());
        sc_trace(o_vcd, i_msto_ar_bits_burst, i_msto_ar_bits_burst.name());
        sc_trace(o_vcd, i_msto_ar_bits_lock, i_msto_ar_bits_lock.name());
        sc_trace(o_vcd, i_msto_ar_bits_cache, i_msto_ar_bits_cache.name());
        sc_trace(o_vcd, i_msto_ar_bits_prot, i_msto_ar_bits_prot.name());
        sc_trace(o_vcd, i_msto_ar_bits_qos, i_msto_ar_bits_qos.name());
        sc_trace(o_vcd, i_msto_ar_bits_region, i_msto_ar_bits_region.name());
        sc_trace(o_vcd, i_msto_ar_id, i_msto_ar_id.name());
        sc_trace(o_vcd, i_msto_ar_user, i_msto_ar_user.name());
        sc_trace(o_vcd, i_msto_r_ready, i_msto_r_ready.name());
        sc_trace(o_vcd, o_msti_aw_ready, o_msti_aw_ready.name());
        sc_trace(o_vcd, o_msti_w_ready, o_msti_w_ready.name());
        sc_trace(o_vcd, o_msti_b_valid, o_msti_b_valid.name());
        sc_trace(o_vcd, o_msti_b_resp, o_msti_b_resp.name());
        sc_trace(o_vcd, o_msti_b_id, o_msti_b_id.name());
        sc_trace(o_vcd, o_msti_b_user, o_msti_b_user.name());
        sc_trace(o_vcd, o_msti_ar_ready, o_msti_ar_ready.name());
        sc_trace(o_vcd, o_msti_r_valid, o_msti_r_valid.name());
        sc_trace(o_vcd, o_msti_r_resp, o_msti_r_resp.name());
        sc_trace(o_vcd, o_msti_r_data, o_msti_r_data.name());
        sc_trace(o_vcd, o_msti_r_last, o_msti_r_last.name());
        sc_trace(o_vcd, o_msti_r_id, o_msti_r_id.name());
        sc_trace(o_vcd, o_msti_r_user, o_msti_r_user.name());

        std::string pn(name());
        sc_trace(o_vcd, r.nrst, pn + ".r_nrst");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.req_burst, pn + ".r_req_burst");
        sc_trace(o_vcd, r.b_resp, pn + ".r_b_resp");
        sc_trace(o_vcd, r.b_valid, pn + ".r_b_valid");
        sc_trace(o_vcd, wb_wdata, pn + ".wb_wdata");
        sc_trace(o_vcd, wb_wstrb, pn + ".wb_wstrb");
    }
}

void RtlWrapper::clk_gen() {
    // todo: instead sc_clock
}

void RtlWrapper::comb() {
    bool w_req_mem_ready;
    sc_uint<BUS_ADDR_WIDTH> vb_req_addr;
    sc_biguint<DCACHE_LINE_BITS> vb_line_cached_o;
    sc_biguint<DCACHE_LINE_BITS> vb_line_uncached_o;
    sc_biguint<DCACHE_LINE_BITS> vb_line_o;
    sc_uint<4> vb_r_resp;
    bool v_r_valid;
    bool v_w_ready;
    sc_uint<BUS_DATA_WIDTH> vb_wdata;
    sc_uint<BUS_DATA_BYTES> vb_wstrb;
    int tmux = (DCACHE_BURST_LEN - 1) - r.req_len.read().to_int();

    w_req_mem_ready = 0;
    vb_r_resp = 0; // OKAY
    v_r_valid = 0;
    v_w_ready = 0;
    v.b_valid = 0;
    v.b_resp = 0;

    vb_line_cached_o = (wb_resp_data, r.line.read()(DCACHE_LINE_BITS-1, BUS_DATA_WIDTH));
    vb_line_uncached_o = (0, wb_resp_data);
    vb_line_o = 0;
    vb_wdata = 0;
    vb_wstrb = 0;

    v.interrupt = w_interrupt;
    v.halted = i_halted.read();

    v.nrst = (r.nrst.read() << 1) | 1;
    switch (r.state.read()) {
    case State_Idle:
        v.r_error = 0;
        v.w_error = 0;
        if (request_reset) {
            v.state = State_Reset;
        } else {
            w_req_mem_ready = 1;
        }
        break;
    case State_ReadUncached:
        v_r_valid = 1;
        vb_line_o = vb_line_uncached_o;
        w_req_mem_ready = 1;
        break;
    case State_ReadCached:
        vb_line_o = vb_line_cached_o;
        v.line = vb_line_cached_o;
        if (r.req_len.read() == 0) {
            v_r_valid = 1;
            w_req_mem_ready = 1;
        } else {
            v.r_error = r.r_error.read() || w_r_error;
            v.req_len = r.req_len.read() - 1;
            if (r.req_burst.read() == 0) {
                vb_req_addr = r.req_addr.read();
            } else if (r.req_burst.read() == 1) {
                vb_req_addr = r.req_addr.read() + 8;
            } else if (r.req_burst.read() == 2) {
                vb_req_addr(4, 0) = r.req_addr.read()(4, 0) + 8;
                vb_req_addr(31, 5) = r.req_addr.read()(31, 5);
            }
            v.req_addr = vb_req_addr;
        }
        break;
    case State_WriteUncached:
        v_w_ready = 1;
        vb_wdata = i_msto_w_data.read()(BUS_DATA_WIDTH-1, 0).to_uint64();
        vb_wstrb = i_msto_w_strb.read()(BUS_DATA_BYTES-1, 0);
        v.b_valid = 1;
        v.b_resp = w_w_error ? 0x2 : 0x0;
        w_req_mem_ready = 1;
        break;
    case State_WriteCached:
        vb_wdata = i_msto_w_data.read()((tmux+1)*BUS_DATA_WIDTH-1,
                                        tmux*BUS_DATA_WIDTH).to_uint64();
        vb_wstrb = (1u << BUS_DATA_BYTES) - 1;
        if (r.req_len.read() == 0) {
            v_w_ready = 1;
            v.b_valid = 1;
            v.b_resp = r.w_error.read() || w_w_error ? 0x2 : 0x0;
            w_req_mem_ready = 1;
        } else {
            v.w_error = r.w_error.read() || w_w_error;
            v.req_len = r.req_len.read() - 1;
            if (r.req_burst.read() == 0) {
                vb_req_addr = r.req_addr.read();
            } else if (r.req_burst.read() == 1) {
                vb_req_addr = r.req_addr.read() + 8;
            } else if (r.req_burst.read() == 2) {
                vb_req_addr(4, 0) = r.req_addr.read()(4, 0) + 8;
                vb_req_addr(31, 5) = r.req_addr.read()(31, 5);
            }
            v.req_addr = vb_req_addr;
        }
        break;
    case State_Reset:
        request_reset = false;
        v.nrst = (r.nrst.read() << 1);
        v.state = State_Idle;
        break;
    default:;
    }

    if (w_req_mem_ready == 1) {
        v.r_error = 0;
        v.w_error = 0;
        if (i_msto_ar_valid.read()) {
            v.req_addr = i_msto_ar_bits_addr.read();
            v.req_burst = 0x1;                                  // INCR
            if (i_msto_ar_bits_cache.read()[0] == 1) {          // cached:
                v.state = State_ReadCached;
                if (i_msto_ar_bits_prot.read()[2] == 1) {       // instruction
                    v.req_len = ICACHE_BURST_LEN - 1;
                } else {                                        // data
                    v.req_len = DCACHE_BURST_LEN - 1;
                }
            } else {                                            // uncached:
                v.state = State_ReadUncached;
                if (i_msto_ar_bits_prot.read()[2] == 1) {       // instruction
                    v.req_len = 1;
                } else {                                        // data
                    v.req_len = 0;
                }
            }
        } else if (i_msto_aw_valid.read()) {
            v.req_addr = i_msto_aw_bits_addr.read();
            v.req_burst = 0x1;                                  // INCR
            if (i_msto_aw_bits_cache.read()[0] == 1) {          // cached (data only):
                v.state = State_WriteCached;
                v.req_len = DCACHE_BURST_LEN - 1;
            } else {                                            // uncached (data only):
                v.state = State_WriteUncached;
                v.req_len = 0;
            }
        } else {
            v.state = State_Idle;
        }
    }

    if (r.r_error.read() || w_r_error.read()) {
        vb_r_resp = 0x2;    // SLVERR
    }

    wb_wdata = vb_wdata;
    wb_wstrb = vb_wstrb;

    o_nrst = r.nrst.read()[1].to_bool();

    o_msti_aw_ready = w_req_mem_ready;
    o_msti_w_ready = v_w_ready;
    o_msti_b_valid = r.b_valid;
    o_msti_b_resp = r.b_resp;
    o_msti_b_id = 0;
    o_msti_b_user = 0;

    o_msti_ar_ready = w_req_mem_ready;
    o_msti_r_valid = v_r_valid;
    o_msti_r_resp = vb_r_resp;                    // 0=OKAY;1=EXOKAY;2=SLVERR;3=DECER
    o_msti_r_data = vb_line_o;
    o_msti_r_last = v_r_valid;
    o_msti_r_id = 0;
    o_msti_r_user = 0;

    o_interrupt = r.interrupt;

    o_dport_valid = w_dport_valid;
    o_dport_write = w_dport_write;
    o_dport_region = wb_dport_region;
    o_dport_addr = wb_dport_addr;
    o_dport_wdata = wb_dport_wdata;

    if (!r.nrst.read()[1]) {
    }
}

void RtlWrapper::registers() {
    bus_event_.notify(1, SC_NS);
    r = v;
}

void RtlWrapper::sys_bus_proc() {
    /** Simulation events queue */
    IFace *cb;
    uint8_t strob;
    uint64_t offset;

    step_queue_.initProc();
    step_queue_.pushPreQueued();
    uint64_t step_cnt = i_time.read();
    while ((cb = step_queue_.getNext(step_cnt)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

    if (i_halted.read() && !r.halted.read()) {
        IService *iserv = static_cast<IService *>(iparent_);
        RISCV_trigger_hap(iserv, HAP_Halt, "Descr");
    }

    w_interrupt = async_interrupt;

    w_resp_valid = 0;
    wb_resp_data = 0;
    w_r_error = 0;
    w_w_error = 0;

    switch (r.state.read()) {
    case State_ReadUncached:
    case State_ReadCached:
        trans.action = MemAction_Read;
        trans.addr = r.req_addr.read();
        trans.xsize = 8;
        trans.wstrb = 0;
        trans.wpayload.b64[0] = 0;
        resp = ibus_->b_transport(&trans);

        w_resp_valid = 1;
        wb_resp_data = trans.rpayload.b64[0];
        if (resp == TRANS_ERROR) {
            w_r_error = 1;
        }
        break;
    case State_WriteUncached:
        trans.action = MemAction_Write;
        strob = static_cast<uint8_t>(wb_wstrb.read());
        offset = mask2offset(strob);
        trans.addr = r.req_addr.read();
        trans.addr += offset;
        trans.xsize = mask2size(strob >> offset);
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.wpayload.b64[0] = wb_wdata.read();
        resp = ibus_->b_transport(&trans);

        w_resp_valid = 1;
        if (resp == TRANS_ERROR) {
            w_w_error = 1;
        }
        break;
    case State_WriteCached:
        trans.action = MemAction_Write;
        trans.addr = r.req_addr.read();
        trans.xsize = BUS_DATA_BYTES;
        trans.wstrb = wb_wstrb.read().to_uint();
        trans.wpayload.b64[0] = wb_wdata.read();
        resp = ibus_->b_transport(&trans);

        w_resp_valid = 1;
        if (resp == TRANS_ERROR) {
            w_w_error = 1;
        }
        break;
    default:;
    }

    // Debug port handling:
    w_dport_valid = 0;
    if (RISCV_event_is_set(&dport_.valid)) {
        RISCV_event_clear(&dport_.valid);
        w_dport_valid = 1;
        w_dport_write = dport_.trans->write;
        wb_dport_region = dport_.trans->region;
        wb_dport_addr = dport_.trans->addr >> 3;
        wb_dport_wdata = dport_.trans->wdata;
    }
    dport_.idx_missmatch = 0;
    if (i_dport_ready.read()) {
        dport_.trans->rdata = i_dport_rdata.read().to_uint64();
        dport_.trans_idx_down++;
        if (dport_.trans_idx_down != dport_.trans_idx_up) {
            dport_.idx_missmatch = 1;
            RISCV_error("error: sync. is lost: up=%d, down=%d",
                         dport_.trans_idx_up, dport_.trans_idx_down);
            dport_.trans_idx_down = dport_.trans_idx_up;
        }
        dport_.cb->nb_response_debug_port(dport_.trans);
    }

}

uint64_t RtlWrapper::mask2offset(uint8_t mask) {
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (mask & 0x1) {
            return static_cast<uint64_t>(i);
        }
        mask >>= 1;
    }
    return 0;
}

uint32_t RtlWrapper::mask2size(uint8_t mask) {
    uint32_t bytes = 0;
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (!(mask & 0x1)) {
            break;
        }
        bytes++;
        mask >>= 1;
    }
    return bytes;
}

void RtlWrapper::setClockHz(double hz) {
    sc_time dt = sc_get_time_resolution();
    clockCycles_ = static_cast<int>((1.0 / hz) / dt.to_seconds() + 0.5);
}
    
void RtlWrapper::registerStepCallback(IClockListener *cb, uint64_t t) {
    if (request_reset) {
        if (i_time.read() == t) {
            cb->stepCallback(t);
        }
    } else {
        step_queue_.put(t, cb);
    }
}

void RtlWrapper::raiseSignal(int idx) {
    switch (idx) {
    case SIGNAL_HardReset:
        request_reset = true;
        break;
    case INTERRUPT_MExternal:
        async_interrupt = true;
        break;
    default:;
    }
}

void RtlWrapper::lowerSignal(int idx) {
    switch (idx) {
    case INTERRUPT_MExternal:
        async_interrupt = false;
        break;
    default:;
    }
}

void RtlWrapper::nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb) {
    dport_.trans = trans;
    dport_.cb = cb;
    dport_.trans_idx_up++;
    RISCV_event_set(&dport_.valid);
}

}  // namespace debugger

