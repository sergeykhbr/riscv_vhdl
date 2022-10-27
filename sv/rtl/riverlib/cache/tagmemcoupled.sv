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

module TagMemCoupled #(
    parameter bit async_reset = 1'b0,
    parameter int abus = 64,                                // system bus address width (64 or 32 bits)
    parameter int waybits = 2,                              // log2 of number of ways bits (2 for 4 ways cache; 3 for 8 ways)
    parameter int ibits = 6,                                // lines memory address width (usually 6..8)
    parameter int lnbits = 5,                               // One line bits: log2(bytes_per_line)
    parameter int flbits = 4                                // total flags number saved with address tag
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_direct_access,
    input logic i_invalidate,
    input logic i_re,
    input logic i_we,
    input logic [abus-1:0] i_addr,
    input logic [(8 * (2**lnbits))-1:0] i_wdata,
    input logic [(2**lnbits)-1:0] i_wstrb,
    input logic [flbits-1:0] i_wflags,
    output logic [abus-1:0] o_raddr,
    output logic [((8 * (2**lnbits)) + 32)-1:0] o_rdata,
    output logic [flbits-1:0] o_rflags,
    output logic o_hit,
    output logic o_hit_next
);

localparam int EVEN = 0;
localparam int ODD = 1;
localparam int MemTotal = 2;

localparam int LINE_SZ = (2**lnbits);
localparam int TAG_START = (abus - (ibits + lnbits));

typedef struct {
    logic direct_access;
    logic invalidate;
    logic re;
    logic we;
    logic [abus-1:0] addr;
    logic [(8 * (2**lnbits))-1:0] wdata;
    logic [(2**lnbits)-1:0] wstrb;
    logic [flbits-1:0] wflags;
    logic [abus-1:0] snoop_addr;
} tagmem_in_type;

typedef struct {
    logic [abus-1:0] raddr;
    logic [(8 * (2**lnbits))-1:0] rdata;
    logic [flbits-1:0] rflags;
    logic hit;
    logic snoop_ready;
    logic [flbits-1:0] snoop_flags;
} tagmem_out_type;


typedef struct {
    logic [abus-1:0] req_addr;
} TagMemCoupled_registers;

const TagMemCoupled_registers TagMemCoupled_r_reset = '{
    '0                                  // req_addr
};

tagmem_in_type linei[0: MemTotal - 1];
tagmem_out_type lineo[0: MemTotal - 1];
TagMemCoupled_registers r, rin;

for (genvar i = 0; i < MemTotal; i++) begin: memgen
    TagMemNWay #(
        .async_reset(async_reset),
        .abus(abus),
        .waybits(waybits),
        .ibits((ibits - 1)),
        .lnbits(lnbits),
        .flbits(flbits),
        .snoop(0)
    ) memx (
        .i_clk(i_clk),
        .i_nrst(i_nrst),
        .i_direct_access(linei[i].direct_access),
        .i_invalidate(linei[i].invalidate),
        .i_re(linei[i].re),
        .i_we(linei[i].we),
        .i_addr(linei[i].addr),
        .i_wdata(linei[i].wdata),
        .i_wstrb(linei[i].wstrb),
        .i_wflags(linei[i].wflags),
        .o_raddr(lineo[i].raddr),
        .o_rdata(lineo[i].rdata),
        .o_rflags(lineo[i].rflags),
        .o_hit(lineo[i].hit),
        .i_snoop_addr(linei[i].snoop_addr),
        .o_snoop_ready(lineo[i].snoop_ready),
        .o_snoop_flags(lineo[i].snoop_flags)
    );

end: memgen

