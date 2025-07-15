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

module afifo_xslv #(
    parameter int abits_depth = 2,                          // Depth of the address channels AR/AW/B
    parameter int dbits_depth = 10                          // Depth of the data channels R/W
)
(
    input logic i_xslv_nrst,                                // Input requests reset (from master to this slave)
    input logic i_xslv_clk,                                 // Input requests clock (from master to this slave)
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // Input slave interface
    output types_amba_pkg::axi4_slave_out_type o_xslvo,     // Response on input slave reuqest
    input logic i_xmst_nrst,                                // Output request reset
    input logic i_xmst_clk,                                 // Output request clock
    output types_amba_pkg::axi4_slave_in_type o_xmsto,      // Output request to connected slave
    input types_amba_pkg::axi4_slave_out_type i_xmsti       // Response from the connected slave
);

import types_amba_pkg::*;
localparam int AR_REQ_WIDTH = (CFG_SYSBUS_ADDR_BITS  // addr
        + 8  // len
        + 3  // size
        + 2  // burst
        + 1  // lock
        + 4  // cache
        + 3  // prot
        + 4  // qos
        + 4  // region
        + CFG_SYSBUS_ID_BITS  // ar_id
        + CFG_SYSBUS_USER_BITS  // ar_user
);
localparam int AW_REQ_WIDTH = (CFG_SYSBUS_ADDR_BITS  // addr
        + 8  // len
        + 3  // size
        + 2  // burst
        + 1  // lock
        + 4  // cache
        + 3  // prot
        + 4  // qos
        + 4  // region
        + CFG_SYSBUS_ID_BITS  // aw_id
        + CFG_SYSBUS_USER_BITS  // aw_user
);
localparam int W_REQ_WIDTH = (CFG_SYSBUS_DATA_BITS  // w_data
        + 1  // w_last
        + CFG_SYSBUS_DATA_BYTES  // w_strb
        + CFG_SYSBUS_USER_BITS  // w_user
);
localparam int R_RESP_WIDTH = (2  // r_resp
        + CFG_SYSBUS_DATA_BITS  // r_data
        + 1  // r_last
        + CFG_SYSBUS_ID_BITS  // r_id
        + CFG_SYSBUS_USER_BITS  // r_user
);
localparam int B_RESP_WIDTH = (2  // b_resp
        + CFG_SYSBUS_ID_BITS  // b_id
        + CFG_SYSBUS_USER_BITS  // b_user
);

logic w_req_ar_valid_i;
logic [AR_REQ_WIDTH-1:0] wb_req_ar_payload_i;
logic w_req_ar_wready_o;
logic w_req_ar_rd_i;
logic [AR_REQ_WIDTH-1:0] wb_req_ar_payload_o;
logic w_req_ar_rvalid_o;
logic w_req_aw_valid_i;
logic [AW_REQ_WIDTH-1:0] wb_req_aw_payload_i;
logic w_req_aw_wready_o;
logic w_req_aw_rd_i;
logic [AW_REQ_WIDTH-1:0] wb_req_aw_payload_o;
logic w_req_aw_rvalid_o;
logic w_req_w_valid_i;
logic [W_REQ_WIDTH-1:0] wb_req_w_payload_i;
logic w_req_w_wready_o;
logic w_req_w_rd_i;
logic [W_REQ_WIDTH-1:0] wb_req_w_payload_o;
logic w_req_w_rvalid_o;
logic w_resp_r_valid_i;
logic [R_RESP_WIDTH-1:0] wb_resp_r_payload_i;
logic w_resp_r_wready_o;
logic w_resp_r_rd_i;
logic [R_RESP_WIDTH-1:0] wb_resp_r_payload_o;
logic w_resp_r_rvalid_o;
logic w_resp_b_valid_i;
logic [B_RESP_WIDTH-1:0] wb_resp_b_payload_i;
logic w_resp_b_wready_o;
logic w_resp_b_rd_i;
logic [B_RESP_WIDTH-1:0] wb_resp_b_payload_o;
logic w_resp_b_rvalid_o;

cdc_afifo #(
    .abits(abits_depth),
    .dbits(AR_REQ_WIDTH)
) req_ar (
    .i_nrst(i_xslv_nrst),
    .i_wclk(i_xslv_clk),
    .i_wr(w_req_ar_valid_i),
    .i_wdata(wb_req_ar_payload_i),
    .o_wready(w_req_ar_wready_o),
    .i_rclk(i_xmst_clk),
    .i_rd(w_req_ar_rd_i),
    .o_rdata(wb_req_ar_payload_o),
    .o_rvalid(w_req_ar_rvalid_o)
);

