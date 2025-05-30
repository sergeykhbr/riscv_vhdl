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

module axi_dma #(
    parameter int abits = 48,                               // adress bits used
    parameter logic async_reset = 1'b0,
    parameter int userbits = 1
)
(
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_clk,                                      // CPU clock
    output logic o_req_mem_ready,                           // Ready to accept next data
    input logic i_req_mem_valid,                            // Request data is ready to accept
    input logic i_req_mem_write,                            // 0=read; 1=write operation
    input logic [9:0] i_req_mem_bytes,                      // 0=1024 B; 4=DWORD; 8=QWORD; ...
    input logic [abits-1:0] i_req_mem_addr,                 // Address to read/write
    input logic [7:0] i_req_mem_strob,                      // Byte enabling write strob
    input logic [63:0] i_req_mem_data,                      // Data to write
    input logic i_req_mem_last,                             // Last data payload in a sequence
    output logic o_resp_mem_valid,                          // Read/Write data is valid. All write transaction with valid response.
    output logic o_resp_mem_last,                           // Last response in a sequence.
    output logic o_resp_mem_fault,                          // Error on memory access
    output logic [abits-1:0] o_resp_mem_addr,               // Read address value
    output logic [63:0] o_resp_mem_data,                    // Read data value
    input logic i_resp_mem_ready,                           // Ready to accept response
    input types_amba_pkg::axi4_master_in_type i_msti,       // AXI master input
    output types_amba_pkg::axi4_master_out_type o_msto,     // AXI master output
    output logic o_dbg_valid,
    output logic [63:0] o_dbg_payload
);

import types_amba_pkg::*;
localparam bit [2:0] state_idle = 3'd0;
localparam bit [2:0] state_ar = 3'd1;
localparam bit [2:0] state_r = 3'd2;
localparam bit [2:0] state_r_wait_accept = 3'd3;
localparam bit [2:0] state_aw = 3'd4;
localparam bit [2:0] state_w = 3'd5;
localparam bit [2:0] state_w_wait_accept = 3'd6;
localparam bit [2:0] state_b = 3'd7;

typedef struct {
    logic [2:0] state;
    logic ar_valid;
    logic aw_valid;
    logic w_valid;
    logic r_ready;
    logic b_ready;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] req_addr;
    logic [CFG_SYSBUS_DATA_BITS-1:0] req_wdata;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] req_wstrb;
    logic [2:0] req_size;
    logic [7:0] req_len;
    logic req_last;
    logic req_ready;
    logic resp_valid;
    logic resp_last;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] resp_addr;
    logic [CFG_SYSBUS_DATA_BITS-1:0] resp_data;
    logic resp_error;
    logic [CFG_SYSBUS_USER_BITS-1:0] user_count;
    logic dbg_valid;
    logic [63:0] dbg_payload;
} axi_dma_registers;

const axi_dma_registers axi_dma_r_reset = '{
    state_idle,                         // state
    1'b0,                               // ar_valid
    1'b0,                               // aw_valid
    1'b0,                               // w_valid
    1'b0,                               // r_ready
    1'b0,                               // b_ready
    '0,                                 // req_addr
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // req_size
    '0,                                 // req_len
    1'b0,                               // req_last
    1'b1,                               // req_ready
    '0,                                 // resp_valid
    '0,                                 // resp_last
    '0,                                 // resp_addr
    '0,                                 // resp_data
    1'b0,                               // resp_error
    '0,                                 // user_count
    1'b0,                               // dbg_valid
    '0                                  // dbg_payload
};
axi_dma_registers r;
axi_dma_registers rin;


