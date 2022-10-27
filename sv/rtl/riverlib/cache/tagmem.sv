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

module TagMem #(
    parameter bit async_reset = 1'b0,
    parameter int abus = 64,                                // system bus address width (64 or 32 bits)
    parameter int ibits = 6,                                // lines memory address width (usually 6..8)
    parameter int lnbits = 5,                               // One line bits: log2(bytes_per_line)
    parameter int flbits = 4,                               // total flags number saved with address tag
    parameter int snoop = 0                                 // 0 Snoop port disabled; 1 Enabled (L2 caching)
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [abus-1:0] i_addr,
    input logic [(2**lnbits)-1:0] i_wstrb,
    input logic [(8 * (2**lnbits))-1:0] i_wdata,
    input logic [flbits-1:0] i_wflags,
    output logic [abus-1:0] o_raddr,
    output logic [(8 * (2**lnbits))-1:0] o_rdata,
    output logic [flbits-1:0] o_rflags,
    output logic o_hit,
    // L2 snoop port, active when snoop = 1
    input logic [abus-1:0] i_snoop_addr,
    output logic [flbits-1:0] o_snoop_flags
);

localparam int TAG_BITS = ((abus - ibits) - lnbits);
localparam int TAG_WITH_FLAGS = (TAG_BITS + flbits);

typedef struct {
    logic [TAG_BITS-1:0] tagaddr;
    logic [ibits-1:0] index;
    logic [TAG_BITS-1:0] snoop_tagaddr;
} TagMem_registers;

const TagMem_registers TagMem_r_reset = '{
    '0,                                 // tagaddr
    '0,                                 // index
    '0                                  // snoop_tagaddr
};

logic [ibits-1:0] wb_index;
logic [TAG_WITH_FLAGS-1:0] wb_tago_rdata;
logic [TAG_WITH_FLAGS-1:0] wb_tagi_wdata;
logic w_tagi_we;
logic [ibits-1:0] wb_snoop_index;
logic [TAG_BITS-1:0] wb_snoop_tagaddr;
logic [TAG_WITH_FLAGS-1:0] wb_tago_snoop_rdata;
TagMem_registers r, rin;

// bwe = byte write enable

ram_cache_bwe_tech #(
    .abits(ibits),
    .dbits((8 * (2**lnbits)))
) data0 (
    .i_clk(i_clk),
    .i_addr(wb_index),
    .i_wena(i_wstrb),
    .i_wdata(i_wdata),
    .o_rdata(o_rdata)
);


ram_tech #(
    .abits(ibits),
    .dbits(TAG_WITH_FLAGS)
) tag0 (
    .i_clk(i_clk),
    .i_addr(wb_index),
    .i_wena(w_tagi_we),
    .i_wdata(wb_tagi_wdata),
    .o_rdata(wb_tago_rdata)
);


generate
    if (snoop) begin: snoop_en
        ram_tech #(
            .abits(ibits),
            .dbits(TAG_WITH_FLAGS)
        ) tagsnoop0 (
            .i_clk(i_clk),
            .i_addr(wb_snoop_index),
            .i_wena(w_tagi_we),
            .i_wdata(wb_tagi_wdata),
            .o_rdata(wb_tago_snoop_rdata)
        );
    end: snoop_en
    else begin: snoop_dis
        assign wb_tago_snoop_rdata = '0;
    end: snoop_dis

endgenerate

always_comb
begin: comb_proc
    TagMem_registers v;
    logic [ibits-1:0] vb_index;
    logic [abus-1:0] vb_raddr;
    logic [TAG_WITH_FLAGS-1:0] vb_tagi_wdata;
    logic v_hit;
    logic [ibits-1:0] vb_snoop_index;
    logic [TAG_BITS-1:0] vb_snoop_tagaddr;
    logic [flbits-1:0] vb_snoop_flags;

    vb_index = 0;
    vb_raddr = 0;
    vb_tagi_wdata = 0;
    v_hit = 0;
    vb_snoop_index = 0;
    vb_snoop_tagaddr = 0;
    vb_snoop_flags = 0;

    v = r;


    if (r.tagaddr == wb_tago_rdata[(TAG_BITS - 1): 0]) begin
        v_hit = wb_tago_rdata[TAG_BITS];                    // valid bit
    end

    vb_raddr[(abus - 1): (ibits + lnbits)] = wb_tago_rdata[(TAG_BITS - 1): 0];
    vb_raddr[((ibits + lnbits) - 1): lnbits] = r.index;

    vb_index = i_addr[((ibits + lnbits) - 1): lnbits];
    vb_tagi_wdata[(TAG_BITS - 1): 0] = i_addr[(abus - 1): (ibits + lnbits)];
    vb_tagi_wdata[(TAG_WITH_FLAGS - 1): TAG_BITS] = i_wflags;

    if (snoop == 1) begin
        vb_snoop_flags = wb_tago_snoop_rdata[(TAG_WITH_FLAGS - 1): TAG_BITS];
        vb_snoop_index = i_snoop_addr[((ibits + lnbits) - 1): lnbits];
        vb_snoop_tagaddr = i_snoop_addr[(abus - 1): (ibits + lnbits)];
        if ((|i_wstrb) == 1'b1) begin
            vb_snoop_index = vb_index;
        end
        if (r.snoop_tagaddr != wb_tago_snoop_rdata[(TAG_BITS - 1): 0]) begin
            vb_snoop_flags = '0;
        end
    end

    v.tagaddr = vb_tagi_wdata[(TAG_BITS - 1): 0];
    v.index = vb_index;
    v.snoop_tagaddr = vb_snoop_tagaddr;

    if (~async_reset && i_nrst == 1'b0) begin
        v = TagMem_r_reset;
    end

    wb_index = vb_index;
    w_tagi_we = (|i_wstrb);
    wb_tagi_wdata = vb_tagi_wdata;

    o_raddr = vb_raddr;
    o_rflags = wb_tago_rdata[(TAG_WITH_FLAGS - 1): TAG_BITS];
    o_hit = v_hit;

    wb_snoop_index = vb_snoop_index;
    wb_snoop_tagaddr = vb_snoop_tagaddr;
    o_snoop_flags = vb_snoop_flags;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= TagMem_r_reset;
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

endmodule: TagMem