cdc_afifo #(
    .abits(abits_depth),
    .dbits(AW_REQ_WIDTH)
) req_aw (
    .i_nrst(i_xslv_nrst),
    .i_wclk(i_xslv_clk),
    .i_wr(w_req_aw_valid_i),
    .i_wdata(wb_req_aw_payload_i),
    .o_wready(w_req_aw_wready_o),
    .i_rclk(i_xmst_clk),
    .i_rd(w_req_aw_rd_i),
    .o_rdata(wb_req_aw_payload_o),
    .o_rvalid(w_req_aw_rvalid_o)
);

cdc_afifo #(
    .abits(dbits_depth),
    .dbits(W_REQ_WIDTH)
) req_w (
    .i_nrst(i_xslv_nrst),
    .i_wclk(i_xslv_clk),
    .i_wr(w_req_w_valid_i),
    .i_wdata(wb_req_w_payload_i),
    .o_wready(w_req_w_wready_o),
    .i_rclk(i_xmst_clk),
    .i_rd(w_req_w_rd_i),
    .o_rdata(wb_req_w_payload_o),
    .o_rvalid(w_req_w_rvalid_o)
);

cdc_afifo #(
    .abits(dbits_depth),
    .dbits(R_RESP_WIDTH)
) resp_r (
    .i_nrst(i_xmst_nrst),
    .i_wclk(i_xmst_clk),
    .i_wr(w_resp_r_valid_i),
    .i_wdata(wb_resp_r_payload_i),
    .o_wready(w_resp_r_wready_o),
    .i_rclk(i_xslv_clk),
    .i_rd(w_resp_r_rd_i),
    .o_rdata(wb_resp_r_payload_o),
    .o_rvalid(w_resp_r_rvalid_o)
);

cdc_afifo #(
    .abits(abits_depth),
    .dbits(B_RESP_WIDTH)
) resp_b (
    .i_nrst(i_xmst_nrst),
    .i_wclk(i_xmst_clk),
    .i_wr(w_resp_b_valid_i),
    .i_wdata(wb_resp_b_payload_i),
    .o_wready(w_resp_b_wready_o),
    .i_rclk(i_xslv_clk),
    .i_rd(w_resp_b_rd_i),
    .o_rdata(wb_resp_b_payload_o),
    .o_rvalid(w_resp_b_rvalid_o)
);

