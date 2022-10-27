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

module TagMemNWay #(
    parameter bit async_reset = 1'b0,
    parameter int abus = 64,                                // system bus address width (64 or 32 bits)
    parameter int waybits = 2,                              // log2 of number of ways bits (2 for 4 ways cache; 3 for 8 ways)
    parameter int ibits = 6,                                // lines memory address width (usually 6..8)
    parameter int lnbits = 5,                               // One line bits: log2(bytes_per_line)
    parameter int flbits = 4,                               // total flags number saved with address tag
    parameter int snoop = 0                                 // 0 Snoop port disabled; 1 Enabled (L2 caching)
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_direct_access,                            // lsb bits of address forms way index to access
    input logic i_invalidate,
    input logic i_re,
    input logic i_we,
    input logic [abus-1:0] i_addr,
    input logic [(8 * (2**lnbits))-1:0] i_wdata,
    input logic [(2**lnbits)-1:0] i_wstrb,
    input logic [flbits-1:0] i_wflags,
    output logic [abus-1:0] o_raddr,
    output logic [(8 * (2**lnbits))-1:0] o_rdata,
    output logic [flbits-1:0] o_rflags,
    output logic o_hit,
    // L2 snoop port, active when snoop = 1
    input logic [abus-1:0] i_snoop_addr,
    output logic o_snoop_ready,                             // single port memory not used for writing
    output logic [flbits-1:0] o_snoop_flags
);

localparam int FL_VALID = 0;

localparam int NWAYS = (2**waybits);

typedef struct {
    logic [abus-1:0] addr;
    logic [(2**lnbits)-1:0] wstrb;
    logic [(8 * (2**lnbits))-1:0] wdata;
    logic [flbits-1:0] wflags;
    logic [abus-1:0] snoop_addr;
} WayInType;

typedef struct {
    logic [abus-1:0] raddr;
    logic [(8 * (2**lnbits))-1:0] rdata;
    logic [flbits-1:0] rflags;
    logic hit;
    logic [flbits-1:0] snoop_flags;
} WayOutType;


typedef struct {
    logic [abus-1:0] req_addr;
    logic direct_access;
    logic invalidate;
    logic re;
} TagMemNWay_registers;

const TagMemNWay_registers TagMemNWay_r_reset = '{
    '0,                                 // req_addr
    1'b0,                               // direct_access
    1'b0,                               // invalidate
    1'b0                                // re
};

logic w_lrui_init;
logic [ibits-1:0] wb_lrui_raddr;
logic [ibits-1:0] wb_lrui_waddr;
logic w_lrui_up;
logic w_lrui_down;
logic [waybits-1:0] wb_lrui_lru;
logic [waybits-1:0] wb_lruo_lru;
WayInType way_i[0: NWAYS - 1];
WayOutType way_o[0: NWAYS - 1];
TagMemNWay_registers r, rin;

for (genvar i = 0; i < NWAYS; i++) begin: waygen
    TagMem #(
        .async_reset(async_reset),
        .abus(abus),
        .ibits(ibits),
        .lnbits(lnbits),
        .flbits(flbits),
        .snoop(snoop)
    ) wayx (
        .i_clk(i_clk),
        .i_nrst(i_nrst),
        .i_addr(way_i[i].addr),
        .i_wstrb(way_i[i].wstrb),
        .i_wdata(way_i[i].wdata),
        .i_wflags(way_i[i].wflags),
        .o_raddr(way_o[i].raddr),
        .o_rdata(way_o[i].rdata),
        .o_rflags(way_o[i].rflags),
        .o_hit(way_o[i].hit),
        .i_snoop_addr(way_i[i].snoop_addr),
        .o_snoop_flags(way_o[i].snoop_flags)
    );

end: waygen

lrunway #(
    .abits(ibits),
    .waybits(waybits)
) lru0 (
    .i_clk(i_clk),
    .i_init(w_lrui_init),
    .i_raddr(wb_lrui_raddr),
    .i_waddr(wb_lrui_waddr),
    .i_up(w_lrui_up),
    .i_down(w_lrui_down),
    .i_lru(wb_lrui_lru),
    .o_lru(wb_lruo_lru)
);


