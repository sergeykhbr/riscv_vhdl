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

module L2Destination #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_resp_valid,
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_resp_rdata,
    input logic [1:0] i_resp_status,
    input types_river_pkg::axi4_l1_out_vector i_l1o,
    output types_river_pkg::axi4_l1_in_vector o_l1i,
    // cache interface
    input logic i_req_ready,
    output logic o_req_valid,
    output logic [river_cfg_pkg::L2_REQ_TYPE_BITS-1:0] o_req_type,
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_req_addr,
    output logic [2:0] o_req_size,
    output logic [2:0] o_req_prot,
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_req_wdata,
    output logic [river_cfg_pkg::L1CACHE_BYTES_PER_LINE-1:0] o_req_wstrb
);

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;
import l2_dst_pkg::*;

L2Destination_registers r, rin;

always_comb
begin: comb_proc
    L2Destination_registers v;
    axi4_l1_out_type vcoreo[0: (CFG_SLOT_L1_TOTAL + 1)-1];
    axi4_l1_in_type vlxi[0: CFG_SLOT_L1_TOTAL-1];
    logic [CFG_SLOT_L1_TOTAL-1:0] vb_src_aw;
    logic [CFG_SLOT_L1_TOTAL-1:0] vb_src_ar;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] vb_broadband_mask_full;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] vb_broadband_mask;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] vb_ac_valid;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] vb_cr_ready;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] vb_cd_ready;
    logic [2:0] vb_srcid;
    logic v_req_valid;
    logic [L2_REQ_TYPE_BITS-1:0] vb_req_type;

    for (int i = 0; i < (CFG_SLOT_L1_TOTAL + 1); i++) begin
        vcoreo[i] = axi4_l1_out_none;
    end
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        vlxi[i] = axi4_l1_in_none;
    end
    vb_src_aw = 0;
    vb_src_ar = 0;
    vb_broadband_mask_full = 0;
    vb_broadband_mask = 0;
    vb_ac_valid = 0;
    vb_cr_ready = 0;
    vb_cd_ready = 0;
    vb_srcid = 0;
    v_req_valid = 0;
    vb_req_type = 0;

    v = r;

    vb_req_type = r.req_type;

    vb_srcid = CFG_SLOT_L1_TOTAL;
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        vcoreo[i] = i_l1o[i];                               // Cannot read vector item from port in systemc
        vlxi[i] = axi4_l1_in_none;

        vb_src_aw[i] = vcoreo[i].aw_valid;
        vb_src_ar[i] = vcoreo[i].ar_valid;
    end
    vcoreo[CFG_SLOT_L1_TOTAL] = axi4_l1_out_none;

    // select source (aw has higher priority):
    if ((|vb_src_aw) == 1'b0) begin
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            if ((vb_srcid == CFG_SLOT_L1_TOTAL) && (vb_src_ar[i] == 1'b1)) begin
                vb_srcid = i;
            end
        end
    end else begin
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            if ((vb_srcid == CFG_SLOT_L1_TOTAL) && (vb_src_aw[i] == 1'b1)) begin
                vb_srcid = i;
            end
        end
    end

    vb_ac_valid = r.ac_valid;
    vb_cr_ready = r.cr_ready;
    vb_cd_ready = r.cd_ready;

    vb_broadband_mask_full = '1;
    vb_broadband_mask_full[CFG_SLOT_L1_TOTAL] = 1'b0;       // exclude empty slot
    vb_broadband_mask = vb_broadband_mask_full;
    vb_broadband_mask[int'(vb_srcid)] = 1'b0;               // exclude source

    case (r.state)
    Idle: begin
        vb_req_type = '0;
        if ((|vb_src_aw) == 1'b1) begin
            v.state = CacheWriteReq;
            vlxi[int'(vb_srcid)].aw_ready = 1'h1;
            vlxi[int'(vb_srcid)].w_ready = 1'h1;            // Lite-interface

            v.srcid = vb_srcid;
            v.req_addr = vcoreo[int'(vb_srcid)].aw_bits.addr;
            v.req_size = vcoreo[int'(vb_srcid)].aw_bits.size;
            v.req_prot = vcoreo[int'(vb_srcid)].aw_bits.prot;
            vb_req_type[L2_REQ_TYPE_WRITE] = 1'b1;
            if (vcoreo[int'(vb_srcid)].aw_bits.cache[0] == 1'b1) begin
                vb_req_type[L2_REQ_TYPE_CACHED] = 1'b1;
                if (vcoreo[int'(vb_srcid)].aw_snoop == AWSNOOP_WRITE_LINE_UNIQUE) begin
                    vb_req_type[L2_REQ_TYPE_UNIQUE] = 1'b1;
                    v.ac_valid = vb_broadband_mask;
                    v.cr_ready = '0;
                    v.cd_ready = '0;
                    v.state = snoop_ac;
                end
            end
        end else if ((|vb_src_ar) == 1'b1) begin
            v.state = CacheReadReq;
            vlxi[int'(vb_srcid)].ar_ready = 1'h1;

            v.srcid = vb_srcid;
            v.req_addr = vcoreo[int'(vb_srcid)].ar_bits.addr;
            v.req_size = vcoreo[int'(vb_srcid)].ar_bits.size;
            v.req_prot = vcoreo[int'(vb_srcid)].ar_bits.prot;
            if (vcoreo[int'(vb_srcid)].ar_bits.cache[0] == 1'b1) begin
                vb_req_type[L2_REQ_TYPE_CACHED] = 1'b1;
                if (vcoreo[int'(vb_srcid)].ar_snoop == ARSNOOP_READ_MAKE_UNIQUE) begin
                    vb_req_type[L2_REQ_TYPE_UNIQUE] = 1'b1;
                end
                // prot[2]: 0=Data, 1=Instr.
                // If source is I$ then request D$ of the same CPU
                if (vcoreo[int'(vb_srcid)].ar_bits.prot[2] == 1'b1) begin
                    v.ac_valid = vb_broadband_mask_full;
                end else begin
                    v.ac_valid = vb_broadband_mask;
                end
                v.cr_ready = '0;
                v.cd_ready = '0;
                v.state = snoop_ac;
            end
        end
        v.req_type = vb_req_type;
        // Lite-interface
        v.req_wdata = vcoreo[int'(vb_srcid)].w_data;
        v.req_wstrb = vcoreo[int'(vb_srcid)].w_strb;
    end
    CacheReadReq: begin
        v_req_valid = 1'b1;
        if (i_req_ready == 1'b1) begin
            v.state = ReadMem;
        end
    end
    CacheWriteReq: begin
        v_req_valid = 1'b1;
        if (i_req_ready == 1'b1) begin
            v.state = WriteMem;
        end
    end
    ReadMem: begin
        vlxi[int'(r.srcid)].r_valid = i_resp_valid;
        vlxi[int'(r.srcid)].r_last = i_resp_valid;          // Lite interface
        if (r.req_type[L2_REQ_TYPE_SNOOP] == 1'b1) begin
            vlxi[int'(r.srcid)].r_data = r.req_wdata;
        end else begin
            vlxi[int'(r.srcid)].r_data = i_resp_rdata;
        end
        if ((|i_resp_status) == 1'b0) begin
            vlxi[int'(r.srcid)].r_resp = AXI_RESP_OKAY;
        end else begin
            vlxi[int'(r.srcid)].r_resp = AXI_RESP_SLVERR;
        end
        if (i_resp_valid == 1'b1) begin
            v.state = Idle;                                 // Wouldn't implement wait to accept because L1 is always ready
        end
    end
    WriteMem: begin
        vlxi[int'(r.srcid)].b_valid = i_resp_valid;
        if ((|i_resp_status) == 1'b0) begin
            vlxi[int'(r.srcid)].b_resp = AXI_RESP_OKAY;
        end else begin
            vlxi[int'(r.srcid)].b_resp = AXI_RESP_SLVERR;
        end
        if (i_resp_valid == 1'b1) begin
            v.state = Idle;                                 // Wouldn't implement wait to accept because L1 is always ready
        end
    end
    snoop_ac: begin
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            vlxi[i].ac_valid = r.ac_valid[i];
            vlxi[i].ac_addr = r.req_addr;
            if (r.req_type[L2_REQ_TYPE_UNIQUE] == 1'b1) begin
                vlxi[i].ac_snoop = AC_SNOOP_READ_UNIQUE;
            end else begin
                vlxi[i].ac_snoop = 4'h0;
            end
            if ((r.ac_valid[i] == 1'b1) && (vcoreo[i].ac_ready == 1'b1)) begin
                vb_ac_valid[i] = 1'b0;
                vb_cr_ready[i] = 1'b1;
            end
        end
        v.ac_valid = vb_ac_valid;
        v.cr_ready = vb_cr_ready;
        if ((|vb_ac_valid) == 1'b0) begin
            v.state = snoop_cr;
        end
    end
    snoop_cr: begin
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            vlxi[i].cr_ready = r.cr_ready[i];
            if ((r.cr_ready[i] == 1'b1) && (vcoreo[i].cr_valid == 1'b1)) begin
                vb_cr_ready[i] = 1'b0;
                if (vcoreo[i].cr_resp[0] == 1'b1) begin     // data transaction flag ACE spec
                    vb_cd_ready[i] = 1'b1;
                end
            end
        end
        v.cr_ready = vb_cr_ready;
        v.cd_ready = vb_cd_ready;
        if ((|vb_cr_ready) == 1'b0) begin
            if ((|vb_cd_ready) == 1'b1) begin
                v.state = snoop_cd;
            end else if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
                v.state = CacheWriteReq;
            end else begin
                v.state = CacheReadReq;
            end
        end
    end
    snoop_cd: begin
        // Here only to read Unique data from L1 and write to L2
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
            vlxi[i].cd_ready = r.cd_ready[i];
            if ((r.cd_ready[i] == 1'b1) && (vcoreo[i].cd_valid == 1'b1)) begin
                vb_cd_ready[i] = 1'b0;
                v.req_wdata = vcoreo[i].cd_data;
            end
        end
        v.cd_ready = vb_cd_ready;
        if ((|vb_cd_ready) == 1'b0) begin
            if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
                v.state = CacheWriteReq;
            end else begin
                v.state = CacheReadReq;
                v.req_wstrb = '1;
            end
            // write to L2 for Read and Write requests
            vb_req_type[L2_REQ_TYPE_WRITE] = 1'b1;
            vb_req_type[L2_REQ_TYPE_SNOOP] = 1'b1;
            v.req_type = vb_req_type;
        end
    end
    default: begin
    end
    endcase

    if (~async_reset && i_nrst == 1'b0) begin
        v = L2Destination_r_reset;
    end

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) begin
        o_l1i[i] = vlxi[i];                                 // vector should be assigned in cycle in systemc
    end

    o_req_valid = v_req_valid;
    o_req_type = r.req_type;
    o_req_addr = r.req_addr;
    o_req_size = r.req_size;
    o_req_prot = r.req_prot;
    o_req_wdata = r.req_wdata;
    o_req_wstrb = r.req_wstrb;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= L2Destination_r_reset;
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

endmodule: L2Destination