always_comb
begin: comb_proc
    axi4_slave_out_type vb_xslvo;
    axi4_slave_in_type vb_xmsto;

    // ar channel write side:
    w_req_ar_valid_i = i_xslvi.ar_valid;
    vb_xslvo.ar_ready = w_req_ar_wready_o;
    wb_req_ar_payload_i = {i_xslvi.ar_bits.addr,
            i_xslvi.ar_bits.len,
            i_xslvi.ar_bits.size,
            i_xslvi.ar_bits.burst,
            i_xslvi.ar_bits.lock,
            i_xslvi.ar_bits.cache,
            i_xslvi.ar_bits.prot,
            i_xslvi.ar_bits.qos,
            i_xslvi.ar_bits.region,
            i_xslvi.ar_id,
            i_xslvi.ar_user};
    // ar channel read side:
    vb_xmsto.ar_valid = w_req_ar_rvalid_o;
    w_req_ar_rd_i = i_xmsti.ar_ready;
    vb_xmsto.ar_bits.addr = wb_req_ar_payload_o[84: 37];
    vb_xmsto.ar_bits.len = wb_req_ar_payload_o[36: 29];
    vb_xmsto.ar_bits.size = wb_req_ar_payload_o[28: 26];
    vb_xmsto.ar_bits.burst = wb_req_ar_payload_o[25: 24];
    vb_xmsto.ar_bits.lock = wb_req_ar_payload_o[23];
    vb_xmsto.ar_bits.cache = wb_req_ar_payload_o[22: 19];
    vb_xmsto.ar_bits.prot = wb_req_ar_payload_o[18: 16];
    vb_xmsto.ar_bits.qos = wb_req_ar_payload_o[15: 12];
    vb_xmsto.ar_bits.region = wb_req_ar_payload_o[11: 8];
    vb_xmsto.ar_id = wb_req_ar_payload_o[7: 3];
    vb_xmsto.ar_user = wb_req_ar_payload_o[2: 0];

    // aw channel write side:
    w_req_aw_valid_i = i_xslvi.aw_valid;
    vb_xslvo.aw_ready = w_req_aw_wready_o;
    wb_req_aw_payload_i = {i_xslvi.aw_bits.addr,
            i_xslvi.aw_bits.len,
            i_xslvi.aw_bits.size,
            i_xslvi.aw_bits.burst,
            i_xslvi.aw_bits.lock,
            i_xslvi.aw_bits.cache,
            i_xslvi.aw_bits.prot,
            i_xslvi.aw_bits.qos,
            i_xslvi.aw_bits.region,
            i_xslvi.aw_id,
            i_xslvi.aw_user};
    // aw channel read side:
    vb_xmsto.aw_valid = w_req_aw_rvalid_o;
    w_req_aw_rd_i = i_xmsti.aw_ready;
    vb_xmsto.aw_bits.addr = wb_req_aw_payload_o[84: 37];
    vb_xmsto.aw_bits.len = wb_req_aw_payload_o[36: 29];
    vb_xmsto.aw_bits.size = wb_req_aw_payload_o[28: 26];
    vb_xmsto.aw_bits.burst = wb_req_aw_payload_o[25: 24];
    vb_xmsto.aw_bits.lock = wb_req_aw_payload_o[23];
    vb_xmsto.aw_bits.cache = wb_req_aw_payload_o[22: 19];
    vb_xmsto.aw_bits.prot = wb_req_aw_payload_o[18: 16];
    vb_xmsto.aw_bits.qos = wb_req_aw_payload_o[15: 12];
    vb_xmsto.aw_bits.region = wb_req_aw_payload_o[11: 8];
    vb_xmsto.aw_id = wb_req_aw_payload_o[7: 3];
    vb_xmsto.aw_user = wb_req_aw_payload_o[2: 0];

    // w channel write side:
    w_req_w_valid_i = i_xslvi.w_valid;
    vb_xslvo.w_ready = w_req_w_wready_o;
    wb_req_w_payload_i = {i_xslvi.w_data,
            i_xslvi.w_last,
            i_xslvi.w_strb,
            i_xslvi.w_user};
    // w channel read side:
    vb_xmsto.w_valid = w_req_w_rvalid_o;
    w_req_w_rd_i = i_xmsti.w_ready;
    vb_xmsto.w_data = wb_req_w_payload_o[75: 12];
    vb_xmsto.w_last = wb_req_w_payload_o[11];
    vb_xmsto.w_strb = wb_req_w_payload_o[10: 3];
    vb_xmsto.w_user = wb_req_w_payload_o[2: 0];

    // r channel write side:
    w_resp_r_valid_i = i_xmsti.r_valid;
    vb_xmsto.r_ready = w_resp_r_wready_o;
    wb_resp_r_payload_i = {i_xmsti.r_resp,
            i_xmsti.r_data,
            i_xmsti.r_last,
            i_xmsti.r_id,
            i_xmsti.r_user};
    // r channel read side:
    vb_xslvo.r_valid = w_resp_r_rvalid_o;
    w_resp_r_rd_i = i_xslvi.r_ready;
    vb_xslvo.r_resp = wb_resp_r_payload_o[74: 73];
    vb_xslvo.r_data = wb_resp_r_payload_o[72: 9];
    vb_xslvo.r_last = wb_resp_r_payload_o[8];
    vb_xslvo.r_id = wb_resp_r_payload_o[7: 3];
    vb_xslvo.r_user = wb_resp_r_payload_o[2: 0];

    // b channel write side:
    w_resp_b_valid_i = i_xmsti.b_valid;
    vb_xmsto.b_ready = w_resp_b_wready_o;
    wb_resp_b_payload_i = {i_xmsti.b_resp,
            i_xmsti.b_id,
            i_xmsti.b_user};
    // b channel read side:
    vb_xslvo.b_valid = w_resp_b_rvalid_o;
    w_resp_b_rd_i = i_xslvi.b_ready;
    vb_xslvo.b_resp = wb_resp_b_payload_o[9: 8];
    vb_xslvo.b_id = wb_resp_b_payload_o[7: 3];
    vb_xslvo.b_user = wb_resp_b_payload_o[2: 0];

    o_xslvo = vb_xslvo;
    o_xmsto = vb_xmsto;
end: comb_proc

endmodule: afifo_xslv