always_comb
begin: comb_proc
    TagMemNWay_registers v;
    logic [abus-1:0] vb_raddr;
    logic [(8 * (2**lnbits))-1:0] vb_rdata;
    logic [flbits-1:0] vb_rflags;
    logic v_hit;
    logic [waybits-1:0] vb_hit_idx;
    logic v_way_we;
    logic [(2**lnbits)-1:0] vb_wstrb;
    logic [flbits-1:0] vb_wflags;
    logic v_snoop_ready;
    logic [flbits-1:0] vb_snoop_flags;

    vb_raddr = 0;
    vb_rdata = 0;
    vb_rflags = 0;
    v_hit = 0;
    vb_hit_idx = 0;
    v_way_we = 0;
    vb_wstrb = 0;
    vb_wflags = 0;
    v_snoop_ready = 0;
    vb_snoop_flags = 0;

    v = r;


    v.direct_access = i_direct_access;
    v.invalidate = i_invalidate;
    v.re = i_re;
    v.req_addr = i_addr;

    vb_hit_idx = wb_lruo_lru;
    if (r.direct_access == 1'b1) begin
        vb_hit_idx = r.req_addr[(waybits - 1): 0];
    end else begin
        for (int i = 0; i < NWAYS; i++) begin
            if (way_o[i].hit == 1'b1) begin
                vb_hit_idx = i;
            end
        end
    end
    vb_raddr = way_o[int'(vb_hit_idx)].raddr;
    vb_rdata = way_o[int'(vb_hit_idx)].rdata;
    vb_rflags = way_o[int'(vb_hit_idx)].rflags;
    v_hit = way_o[int'(vb_hit_idx)].hit;

    if (r.invalidate == 1'b1) begin
        vb_wflags = '0;
        vb_wstrb = '1;
    end else begin
        vb_wflags = i_wflags;
        vb_wstrb = i_wstrb;
    end

    //     Warning: we can write only into previously read line,
    //                 if the previuosly read line is hit and contains valid flags
    //                 HIGH we modify it. Otherwise, we write into displacing line.

    for (int i = 0; i < NWAYS; i++) begin
        way_i[i].addr = i_addr;
        way_i[i].wdata = i_wdata;
        way_i[i].wstrb = '0;
        way_i[i].wflags = vb_wflags;
        way_i[i].snoop_addr = i_snoop_addr;
    end

    v_way_we = (i_we || (r.invalidate && v_hit));
    if (v_way_we == 1'b1) begin
        way_i[int'(vb_hit_idx)].wstrb = vb_wstrb;
    end

    v_snoop_ready = 1'b1;
    if (snoop == 1) begin
        for (int i = 0; i < NWAYS; i++) begin
            // tagmem already cleared snoop flags if there's no snoop hit
            if (way_o[i].snoop_flags[FL_VALID] == 1'b1) begin
                vb_snoop_flags = way_o[i].snoop_flags;
            end
        end
        // Writing into snoop tag memory, output value won't be valid on next clock
        if (v_way_we == 1'b1) begin
            v_snoop_ready = 1'b0;
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = TagMemNWay_r_reset;
    end

    w_lrui_init = r.direct_access;
    wb_lrui_raddr = i_addr[((ibits + lnbits) - 1): lnbits];
    wb_lrui_waddr = r.req_addr[((ibits + lnbits) - 1): lnbits];
    w_lrui_up = (i_we || (v_hit && r.re));
    w_lrui_down = (v_hit && r.invalidate);
    wb_lrui_lru = vb_hit_idx;

    o_raddr = vb_raddr;
    o_rdata = vb_rdata;
    o_rflags = vb_rflags;
    o_hit = v_hit;
    o_snoop_ready = v_snoop_ready;
    o_snoop_flags = vb_snoop_flags;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= TagMemNWay_r_reset;
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

endmodule: TagMemNWay
