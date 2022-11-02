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

module L2SerDes #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    output types_river_pkg::axi4_l2_in_type o_l2i,
    input types_river_pkg::axi4_l2_out_type i_l2o,
    input types_amba_pkg::axi4_master_in_type i_msti,
    output types_amba_pkg::axi4_master_out_type o_msto
);

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;
import l2serdes_pkg::*;

L2SerDes_registers r, rin;

function logic [7:0] size2len(input logic [2:0] size);
logic [7:0] ret;
begin
    case (size)
    3'h4: begin                                             // 16 Bytes
        ret = 8'h01;
    end
    3'h5: begin                                             // 32 Bytes
        ret = 8'h03;
    end
    3'h6: begin                                             // 64 Bytes
        ret = 8'h07;
    end
    3'h7: begin                                             // 128 Bytes
        ret = 8'h0f;
    end
    default: begin
        ret = '0;
    end
    endcase
    return ret;
end
endfunction: size2len

function logic [2:0] size2size(input logic [2:0] size);
logic [2:0] ret;
begin
    if (size >= 3'h3) begin
        ret = 3'h3;
    end else begin
        ret = size;
    end
    return ret;
end
endfunction: size2size

always_comb
begin: comb_proc
    L2SerDes_registers v;
    logic v_req_mem_ready;
    logic [busw-1:0] vb_r_data;
    logic [linew-1:0] vb_line_o;
    logic v_r_valid;
    logic v_w_valid;
    logic v_w_last;
    logic v_w_ready;
    logic [7:0] vb_len;
    logic [2:0] vb_size;
    logic [linew-1:0] t_line;
    logic [lineb-1:0] t_wstrb;
    axi4_l2_in_type vl2i;
    axi4_master_out_type vmsto;

    v_req_mem_ready = 0;
    vb_r_data = 0;
    vb_line_o = 0;
    v_r_valid = 0;
    v_w_valid = 0;
    v_w_last = 0;
    v_w_ready = 0;
    vb_len = 0;
    vb_size = 0;
    t_line = 0;
    t_wstrb = 0;
    vl2i = axi4_l2_in_none;
    vmsto = axi4_master_out_none;

    v = r;

    t_line = r.line;
    t_wstrb = r.wstrb;
    vb_r_data = i_msti.r_data;
    vb_line_o = r.line;
    for (int i = 0; i < SERDES_BURST_LEN; i++) begin
        if (r.rmux[i] == 1'b1) begin
            vb_line_o[(i * busw) +: busw] = vb_r_data;
        end
    end

    if ((i_l2o.b_ready == 1'b1) && (i_msti.b_valid == 1'b1)) begin
        v.b_wait = 1'b0;
    end

    case (r.state)
    State_Idle: begin
        v_req_mem_ready = 1'b1;
    end
    State_Read: begin
        if (i_msti.r_valid == 1'b1) begin
            v.line = vb_line_o;
            v.rmux = {r.rmux[(SERDES_BURST_LEN - 2): 0], {1{1'b0}}};
            if ((|r.req_len) == 1'b0) begin
                v_r_valid = 1'b1;
                v_req_mem_ready = 1'b1;
            end else begin
                v.req_len = (r.req_len - 1);
            end
        end
    end
    State_Write: begin
        v_w_valid = 1'b1;
        if ((|r.req_len) == 1'b0) begin
            v_w_last = 1'b1;
        end
        if (i_msti.w_ready == 1'b1) begin
            t_line[(linew - 1): (linew - busw)] = '0;
            t_line[((linew - busw) - 1): 0] = r.line[(linew - 1): busw];
            v.line = t_line;
            t_wstrb[(lineb - 1): (lineb - busb)] = '0;
            t_wstrb[((lineb - busb) - 1): 0] = r.wstrb[(lineb - 1): busb];
            v.wstrb = t_wstrb;
            if ((|r.req_len) == 1'b0) begin
                v_w_ready = 1'b1;
                v.b_wait = 1'b1;
                v_req_mem_ready = 1'b1;
            end else begin
                v.req_len = (r.req_len - 1);
            end
        end
    end
    default: begin
    end
    endcase

    if (i_l2o.ar_valid == 1'b1) begin
        vb_len = size2len(i_l2o.ar_bits.size);
        vb_size = size2size(i_l2o.ar_bits.size);
    end else begin
        vb_len = size2len(i_l2o.aw_bits.size);
        vb_size = size2size(i_l2o.aw_bits.size);
    end

    if (v_req_mem_ready == 1'b1) begin
        if ((i_l2o.ar_valid && i_msti.ar_ready) == 1'b1) begin
            v.state = State_Read;
            v.rmux = 4'd1;
        end else if ((i_l2o.aw_valid && i_msti.aw_ready) == 1'b1) begin
            v.line = i_l2o.w_data;                          // Undocumented RIVER (Axi-lite feature)
            v.wstrb = i_l2o.w_strb;
            v.state = State_Write;
        end else begin
            v.state = State_Idle;
        end
        v.req_len = vb_len;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = L2SerDes_r_reset;
    end

    vmsto.aw_valid = i_l2o.aw_valid;
    vmsto.aw_bits.addr = i_l2o.aw_bits.addr;
    vmsto.aw_bits.len = vb_len;                             // burst len = len[7:0] + 1
    vmsto.aw_bits.size = vb_size;                           // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.aw_bits.burst = 2'h1;                             // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    vmsto.aw_bits.lock = i_l2o.aw_bits.lock;
    vmsto.aw_bits.cache = i_l2o.aw_bits.cache;
    vmsto.aw_bits.prot = i_l2o.aw_bits.prot;
    vmsto.aw_bits.qos = i_l2o.aw_bits.qos;
    vmsto.aw_bits.region = i_l2o.aw_bits.region;
    vmsto.aw_id = i_l2o.aw_id;
    vmsto.aw_user = i_l2o.aw_user;
    vmsto.w_valid = v_w_valid;
    vmsto.w_last = v_w_last;
    vmsto.w_data = r.line[(busw - 1): 0];
    vmsto.w_strb = r.wstrb[(busb - 1): 0];
    vmsto.w_user = i_l2o.w_user;
    vmsto.b_ready = i_l2o.b_ready;
    vmsto.ar_valid = i_l2o.ar_valid;
    vmsto.ar_bits.addr = i_l2o.ar_bits.addr;
    vmsto.ar_bits.len = vb_len;                             // burst len = len[7:0] + 1
    vmsto.ar_bits.size = vb_size;                           // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.ar_bits.burst = 2'h1;                             // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    vmsto.ar_bits.lock = i_l2o.ar_bits.lock;
    vmsto.ar_bits.cache = i_l2o.ar_bits.cache;
    vmsto.ar_bits.prot = i_l2o.ar_bits.prot;
    vmsto.ar_bits.qos = i_l2o.ar_bits.qos;
    vmsto.ar_bits.region = i_l2o.ar_bits.region;
    vmsto.ar_id = i_l2o.ar_id;
    vmsto.ar_user = i_l2o.ar_user;
    vmsto.r_ready = i_l2o.r_ready;

    vl2i.aw_ready = i_msti.aw_ready;
    vl2i.w_ready = v_w_ready;
    vl2i.b_valid = (i_msti.b_valid && r.b_wait);
    vl2i.b_resp = i_msti.b_resp;
    vl2i.b_id = i_msti.b_id;
    vl2i.b_user = i_msti.b_user;
    vl2i.ar_ready = i_msti.ar_ready;
    vl2i.r_valid = v_r_valid;
    vl2i.r_resp = i_msti.r_resp;
    vl2i.r_data = vb_line_o;
    vl2i.r_last = v_r_valid;
    vl2i.r_id = i_msti.r_id;
    vl2i.r_user = i_msti.r_user;

    o_msto = vmsto;
    o_l2i = vl2i;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= L2SerDes_r_reset;
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

endmodule: L2SerDes
