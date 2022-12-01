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

module ic_axi4_to_l1 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // AXI4 port
    input types_amba_pkg::axi4_master_out_type i_xmsto,
    output types_amba_pkg::axi4_master_in_type o_xmsti,
    // L1 port
    input types_river_pkg::axi4_l1_in_type i_l1i,
    output types_river_pkg::axi4_l1_out_type o_l1o
);

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import ic_axi4_to_l1_pkg::*;

ic_axi4_to_l1_registers r, rin;

always_comb
begin: comb_proc
    ic_axi4_to_l1_registers v;
    axi4_master_in_type vb_xmsti;
    axi4_l1_out_type vb_l1o;
    logic [(CFG_LOG2_L1CACHE_BYTES_PER_LINE - 3)-1:0] idx;  // request always 64 bits
    logic [XSIZE_TOTAL-1:0] vb_req_xbytes;
    logic [63:0] vb_req_mask;
    logic [L1CACHE_LINE_BITS-1:0] vb_r_data_modified;
    logic [L1CACHE_BYTES_PER_LINE-1:0] vb_line_wstrb;
    logic [63:0] vb_resp_data;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] t_req_addr;

    vb_xmsti = axi4_master_in_none;
    vb_l1o = axi4_l1_out_none;
    idx = 2'h0;
    vb_req_xbytes = 0;
    vb_req_mask = 0;
    vb_r_data_modified = 0;
    vb_line_wstrb = 0;
    vb_resp_data = 0;
    t_req_addr = 0;

    v = r;

    vb_xmsti = axi4_master_in_none;
    vb_l1o = axi4_l1_out_none;
    t_req_addr = r.req_addr;

    idx = r.req_addr[(CFG_LOG2_L1CACHE_BYTES_PER_LINE - 1): 3];
    vb_req_xbytes = XSizeToBytes(r.req_size);

    vb_req_mask = '0;
    for (int i = 0; i < 8; i++) begin
        if (r.req_wstrb[i] == 1'b1) begin
            vb_req_mask[(8 * i) +: 8] = 8'hff;
        end
    end

    vb_resp_data = i_l1i.r_data[(idx * 64) +: 64];

    vb_r_data_modified = i_l1i.r_data;
    vb_r_data_modified[(idx * 64) +: 64] = ((i_l1i.r_data[(idx * 64) +: 64] & (~vb_req_mask))
            | (r.req_wdata & vb_req_mask));

    vb_line_wstrb = '0;
    vb_line_wstrb[(idx * 8) +: 8] = r.req_wstrb;

    case (r.state)
    Idle: begin
        vb_xmsti.ar_ready = 1'b1;
        vb_xmsti.aw_ready = 1'b1;
        v.read_modify_write = 1'b0;
        v.writing = 1'b0;
        if (i_xmsto.aw_valid == 1'b1) begin
            // Convert AXI4 to AXI4Lite used in L1
            v.req_id = i_xmsto.aw_id;
            v.req_user = i_xmsto.aw_user;
            v.req_addr = i_xmsto.aw_bits.addr;
            v.req_size = i_xmsto.aw_bits.size;
            v.req_len = i_xmsto.aw_bits.len;
            v.req_prot = i_xmsto.aw_bits.prot;
            v.writing = 1'b1;
            v.state = WriteDataAccept;
        end else if (i_xmsto.ar_valid == 1'b1) begin
            v.req_id = i_xmsto.ar_id;
            v.req_user = i_xmsto.ar_user;
            v.req_addr = i_xmsto.ar_bits.addr;
            v.req_size = i_xmsto.ar_bits.size;
            v.req_len = i_xmsto.ar_bits.len;
            v.req_prot = i_xmsto.ar_bits.prot;
            v.state = ReadLineRequest;
        end
    end
    WriteDataAccept: begin
        vb_xmsti.w_ready = 1'b1;
        v.req_wdata = i_xmsto.w_data;
        v.req_wstrb = i_xmsto.w_strb;
        if (i_xmsto.w_valid == 1'b1) begin
            // Cachable memory read and make unique to modify line
            v.read_modify_write = 1'b1;
            v.state = ReadLineRequest;
        end
    end
    ReadLineRequest: begin
        vb_l1o.ar_valid = 1'b1;
        vb_l1o.ar_bits.addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
        vb_l1o.ar_bits.cache = ARCACHE_WRBACK_READ_ALLOCATE;
        vb_l1o.ar_bits.size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
        vb_l1o.ar_bits.len = '0;
        vb_l1o.ar_bits.prot = r.req_prot;
        vb_l1o.ar_snoop = ARSNOOP_READ_MAKE_UNIQUE;
        vb_l1o.ar_id = r.req_id;
        vb_l1o.ar_user = r.req_user;
        if (i_l1i.ar_ready == 1'b1) begin
            v.state = WaitReadLineResponse;
        end
    end
    WaitReadLineResponse: begin
        vb_l1o.r_ready = 1'b1;
        v.line_data = i_l1i.r_data;
        v.resp_data = vb_resp_data;
        if (i_l1i.r_valid == 1'b1) begin
            if (r.read_modify_write == 1'b1) begin
                v.line_data = vb_r_data_modified;
                v.line_wstrb = '1;
                v.state = WriteLineRequest;
            end else begin
                v.state = WaitReadAccept;
            end
        end
    end
    WriteLineRequest: begin
        vb_l1o.aw_valid = 1'b1;
        vb_l1o.aw_bits.addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
        vb_l1o.aw_bits.cache = AWCACHE_DEVICE_NON_BUFFERABLE;
        vb_l1o.aw_bits.size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
        vb_l1o.aw_bits.len = '0;
        vb_l1o.aw_bits.prot = r.req_prot;
        vb_l1o.aw_snoop = AWSNOOP_WRITE_NO_SNOOP;           // offloading non-cached always
        vb_l1o.aw_id = r.req_id;
        vb_l1o.aw_user = r.req_user;
        // axi lite for L2-cache
        vb_l1o.w_valid = 1'b1;
        vb_l1o.w_last = 1'b1;
        vb_l1o.w_data = r.line_data;
        vb_l1o.w_strb = r.line_wstrb;
        if ((i_l1i.aw_ready == 1'b1) && (i_l1i.w_ready == 1'b1)) begin
            if ((|r.req_len) == 1'b0) begin
                v.state = WaitWriteConfirmResponse;
            end else begin
                v.state = CheckBurst;
            end
        end
    end
    WaitWriteConfirmResponse: begin
        vb_l1o.b_ready = 1'b1;
        if (i_l1i.b_valid == 1'b1) begin
            v.state = WaitWriteAccept;
        end
    end
    WaitWriteAccept: begin
        vb_xmsti.b_valid = 1'b1;
        vb_xmsti.b_id = r.req_id;
        vb_xmsti.b_user = r.req_user;
        if (i_xmsto.b_ready == 1'b1) begin
            v.state = Idle;
        end
    end
    WaitReadAccept: begin
        vb_xmsti.r_valid = 1'b1;
        vb_xmsti.r_data = r.resp_data;
        vb_xmsti.r_last = (~(|r.req_len));
        vb_xmsti.r_id = r.req_id;
        vb_xmsti.r_user = r.req_user;
        if (i_xmsto.r_ready == 1'b1) begin
            v.state = CheckBurst;
        end
    end
    CheckBurst: begin
        if ((|r.req_len) == 1'b0) begin
            v.state = Idle;
        end else begin
            // Burst transaction to support external DMA engine
            v.req_len = (r.req_len - 1);
            t_req_addr[11: 0] = (r.req_addr[11: 0] + vb_req_xbytes);
            v.req_addr = t_req_addr;
            v.read_modify_write = 1'b0;
            if (r.writing == 1'b1) begin
                v.state = WriteDataAccept;
            end else begin
                v.state = ReadLineRequest;
            end
        end
    end
    default: begin
    end
    endcase

    o_xmsti = vb_xmsti;
    o_l1o = vb_l1o;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= ic_axi4_to_l1_r_reset;
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

endmodule: ic_axi4_to_l1