always_comb
begin: comb_proc
    axi_dma_registers v;
    logic [9:0] vb_req_mem_bytes_m1;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] vb_req_addr_inc;
    logic [CFG_SYSBUS_DATA_BITS-1:0] vb_r_data_swap;
    axi4_master_out_type vmsto;

    v = r;
    vb_req_mem_bytes_m1 = '0;
    vb_req_addr_inc = '0;
    vb_r_data_swap = '0;
    vmsto = axi4_master_out_none;

    vb_req_mem_bytes_m1 = (i_req_mem_bytes - 1);
    vb_req_addr_inc = r.req_addr;

    // Byte swapping:
    if (r.req_size == 3'd0) begin
        vb_req_addr_inc[9: 0] = (r.req_addr[9: 0] + 10'h001);
        if (r.req_addr[2: 0] == 3'd0) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[7: 0], i_msti.r_data[7: 0], i_msti.r_data[7: 0], i_msti.r_data[7: 0]};
        end else if (r.req_addr[2: 0] == 3'd1) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[15: 8], i_msti.r_data[15: 8], i_msti.r_data[15: 8], i_msti.r_data[15: 8]};
        end else if (r.req_addr[2: 0] == 3'd2) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[23: 16], i_msti.r_data[23: 16], i_msti.r_data[23: 16], i_msti.r_data[23: 16]};
        end else if (r.req_addr[2: 0] == 3'd3) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[31: 24], i_msti.r_data[31: 24], i_msti.r_data[31: 24], i_msti.r_data[31: 24]};
        end else if (r.req_addr[2: 0] == 3'd4) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[39: 32], i_msti.r_data[39: 32], i_msti.r_data[39: 32], i_msti.r_data[39: 32]};
        end else if (r.req_addr[2: 0] == 3'd5) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[47: 40], i_msti.r_data[47: 40], i_msti.r_data[47: 40], i_msti.r_data[47: 40]};
        end else if (r.req_addr[2: 0] == 3'd6) begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[55: 48], i_msti.r_data[55: 48], i_msti.r_data[55: 48], i_msti.r_data[55: 48]};
        end else begin
            vb_r_data_swap[31: 0] = {i_msti.r_data[63: 56], i_msti.r_data[63: 56], i_msti.r_data[63: 56], i_msti.r_data[63: 56]};
        end
        vb_r_data_swap[63: 32] = vb_r_data_swap[31: 0];
    end else if (r.req_size == 3'd1) begin
        vb_req_addr_inc[9: 0] = (r.req_addr[9: 0] + 10'h002);
        if (r.req_addr[2: 1] == 2'd0) begin
            vb_r_data_swap = {i_msti.r_data[15: 0], i_msti.r_data[15: 0], i_msti.r_data[15: 0], i_msti.r_data[15: 0]};
        end else if (r.req_addr[2: 1] == 2'd1) begin
            vb_r_data_swap = {i_msti.r_data[31: 16], i_msti.r_data[31: 16], i_msti.r_data[31: 16], i_msti.r_data[31: 16]};
        end else if (r.req_addr[2: 1] == 2'd2) begin
            vb_r_data_swap = {i_msti.r_data[47: 32], i_msti.r_data[47: 32], i_msti.r_data[47: 32], i_msti.r_data[47: 32]};
        end else begin
            vb_r_data_swap = {i_msti.r_data[63: 48], i_msti.r_data[63: 48], i_msti.r_data[63: 48], i_msti.r_data[63: 48]};
        end
    end else if (r.req_size == 3'd2) begin
        vb_req_addr_inc[9: 0] = (r.req_addr[9: 0] + 10'h004);
        if (r.req_addr[2] == 1'b0) begin
            vb_r_data_swap = {i_msti.r_data[31: 0], i_msti.r_data[31: 0]};
        end else begin
            vb_r_data_swap = {i_msti.r_data[63: 32], i_msti.r_data[63: 32]};
        end
    end else begin
        vb_req_addr_inc[9: 0] = (r.req_addr[9: 0] + 10'h008);
        vb_r_data_swap = i_msti.r_data;
    end

    v.dbg_valid = 1'b0;
    case (r.state)
    state_idle: begin
        v.req_ready = 1'b1;
        v.resp_valid = 1'b0;
        v.resp_last = 1'b0;
        if (i_req_mem_valid == 1'b1) begin
            v.req_ready = 1'b0;
            v.req_addr = {'0, i_req_mem_addr};
            if (i_req_mem_bytes == 10'd1) begin
                v.req_size = 3'd0;
                v.req_len = 8'd0;
            end else if (i_req_mem_bytes == 10'd2) begin
                v.req_size = 3'd1;
                v.req_len = 8'd0;
            end else if (i_req_mem_bytes == 10'd4) begin
                v.req_size = 3'd2;
                v.req_len = 8'd0;
            end else begin
                v.req_size = 3'd3;
                v.req_len = {1'b0, vb_req_mem_bytes_m1[9: 3]};
            end
            if (i_req_mem_write == 1'b0) begin
                v.ar_valid = 1'b1;
                v.state = state_ar;
                v.req_wdata = 64'd0;
                v.req_wstrb = 8'd0;
                v.req_last = 1'b0;
            end else begin
                v.aw_valid = 1'b1;
                v.w_valid = i_req_mem_last;                 // Try to use AXI Lite
                v.req_wdata = i_req_mem_data;
                v.req_wstrb = i_req_mem_strob;
                v.req_last = i_req_mem_last;
                v.state = state_aw;
                v.dbg_valid = 1'b1;
            end
            // debug interface:
            v.dbg_payload = {1'b1,
                    i_req_mem_addr[10: 0],
                    2'h0,
                    i_req_mem_bytes,
                    i_req_mem_strob,
                    i_req_mem_data[31: 0]};
        end
    end
    state_ar: begin
        if (i_msti.ar_ready == 1'b1) begin
            v.resp_addr = r.req_addr;
            v.ar_valid = 1'b0;
            v.r_ready = 1'b1;
            v.state = state_r;
        end
    end
    state_r: begin
        if (i_msti.r_valid == 1'b1) begin
            v.resp_valid = 1'b1;
            v.resp_addr = r.req_addr;
            v.resp_data = vb_r_data_swap;
            v.resp_last = i_msti.r_last;
            v.resp_error = i_msti.r_resp[1];
            v.req_addr = vb_req_addr_inc;

            if ((i_resp_mem_ready == 1'b0) || (i_msti.r_last == 1'b1)) begin
                v.r_ready = 1'b0;
                v.state = state_r_wait_accept;
            end
        end
    end
    state_r_wait_accept: begin
        if (i_resp_mem_ready == 1'b1) begin
            v.resp_valid = 1'b0;
            // debug interface:
            v.dbg_valid = 1'b1;
            v.dbg_payload = {1'b0, r.dbg_payload[62: 32], r.resp_data[31: 0]};

            if (r.resp_last == 1'b1) begin
                v.resp_last = 1'b0;
                v.user_count = (r.user_count + 1);
                v.req_ready = 1'b1;
                v.state = state_idle;
            end else begin
                v.r_ready = 1'b1;
                v.state = state_r;
            end
        end
    end

    state_aw: begin
        if (i_msti.aw_ready == 1'b1) begin
            v.aw_valid = 1'b0;
            v.state = state_w;
            v.resp_addr = r.req_addr;

            if (r.w_valid && (i_msti.w_ready == 1'b1)) begin
                // AXI Lite accepted
                v.w_valid = 1'b0;
                v.b_ready = i_resp_mem_ready;
                v.state = state_b;
            end else begin
                v.w_valid = 1'b1;
                v.req_ready = (~r.req_last);
                v.state = state_w;
            end
        end
    end
    state_w: begin
        if (i_msti.w_ready == 1'b1) begin
            // Burst write:
            v.w_valid = i_req_mem_valid;
            v.req_last = i_req_mem_last;
            v.req_wstrb = i_req_mem_strob;
            v.req_wdata = i_req_mem_data;
            v.req_addr = vb_req_addr_inc;
            if (r.req_last == 1'b1) begin
                v.req_last = 1'b0;
                v.w_valid = 1'b0;
                v.b_ready = i_resp_mem_ready;
                v.req_ready = 1'b0;
                v.state = state_b;
            end
        end else if (r.w_valid == 1'b1) begin
            v.req_ready = 1'b0;
            v.state = state_w_wait_accept;
        end
    end
    state_w_wait_accept: begin
        if (i_msti.w_ready == 1'b1) begin
            v.w_valid = 1'b0;
            if (r.req_last == 1'b1) begin
                v.req_last = 1'b0;
                v.b_ready = i_resp_mem_ready;
                v.state = state_b;
            end else begin
                v.req_ready = 1'b1;
                v.state = state_w;
            end
        end
    end
    state_b: begin
        v.b_ready = i_resp_mem_ready;
        if ((r.b_ready == 1'b1) && (i_msti.b_valid == 1'b1)) begin
            v.b_ready = 1'b0;
            v.state = state_idle;
            v.user_count = (r.user_count + 1);
            v.resp_error = i_msti.b_resp[1];
            v.resp_valid = 1'b1;
            v.resp_last = 1'b1;
        end
    end
    endcase

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = axi_dma_r_reset;
    end

    o_resp_mem_valid = r.resp_valid;
    o_resp_mem_last = r.resp_last;
    o_resp_mem_fault = r.resp_error;
    o_resp_mem_addr = r.resp_addr;
    o_resp_mem_data = r.resp_data;
    o_req_mem_ready = r.req_ready;

    vmsto.ar_valid = r.ar_valid;
    vmsto.ar_bits.addr = r.req_addr;
    vmsto.ar_bits.size = r.req_size;
    vmsto.ar_bits.len = r.req_len;
    vmsto.ar_user = r.user_count;
    vmsto.ar_bits.burst = AXI_BURST_INCR;
    vmsto.r_ready = r.r_ready;

    vmsto.aw_valid = r.aw_valid;
    vmsto.aw_bits.addr = r.req_addr;
    vmsto.aw_bits.size = r.req_size;
    vmsto.aw_bits.len = r.req_len;
    vmsto.aw_user = r.user_count;
    vmsto.aw_bits.burst = AXI_BURST_INCR;
    vmsto.w_valid = r.w_valid;
    vmsto.w_last = r.req_last;
    vmsto.w_data = r.req_wdata;
    vmsto.w_strb = r.req_wstrb;
    vmsto.w_user = r.user_count;
    vmsto.b_ready = r.b_ready;

    o_msto = vmsto;
    o_dbg_valid = r.dbg_valid;
    o_dbg_payload = r.dbg_payload;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= axi_dma_r_reset;
            end else begin
                r <= rin;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r <= rin;
        end

    end: async_r_dis
endgenerate

endmodule: axi_dma
