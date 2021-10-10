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

#include "river_amba.h"

namespace debugger {

sc_uint<REQ_MEM_TYPE_BITS> ReadNoSnoop() {
    sc_uint<REQ_MEM_TYPE_BITS> ret = 0x0;
    return ret;
}

sc_uint<REQ_MEM_TYPE_BITS> ReadShared() {
    sc_uint<REQ_MEM_TYPE_BITS> ret = 0x0;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    return ret;
}

sc_uint<REQ_MEM_TYPE_BITS> ReadMakeUnique() {
    sc_uint<REQ_MEM_TYPE_BITS> ret = 0x0;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    ret[REQ_MEM_TYPE_UNIQUE] = 1;
    return ret;
}

sc_uint<REQ_MEM_TYPE_BITS> WriteNoSnoop() {
    sc_uint<REQ_MEM_TYPE_BITS> ret = 0x0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    return ret;
}

sc_uint<REQ_MEM_TYPE_BITS> WriteLineUnique() {
    sc_uint<REQ_MEM_TYPE_BITS> ret = 0x0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    ret[REQ_MEM_TYPE_UNIQUE] = 1;
    return ret;
}

sc_uint<REQ_MEM_TYPE_BITS> WriteBack() {
    sc_uint<REQ_MEM_TYPE_BITS> ret = 0x0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    return ret;
}


RiverAmba::RiverAmba(sc_module_name name_, uint32_t hartid, bool async_reset,
    bool fpu_ena, bool coherence_ena, bool tracer_ena) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_msti("i_msti"),
    o_msto("o_msto"),
    i_ext_irq("i_ext_irq"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_write("i_dport_write"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_rdata("o_dport_rdata"),
    o_halted("o_halted") {
    async_reset_ = async_reset;
    coherence_ena_ = coherence_ena;

    river0 = new RiverTop("river0", hartid, async_reset, fpu_ena,
                                    coherence_ena, tracer_ena);
    river0->i_clk(i_clk);
    river0->i_nrst(i_nrst);
    river0->i_req_mem_ready(req_mem_ready_i);
    river0->o_req_mem_path(req_mem_path_o);
    river0->o_req_mem_valid(req_mem_valid_o);
    river0->o_req_mem_type(req_mem_type_o);
    river0->o_req_mem_size(req_mem_size_o);
    river0->o_req_mem_addr(req_mem_addr_o);
    river0->o_req_mem_strob(req_mem_strob_o);
    river0->o_req_mem_data(req_mem_data_o);
    river0->i_resp_mem_valid(resp_mem_valid_i);
    river0->i_resp_mem_path(r.req_path);
    river0->i_resp_mem_data(resp_mem_data_i);
    river0->i_resp_mem_load_fault(resp_mem_load_fault_i);
    river0->i_resp_mem_store_fault(resp_mem_store_fault_i);
    river0->i_req_snoop_valid(req_snoop_valid_i);
    river0->i_req_snoop_type(req_snoop_type_i);
    river0->o_req_snoop_ready(req_snoop_ready_o);
    river0->i_req_snoop_addr(req_snoop_addr_i);
    river0->i_resp_snoop_ready(resp_snoop_ready_i);
    river0->o_resp_snoop_valid(resp_snoop_valid_o);
    river0->o_resp_snoop_data(resp_snoop_data_o);
    river0->o_resp_snoop_flags(resp_snoop_flags_o);
    river0->i_tmr_irq(i_tmr_irq);
    river0->i_ext_irq(i_ext_irq);
    river0->i_haltreq(i_haltreq);
    river0->i_resumereq(i_resumereq);
    river0->i_step(i_step);
    river0->i_dport_req_valid(i_dport_req_valid);
    river0->i_dport_write(i_dport_write);
    river0->i_dport_addr(i_dport_addr);
    river0->i_dport_wdata(i_dport_wdata);
    river0->o_dport_req_ready(o_dport_req_ready);
    river0->i_dport_resp_ready(i_dport_resp_ready);
    river0->o_dport_resp_valid(o_dport_resp_valid);
    river0->o_dport_rdata(o_dport_rdata);
    river0->o_halted(o_halted);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_msti;
    sensitive << req_mem_path_o;
    sensitive << req_mem_valid_o;
    sensitive << req_mem_type_o;
    sensitive << req_mem_addr_o;
    sensitive << req_mem_strob_o;
    sensitive << req_mem_data_o;
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
    sensitive << w_ac_ready;
    sensitive << w_cr_valid;
    sensitive << wb_cr_resp;
    sensitive << w_cd_valid;
    sensitive << wb_cd_data;
    sensitive << w_cd_last;
    sensitive << w_rack;
    sensitive << w_wack;

    if (coherence_ena) {
        SC_METHOD(snoopcomb);
        sensitive << i_nrst;
        sensitive << i_msti;
        sensitive << req_snoop_ready_o;
        sensitive << resp_snoop_valid_o;
        sensitive << resp_snoop_data_o;
        sensitive << resp_snoop_flags_o;
        sensitive << sr.snoop_state;
        sensitive << sr.ac_addr;
        sensitive << sr.ac_snoop;
        sensitive << sr.cr_resp;
        sensitive << sr.req_snoop_type;
        sensitive << sr.resp_snoop_data;
        sensitive << sr.cache_access;
    } else {
        w_ac_ready = 1;
        w_cr_valid = 1;
        wb_cr_resp = 0;
        w_cd_valid = 0;
        wb_cd_data = 0;
        w_cd_last = 0;
        w_rack = 0;
        w_wack = 0;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

RiverAmba::~RiverAmba() {
    delete river0;
}

void RiverAmba::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_halted, o_halted.name());
        sc_trace(o_vcd, o_dport_req_ready, o_dport_req_ready.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());

        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());

        std::string pn(name());
        sc_trace(o_vcd, req_mem_path_o, pn + ".req_mem_path_o");
        sc_trace(o_vcd, req_mem_valid_o, pn + ".req_mem_valid_o");
        sc_trace(o_vcd, req_mem_type_o, pn + ".req_mem_type_o");
        sc_trace(o_vcd, req_mem_addr_o, pn + ".req_mem_addr_o");
        sc_trace(o_vcd, req_mem_strob_o, pn + ".req_mem_strob_o");
        sc_trace(o_vcd, req_mem_data_o, pn + ".req_mem_data_o");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        
    }

    river0->generateVCD(i_vcd, o_vcd);
}

void RiverAmba::comb() {
    bool v_resp_mem_valid;
    bool v_mem_er_load_fault;
    bool v_mem_er_store_fault;
    bool v_next_ready;
    axi4_l1_out_type vmsto;

    v = r;
    
    v_resp_mem_valid = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = 0;
    v_next_ready = 0;

    vmsto = axi4_l1_out_none;
    vmsto.ar_bits.burst = 0x1;  // INCR (possible any value actually)
    vmsto.aw_bits.burst = 0x1;  // INCR (possible any value actually)

    switch (r.state.read()) {
    case state_idle:
        v_next_ready = 1;
        if (req_mem_valid_o.read() == 1) {
            v.req_path = req_mem_path_o.read();
            v.req_addr = req_mem_addr_o;
            v.req_size = req_mem_size_o;
            // [0] 0=Unpriv/1=Priv;
            // [1] 0=Secure/1=Non-secure;
            // [2]  0=Data/1=Instruction
            v.req_prot = req_mem_path_o.read() << 2;
            if (req_mem_type_o.read()[REQ_MEM_TYPE_WRITE] == 0) {
                v.state = state_ar;
                v.req_wdata = 0;
                v.req_wstrb = 0;
                if (req_mem_type_o.read()[REQ_MEM_TYPE_CACHED] == 1) {
                    v.req_cached = ARCACHE_WRBACK_READ_ALLOCATE;
                } else {
                    v.req_cached = ARCACHE_DEVICE_NON_BUFFERABLE;
                }
                if (coherence_ena_) {
                    v.req_ar_snoop = reqtype2arsnoop(req_mem_type_o.read());
                }
            } else {
                v.state = state_aw;
                v.req_wdata = req_mem_data_o;
                v.req_wstrb = req_mem_strob_o;
                if (req_mem_type_o.read()[REQ_MEM_TYPE_CACHED] == 1) {
                    v.req_cached = AWCACHE_WRBACK_WRITE_ALLOCATE;
                } else {
                    v.req_cached = AWCACHE_DEVICE_NON_BUFFERABLE;
                }
                if (coherence_ena_) {
                    v.req_aw_snoop = reqtype2awsnoop(req_mem_type_o.read());
                }
            }
        }
        break;

    case state_ar:
        vmsto.ar_valid = 1;
        vmsto.ar_bits.addr = r.req_addr.read();
        vmsto.ar_bits.cache = r.req_cached;
        vmsto.ar_bits.size = r.req_size;
        vmsto.ar_bits.prot = r.req_prot;
        vmsto.ar_snoop = r.req_ar_snoop;
        if (i_msti.read().ar_ready == 1) {
            v.state = state_r;
        }
        break;
    case state_r:
        vmsto.r_ready = 1;
        v_mem_er_load_fault = i_msti.read().r_resp[1];
        v_resp_mem_valid = i_msti.read().r_valid;
        // r_valid and r_last always should be in the same time
        if (i_msti.read().r_valid == 1 && i_msti.read().r_last == 1) {
            v.state = state_idle;
        }
        break;

    case state_aw:
        vmsto.aw_valid = 1;
        vmsto.aw_bits.addr = r.req_addr.read();
        vmsto.aw_bits.cache = r.req_cached;
        vmsto.aw_bits.size = r.req_size;
        vmsto.aw_bits.prot = r.req_prot;
        vmsto.aw_snoop = r.req_aw_snoop;
        // axi lite to simplify L2-cache
        vmsto.w_valid = 1;
        vmsto.w_last = 1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
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
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
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
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    vmsto.ac_ready = w_ac_ready;
    vmsto.cr_valid = w_cr_valid;
    vmsto.cr_resp = wb_cr_resp;
    vmsto.cd_valid = w_cd_valid;
    vmsto.cd_data = wb_cd_data;
    vmsto.cd_last = w_cd_last;
    vmsto.rack = w_rack;
    vmsto.wack = w_wack;

    o_msto = vmsto;

    req_mem_ready_i = v_next_ready;  
    resp_mem_valid_i = v_resp_mem_valid;
    resp_mem_data_i = i_msti.read().r_data;
    resp_mem_load_fault_i = v_mem_er_load_fault;
    resp_mem_store_fault_i = v_mem_er_store_fault;
}

void RiverAmba::snoopcomb() {
    bool v_next_ready;
    bool req_snoop_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_req_snoop_addr;
    sc_uint<SNOOP_REQ_TYPE_BITS> vb_req_snoop_type;
    bool v_cr_valid;
    sc_uint<5> vb_cr_resp;
    bool v_cd_valid;
    sc_biguint<L1CACHE_LINE_BITS> vb_cd_data;

    sv = sr;

    v_next_ready = 0;
    req_snoop_valid = 0;
    vb_req_snoop_addr = 0;
    vb_req_snoop_type = 0;
    v_cr_valid = 0;
    vb_cr_resp = 0;
    v_cd_valid = 0;
    vb_cd_data = 0;

    switch (sr.snoop_state.read()) {
    case snoop_idle:
        v_next_ready = 1;
        break;
    case snoop_ac_wait_accept:
        req_snoop_valid = 1;
        vb_req_snoop_addr = sr.ac_addr;
        vb_req_snoop_type = sr.req_snoop_type;
        if (req_snoop_ready_o.read() == 1) {
            if (sr.cache_access == 0) {
                sv.snoop_state = snoop_cr;
            } else {
                sv.snoop_state = snoop_cd;
            }
        }
        break;
    case snoop_cr:
        if (resp_snoop_valid_o.read() == 1) {
            v_cr_valid = 1;
            if (resp_snoop_flags_o.read()[TAG_FL_VALID] == 1
                    && (resp_snoop_flags_o.read()[DTAG_FL_SHARED] == 0
                        || sr.ac_snoop.read() == AC_SNOOP_READ_UNIQUE)) {
                // Need second request with cache access
                sv.cache_access = 1;
                // see table C3-21 "Snoop response bit allocation"
                vb_cr_resp[0] = 1;          // will be Data transfer
                vb_cr_resp[4] = 1;          // WasUnique
                if (sr.ac_snoop.read() == AC_SNOOP_READ_UNIQUE) {
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READCLEAN] = 1;
                } else {
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READDATA] = 1;
                }
                sv.req_snoop_type = vb_req_snoop_type;
                sv.snoop_state = snoop_ac_wait_accept;
                if (i_msti.read().cr_ready == 1) {
                    sv.snoop_state = snoop_ac_wait_accept;
                } else {
                    sv.snoop_state = snoop_cr_wait_accept;
                }
            } else {
                vb_cr_resp = 0;
                if (i_msti.read().cr_ready == 1) {
                    sv.snoop_state = snoop_idle;
                } else {
                    sv.snoop_state = snoop_cr_wait_accept;
                }
            }
            sv.cr_resp = vb_cr_resp;
        }
        break;
    case snoop_cr_wait_accept:
        v_cr_valid = 1;
        vb_cr_resp = sr.cr_resp;
        if (i_msti.read().cr_ready == 1) {
            if (sr.cache_access == 1) {
                sv.snoop_state = snoop_ac_wait_accept;
            } else {
                sv.snoop_state = snoop_idle;
            }
        }
        break;
    case snoop_cd:
        if (resp_snoop_valid_o.read() == 1) {
            v_cd_valid = 1;
            vb_cd_data = resp_snoop_data_o;
            sv.resp_snoop_data = resp_snoop_data_o;
            if (i_msti.read().cd_ready == 1) {
                sv.snoop_state = snoop_idle;
            } else {
                sv.snoop_state = snoop_cd_wait_accept;
            }
        }
        break;
    case snoop_cd_wait_accept:
        v_cd_valid = 1;
        vb_cd_data = sr.resp_snoop_data;
        if (i_msti.read().cd_ready == 1) {
            sv.snoop_state = snoop_idle;
        }
        break;
    default:;
    }

