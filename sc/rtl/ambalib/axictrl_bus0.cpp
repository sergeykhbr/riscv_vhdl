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

#include "axictrl_bus0.h"
#include "api_core.h"

namespace debugger {

axictrl_bus0::axictrl_bus0(sc_module_name name,
                           bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_cfg("o_cfg"),
    i_xmsto("i_xmsto", CFG_BUS0_XMST_TOTAL),
    o_xmsti("o_xmsti", CFG_BUS0_XMST_TOTAL),
    i_xslvo("i_xslvo", CFG_BUS0_XSLV_TOTAL),
    o_xslvi("o_xslvi", CFG_BUS0_XSLV_TOTAL),
    o_mapinfo("o_mapinfo", CFG_BUS0_XSLV_TOTAL) {

    async_reset_ = async_reset;
    xdef0 = 0;

    xdef0 = new axi_slv("xdef0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_AXI_INTERCONNECT);
    xdef0->i_clk(i_clk);
    xdef0->i_nrst(i_nrst);
    xdef0->i_mapinfo(wb_def_mapinfo);
    xdef0->o_cfg(o_cfg);
    xdef0->i_xslvi(wb_def_xslvi);
    xdef0->o_xslvo(wb_def_xslvo);
    xdef0->o_req_valid(w_def_req_valid);
    xdef0->o_req_addr(wb_def_req_addr);
    xdef0->o_req_size(wb_def_req_size);
    xdef0->o_req_write(w_def_req_write);
    xdef0->o_req_wdata(wb_def_req_wdata);
    xdef0->o_req_wstrb(wb_def_req_wstrb);
    xdef0->o_req_last(w_def_req_last);
    xdef0->i_req_ready(w_def_req_ready);
    xdef0->i_resp_valid(w_def_resp_valid);
    xdef0->i_resp_rdata(wb_def_resp_rdata);
    xdef0->i_resp_err(w_def_resp_err);

    SC_METHOD(comb);
    sensitive << i_nrst;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        sensitive << i_xmsto[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << i_xslvo[i];
    }
    sensitive << wb_def_mapinfo;
    sensitive << wb_def_xslvi;
    sensitive << wb_def_xslvo;
    sensitive << w_def_req_valid;
    sensitive << wb_def_req_addr;
    sensitive << wb_def_req_size;
    sensitive << w_def_req_write;
    sensitive << wb_def_req_wdata;
    sensitive << wb_def_req_wstrb;
    sensitive << w_def_req_last;
    sensitive << w_def_req_ready;
    sensitive << w_def_resp_valid;
    sensitive << wb_def_resp_rdata;
    sensitive << w_def_resp_err;
    sensitive << r.r_midx;
    sensitive << r.r_sidx;
    sensitive << r.w_midx;
    sensitive << r.w_sidx;
    sensitive << r.b_midx;
    sensitive << r.b_sidx;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

axictrl_bus0::~axictrl_bus0() {
    if (xdef0) {
        delete xdef0;
    }
}

void axictrl_bus0::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, wb_def_xslvi, wb_def_xslvi.name());
        sc_trace(o_vcd, wb_def_xslvo, wb_def_xslvo.name());
        sc_trace(o_vcd, r.r_midx, pn + ".r_r_midx");
        sc_trace(o_vcd, r.r_sidx, pn + ".r_r_sidx");
        sc_trace(o_vcd, r.w_midx, pn + ".r_w_midx");
        sc_trace(o_vcd, r.w_sidx, pn + ".r_w_sidx");
        sc_trace(o_vcd, r.b_midx, pn + ".r_b_midx");
        sc_trace(o_vcd, r.b_sidx, pn + ".r_b_sidx");
    }

    if (xdef0) {
        xdef0->generateVCD(i_vcd, o_vcd);
    }
}

void axictrl_bus0::comb() {
    axi4_master_in_type vmsti[(CFG_BUS0_XMST_TOTAL + 1)];
    axi4_master_out_type vmsto[(CFG_BUS0_XMST_TOTAL + 1)];
    axi4_slave_in_type vslvi[(CFG_BUS0_XSLV_TOTAL + 1)];
    axi4_slave_out_type vslvo[(CFG_BUS0_XSLV_TOTAL + 1)];
    mapinfo_type vb_def_mapinfo;
    int i_ar_midx;
    int i_aw_midx;
    int i_ar_sidx;
    int i_aw_sidx;
    int i_r_midx;
    int i_r_sidx;
    int i_w_midx;
    int i_w_sidx;
    int i_b_midx;
    int i_b_sidx;
    bool v_aw_fire;
    bool v_ar_fire;
    bool v_w_fire;
    bool v_w_busy;
    bool v_r_fire;
    bool v_r_busy;
    bool v_b_fire;
    bool v_b_busy;

    for (int i = 0; i < (CFG_BUS0_XMST_TOTAL + 1); i++) {
        vmsti[i] = axi4_master_in_none;
    }
    for (int i = 0; i < (CFG_BUS0_XMST_TOTAL + 1); i++) {
        vmsto[i] = axi4_master_out_none;
    }
    for (int i = 0; i < (CFG_BUS0_XSLV_TOTAL + 1); i++) {
        vslvi[i] = axi4_slave_in_none;
    }
    for (int i = 0; i < (CFG_BUS0_XSLV_TOTAL + 1); i++) {
        vslvo[i] = axi4_slave_out_none;
    }
    vb_def_mapinfo = mapinfo_none;
    i_ar_midx = 0;
    i_aw_midx = 0;
    i_ar_sidx = 0;
    i_aw_sidx = 0;
    i_r_midx = 0;
    i_r_sidx = 0;
    i_w_midx = 0;
    i_w_sidx = 0;
    i_b_midx = 0;
    i_b_sidx = 0;
    v_aw_fire = 0;
    v_ar_fire = 0;
    v_w_fire = 0;
    v_w_busy = 0;
    v_r_fire = 0;
    v_r_busy = 0;
    v_b_fire = 0;
    v_b_busy = 0;

    v = r;

    vb_def_mapinfo.addr_start = 0;
    vb_def_mapinfo.addr_end = 0;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        vmsto[i] = i_xmsto[i];                              // Cannot read vector item from port in systemc
        vmsti[i] = axi4_master_in_none;
    }
    // Unmapped default slots:
    vmsto[CFG_BUS0_XMST_TOTAL] = axi4_master_out_none;
    vmsti[CFG_BUS0_XMST_TOTAL] = axi4_master_in_none;

    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        vslvo[i] = i_xslvo[i];                              // Cannot read vector item from port in systemc
        vslvi[i] = axi4_slave_in_none;
    }
    // Unmapped default slots:
    vslvo[CFG_BUS0_XSLV_TOTAL] = wb_def_xslvo;
    vslvi[CFG_BUS0_XSLV_TOTAL] = axi4_slave_in_none;

    w_def_req_ready = 1;
    w_def_resp_valid = 1;
    wb_def_resp_rdata = ~0ull;
    w_def_resp_err = 1;
    i_ar_midx = CFG_BUS0_XMST_TOTAL;
    i_aw_midx = CFG_BUS0_XMST_TOTAL;
    i_ar_sidx = CFG_BUS0_XSLV_TOTAL;
    i_aw_sidx = CFG_BUS0_XSLV_TOTAL;
    i_r_midx = r.r_midx.read().to_int();
    i_r_sidx = r.r_sidx.read().to_int();
    i_w_midx = r.w_midx.read().to_int();
    i_w_sidx = r.w_sidx.read().to_int();
    i_b_midx = r.b_midx.read().to_int();
    i_b_sidx = r.b_sidx.read().to_int();

    // Select Master bus:
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        if (vmsto[i].ar_valid == 1) {
            i_ar_midx = i;
        }
        if (vmsto[i].aw_valid == 1) {
            i_aw_midx = i;
        }
    }

    // Select Slave interface:
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        if (((CFG_BUS0_MAP[i].addr_start >> 12) <= vmsto[i_ar_midx].ar_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12))
                && (vmsto[i_ar_midx].ar_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12) < (CFG_BUS0_MAP[i].addr_end >> 12))) {
            i_ar_sidx = i;
        }
        if (((CFG_BUS0_MAP[i].addr_start >> 12) <= vmsto[i_aw_midx].aw_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12))
                && (vmsto[i_aw_midx].aw_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12) < (CFG_BUS0_MAP[i].addr_end >> 12))) {
            i_aw_sidx = i;
        }
    }

    // Read Channel:
    v_ar_fire = (vmsto[i_ar_midx].ar_valid & vslvo[i_ar_sidx].ar_ready);
    v_r_fire = (vmsto[i_r_midx].r_ready & vslvo[i_r_sidx].r_valid & vslvo[i_r_sidx].r_last);
    // Write channel:
    v_aw_fire = (vmsto[i_aw_midx].aw_valid & vslvo[i_aw_sidx].aw_ready);
    v_w_fire = (vmsto[i_w_midx].w_valid & vmsto[i_w_midx].w_last & vslvo[i_w_sidx].w_ready);
    // Write confirm channel
    v_b_fire = (vmsto[i_b_midx].b_ready & vslvo[i_b_sidx].b_valid);

    if ((r.r_sidx.read() != CFG_BUS0_XSLV_TOTAL) && (v_r_fire == 0)) {
        v_r_busy = 1;
    }

    if (((r.w_sidx.read() != CFG_BUS0_XSLV_TOTAL) && (v_w_fire == 0))
            || ((r.b_sidx.read() != CFG_BUS0_XSLV_TOTAL) && (v_b_fire == 0))) {
        v_w_busy = 1;
    }

    if ((r.b_sidx.read() != CFG_BUS0_XSLV_TOTAL) && (v_b_fire == 0)) {
        v_b_busy = 1;
    }

    if ((v_ar_fire == 1) && (v_r_busy == 0)) {
        v.r_sidx = i_ar_sidx;
        v.r_midx = i_ar_midx;
    } else if (v_r_fire == 1) {
        v.r_sidx = CFG_BUS0_XSLV_TOTAL;
        v.r_midx = CFG_BUS0_XMST_TOTAL;
    }

    if ((v_aw_fire == 1) && (v_w_busy == 0)) {
        v.w_sidx = i_aw_sidx;
        v.w_midx = i_aw_midx;
    } else if ((v_w_fire == 1) && (v_b_busy == 0)) {
        v.w_sidx = CFG_BUS0_XSLV_TOTAL;
        v.w_midx = CFG_BUS0_XMST_TOTAL;
    }

    if ((v_w_fire == 1) && (v_b_busy == 0)) {
        v.b_sidx = r.w_sidx;
        v.b_midx = r.w_midx;
    } else if (v_b_fire == 1) {
        v.b_sidx = CFG_BUS0_XSLV_TOTAL;
        v.b_midx = CFG_BUS0_XMST_TOTAL;
    }

    vmsti[i_ar_midx].ar_ready = (vslvo[i_ar_sidx].ar_ready & (!v_r_busy));
    vslvi[i_ar_sidx].ar_valid = (vmsto[i_ar_midx].ar_valid & (!v_r_busy));
    vslvi[i_ar_sidx].ar_bits = vmsto[i_ar_midx].ar_bits;
    vslvi[i_ar_sidx].ar_id = vmsto[i_ar_midx].ar_id;
    vslvi[i_ar_sidx].ar_user = vmsto[i_ar_midx].ar_user;

    vmsti[i_r_midx].r_valid = vslvo[i_r_sidx].r_valid;
    vmsti[i_r_midx].r_resp = vslvo[i_r_sidx].r_resp;
    vmsti[i_r_midx].r_data = vslvo[i_r_sidx].r_data;
    vmsti[i_r_midx].r_last = vslvo[i_r_sidx].r_last;
    vmsti[i_r_midx].r_id = vslvo[i_r_sidx].r_id;
    vmsti[i_r_midx].r_user = vslvo[i_r_sidx].r_user;
    vslvi[i_r_sidx].r_ready = vmsto[i_r_midx].r_ready;

    vmsti[i_aw_midx].aw_ready = (vslvo[i_aw_sidx].aw_ready & (!v_w_busy));
    vslvi[i_aw_sidx].aw_valid = (vmsto[i_aw_midx].aw_valid & (!v_w_busy));
    vslvi[i_aw_sidx].aw_bits = vmsto[i_aw_midx].aw_bits;
    vslvi[i_aw_sidx].aw_id = vmsto[i_aw_midx].aw_id;
    vslvi[i_aw_sidx].aw_user = vmsto[i_aw_midx].aw_user;

    vmsti[i_w_midx].w_ready = (vslvo[i_w_sidx].w_ready & (!v_b_busy));
    vslvi[i_w_sidx].w_valid = (vmsto[i_w_midx].w_valid & (!v_b_busy));
    vslvi[i_w_sidx].w_data = vmsto[i_w_midx].w_data;
    vslvi[i_w_sidx].w_last = vmsto[i_w_midx].w_last;
    vslvi[i_w_sidx].w_strb = vmsto[i_w_midx].w_strb;
    vslvi[i_w_sidx].w_user = vmsto[i_w_midx].w_user;

    vmsti[i_b_midx].b_valid = vslvo[i_b_sidx].b_valid;
    vmsti[i_b_midx].b_resp = vslvo[i_b_sidx].b_resp;
    vmsti[i_b_midx].b_id = vslvo[i_b_sidx].b_id;
    vmsti[i_b_midx].b_user = vslvo[i_b_sidx].b_user;
    vslvi[i_b_sidx].b_ready = vmsto[i_b_midx].b_ready;

    if (!async_reset_ && i_nrst.read() == 0) {
        axictrl_bus0_r_reset(v);
    }

    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        o_xmsti[i] = vmsti[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        o_xslvi[i] = vslvi[i];
        o_mapinfo[i] = CFG_BUS0_MAP[i];
    }
    wb_def_xslvi = vslvi[CFG_BUS0_XSLV_TOTAL];
    wb_def_mapinfo = vb_def_mapinfo;
}

void axictrl_bus0::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        axictrl_bus0_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