always_comb
begin: comb_proc
    TagMemCoupled_registers v;
    logic v_addr_sel;
    logic v_addr_sel_r;
    logic v_use_overlay;
    logic v_use_overlay_r;
    logic [ibits-1:0] vb_index;
    logic [ibits-1:0] vb_index_next;
    logic [abus-1:0] vb_addr_next;
    logic [abus-1:0] vb_addr_tag_direct;
    logic [abus-1:0] vb_addr_tag_next;
    logic [abus-1:0] vb_raddr_tag;
    logic [abus-1:0] vb_o_raddr;
    logic [((8 * (2**lnbits)) + 32)-1:0] vb_o_rdata;
    logic v_o_hit;
    logic v_o_hit_next;
    logic [flbits-1:0] vb_o_rflags;

    v_addr_sel = 0;
    v_addr_sel_r = 0;
    v_use_overlay = 0;
    v_use_overlay_r = 0;
    vb_index = 0;
    vb_index_next = 0;
    vb_addr_next = 0;
    vb_addr_tag_direct = 0;
    vb_addr_tag_next = 0;
    vb_raddr_tag = 0;
    vb_o_raddr = 0;
    vb_o_rdata = 0;
    v_o_hit = 0;
    v_o_hit_next = 0;
    vb_o_rflags = 0;

    v = r;


    v.req_addr = i_addr;
    v_addr_sel = i_addr[lnbits];
    v_addr_sel_r = r.req_addr[lnbits];

    vb_addr_next = (i_addr + LINE_SZ);

    vb_index = i_addr[((ibits + lnbits) - 1): lnbits];
    vb_index_next = vb_addr_next[((ibits + lnbits) - 1): lnbits];

    if ((&i_addr[(lnbits - 1): 2]) == 1'b1) begin
        v_use_overlay = 1'b1;
    end
    if ((&r.req_addr[(lnbits - 1): 2]) == 1'b1) begin
        v_use_overlay_r = 1'b1;
    end

    // Change the bit order in the requested address:
    //    [tag][line_idx][odd/evenbit][line_bytes] on
    //    [tag][1'b0]    [line_idx]   [line_bytes]
    //  
    // Example (abus=32; ibits=7; lnbits=5;):
    //   [4:0]   byte in line           [4:0]
    //   [11:5]  line index             {[1'b0],[11:6]}
    //   [31:12] tag                    [31:12]
    vb_addr_tag_direct = i_addr;
    vb_addr_tag_direct[((ibits + lnbits) - 1): lnbits] = {'0, vb_index[ibits - 1: 1]};

    vb_addr_tag_next = vb_addr_next;
    vb_addr_tag_next[((ibits + lnbits) - 1): lnbits] = {'0, vb_index_next[ibits - 1: 1]};

    if (v_addr_sel == 1'b0) begin
        linei[EVEN].addr = vb_addr_tag_direct;
        linei[EVEN].wstrb = i_wstrb;
        linei[ODD].addr = vb_addr_tag_next;
        linei[ODD].wstrb = '0;
    end else begin
        linei[EVEN].addr = vb_addr_tag_next;
        linei[EVEN].wstrb = '0;
        linei[ODD].addr = vb_addr_tag_direct;
        linei[ODD].wstrb = i_wstrb;
    end

    linei[EVEN].direct_access = (i_direct_access && ((~v_addr_sel) || v_use_overlay));
    linei[ODD].direct_access = (i_direct_access && (v_addr_sel || v_use_overlay));

    linei[EVEN].invalidate = (i_invalidate && ((~v_addr_sel) || v_use_overlay));
    linei[ODD].invalidate = (i_invalidate && (v_addr_sel || v_use_overlay));

    linei[EVEN].re = (i_re && ((~v_addr_sel) || v_use_overlay));
    linei[ODD].re = (i_re && (v_addr_sel || v_use_overlay));

    linei[EVEN].we = (i_we && ((~v_addr_sel) || v_use_overlay));
    linei[ODD].we = (i_we && (v_addr_sel || v_use_overlay));

    linei[EVEN].wdata = i_wdata;
    linei[ODD].wdata = i_wdata;

    linei[EVEN].wflags = i_wflags;
    linei[ODD].wflags = i_wflags;

    // Form output:
    if (v_addr_sel_r == 1'b0) begin
        vb_o_rdata = {lineo[ODD].rdata[31: 0], lineo[EVEN].rdata};
        vb_raddr_tag = lineo[EVEN].raddr;
        vb_o_rflags = lineo[EVEN].rflags;

        v_o_hit = lineo[EVEN].hit;
        if (v_use_overlay_r == 1'b0) begin
            v_o_hit_next = lineo[EVEN].hit;
        end else begin
            v_o_hit_next = lineo[ODD].hit;
        end
    end else begin
        vb_o_rdata = {lineo[EVEN].rdata[31: 0], lineo[ODD].rdata};
        vb_raddr_tag = lineo[ODD].raddr;
        vb_o_rflags = lineo[ODD].rflags;

        v_o_hit = lineo[ODD].hit;
        if (v_use_overlay_r == 1'b0) begin
            v_o_hit_next = lineo[ODD].hit;
        end else begin
            v_o_hit_next = lineo[EVEN].hit;
        end
    end

    vb_o_raddr = vb_raddr_tag;
    vb_o_raddr[lnbits] = v_addr_sel_r;
    vb_o_raddr[((ibits + lnbits) - 1): (lnbits + 1)] = vb_raddr_tag[((ibits + lnbits) - 2): lnbits];

    o_raddr = vb_o_raddr;
    o_rdata = vb_o_rdata;
    o_rflags = vb_o_rflags;
    o_hit = v_o_hit;
    o_hit_next = v_o_hit_next;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= TagMemCoupled_r_reset;
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

endmodule: TagMemCoupled