    if (v_next_ready == 1) {
        if (i_msti.read().ac_valid == 1) {
            req_snoop_valid = 1;
            sv.cache_access = 0;
            vb_req_snoop_type = 0;              // First snoop operation always just to read flags
            sv.req_snoop_type = 0;
            vb_req_snoop_addr = i_msti.read().ac_addr;
            sv.ac_addr = i_msti.read().ac_addr;
            sv.ac_snoop = i_msti.read().ac_snoop;
            if (req_snoop_ready_o.read() == 1) {
                sv.snoop_state = snoop_cr;
            } else {
                sv.snoop_state = snoop_ac_wait_accept;
            }
        }
    }

    if (!async_reset_ && !i_nrst.read()) {
        SR_RESET(sv);
    }

    req_snoop_valid_i = req_snoop_valid;
    req_snoop_type_i = vb_req_snoop_type;
    req_snoop_addr_i = vb_req_snoop_addr;
    resp_snoop_ready_i = 1;

    w_ac_ready = v_next_ready;
    w_cr_valid = v_cr_valid;
    wb_cr_resp = vb_cr_resp;
    w_cd_valid = v_cd_valid;
    wb_cd_data = vb_cd_data;
    w_cd_last = v_cd_valid;
    w_rack = 0;
    w_wack = 0;
}

void RiverAmba::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
        SR_RESET(sr);
    } else {
        r = v;
        sr = sv;
    }
}

}  // namespace debugger

