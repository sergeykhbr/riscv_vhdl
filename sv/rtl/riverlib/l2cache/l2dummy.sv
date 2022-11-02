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

`timescale 1ns/10ps

module L2Dummy #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_river_pkg::axi4_l1_out_vector i_l1o,
    output types_river_pkg::axi4_l1_in_vector o_l1i,
    input types_river_pkg::axi4_l2_in_type i_l2i,
    output types_river_pkg::axi4_l2_out_type o_l2o,
    input logic i_flush_valid
);

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import l2dummy_pkg::*;

L2Dummy_registers r, rin;

always_comb
begin: comb_proc
    L2Dummy_registers v;
    axi4_l1_out_type vl1o[0: CFG_SLOT_L1_TOTAL-1];
    axi4_l1_in_type vlxi[0: CFG_SLOT_L1_TOTAL-1];
    axi4_l2_out_type vl2o;
    logic [CFG_SLOT_L1_TOTAL-1:0] vb_src_aw;
    logic [CFG_SLOT_L1_TOTAL-1:0] vb_src_ar;
    int vb_srcid;
    logic v_selected;

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        vl1o[i] = axi4_l1_out_none;
    end
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        vlxi[i] = axi4_l1_in_none;
    end
    vl2o = axi4_l2_out_none;
    vb_src_aw = 0;
    vb_src_ar = 0;
    vb_srcid = 0;
    v_selected = 0;

    v = r;

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        vl1o[i] = i_l1o[i];
        vlxi[i] = axi4_l1_in_none;

        vb_src_aw[i] = vl1o[i].aw_valid;
        vb_src_ar[i] = vl1o[i].ar_valid;
    end
    vl2o = axi4_l2_out_none;

    // select source (aw has higher priority):
    if ((|vb_src_aw) == 1'b0) begin
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            if ((v_selected == 1'b0) && (vb_src_ar[i] == 1'b1)) begin
                vb_srcid = i;
                v_selected = 1'b1;
            end
        end
    end else begin
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            if ((v_selected == 1'b0) && (vb_src_aw[i] == 1'b1)) begin
                vb_srcid = i;
                v_selected = 1'b1;
            end
        end
    end
    case (r.state)
    Idle: begin
        if ((|vb_src_aw) == 1'b1) begin
            v.state = state_aw;
            vlxi[vb_srcid].aw_ready = 1'h1;
            vlxi[vb_srcid].w_ready = 1'h1;                  // AXI-Lite-interface

            v.srcid = vb_srcid;
            v.req_addr = vl1o[vb_srcid].aw_bits.addr;
            v.req_size = vl1o[vb_srcid].aw_bits.size;
            v.req_lock = vl1o[vb_srcid].aw_bits.lock;
            v.req_prot = vl1o[vb_srcid].aw_bits.prot;
            v.req_id = vl1o[vb_srcid].aw_id;
            v.req_user = vl1o[vb_srcid].aw_user;
            // AXI-Lite-interface
            v.req_wdata = vl1o[vb_srcid].w_data;
            v.req_wstrb = vl1o[vb_srcid].w_strb;
        end else if ((|vb_src_ar) == 1'b1) begin
            v.state = state_ar;
            vlxi[vb_srcid].ar_ready = 1'h1;

            v.srcid = vb_srcid;
            v.req_addr = vl1o[vb_srcid].ar_bits.addr;
            v.req_size = vl1o[vb_srcid].ar_bits.size;
            v.req_lock = vl1o[vb_srcid].ar_bits.lock;
            v.req_prot = vl1o[vb_srcid].ar_bits.prot;
            v.req_id = vl1o[vb_srcid].ar_id;
            v.req_user = vl1o[vb_srcid].ar_user;
        end
    end
    state_ar: begin
        vl2o.ar_valid = 1'b1;
        vl2o.ar_bits.addr = r.req_addr;
        vl2o.ar_bits.size = r.req_size;
        vl2o.ar_bits.lock = r.req_lock;
        vl2o.ar_bits.prot = r.req_prot;
        vl2o.ar_id = r.req_id;
        vl2o.ar_user = r.req_user;
        if (i_l2i.ar_ready == 1'b1) begin
            v.state = state_r;
        end
    end
    state_r: begin
        vl2o.r_ready = 1'b1;
        if (i_l2i.r_valid == 1'b1) begin
            v.rdata = i_l2i.r_data;
            v.resp = i_l2i.r_resp;
            v.state = l1_r_resp;
        end
    end
    l1_r_resp: begin
        vlxi[int'(r.srcid)].r_valid = 1'h1;
        vlxi[int'(r.srcid)].r_last = 1'h1;
        vlxi[int'(r.srcid)].r_data = r.rdata;
        vlxi[int'(r.srcid)].r_resp = {2'h0, r.resp};
        vlxi[int'(r.srcid)].r_id = r.req_id;
        vlxi[int'(r.srcid)].r_user = r.req_user;
        if (vl1o[int'(r.srcid)].r_ready == 1'b1) begin
            v.state = Idle;
        end
    end
    state_aw: begin
        vl2o.aw_valid = 1'b1;
        vl2o.aw_bits.addr = r.req_addr;
        vl2o.aw_bits.size = r.req_size;
        vl2o.aw_bits.lock = r.req_lock;
        vl2o.aw_bits.prot = r.req_prot;
        vl2o.aw_id = r.req_id;
        vl2o.aw_user = r.req_user;
        vl2o.w_valid = 1'b1;                                // AXI-Lite request
        vl2o.w_last = 1'b1;
        vl2o.w_data = r.req_wdata;
        vl2o.w_strb = r.req_wstrb;
        vl2o.w_user = r.req_user;
        if (i_l2i.aw_ready == 1'b1) begin
            if (i_l2i.w_ready == 1'b1) begin
                v.state = state_b;
            end else begin
                v.state = state_w;
            end
        end
    end
    state_w: begin
        vl2o.w_valid = 1'b1;
        vl2o.w_last = 1'b1;
        vl2o.w_data = r.req_wdata;
        vl2o.w_strb = r.req_wstrb;
        vl2o.w_user = r.req_user;
        if (i_l2i.w_ready == 1'b1) begin
            v.state = state_b;
        end
    end
    state_b: begin
        vl2o.b_ready = 1'b1;
        if (i_l2i.b_valid == 1'b1) begin
            v.resp = i_l2i.b_resp;
            v.state = l1_w_resp;
        end
    end
    l1_w_resp: begin
        vlxi[int'(r.srcid)].b_valid = 1'h1;
        vlxi[int'(r.srcid)].b_resp = r.resp;
        vlxi[int'(r.srcid)].b_id = r.req_id;
        vlxi[int'(r.srcid)].b_user = r.req_user;
        if (vl1o[int'(r.srcid)].b_ready == 1'b1) begin
            v.state = Idle;
        end
    end
    default: begin
    end
    endcase

    if (~async_reset && i_nrst == 1'b0) begin
        v = L2Dummy_r_reset;
    end

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        o_l1i[i] = vlxi[i];
    end
    o_l2o = vl2o;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= L2Dummy_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: L2Dummy
