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

module lrunway #(
    parameter int abits = 6,                                // Cache line address bus (usually 6..8)
    parameter int waybits = 2                               // Number of way bitwidth (=2 for 4 ways cache)
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_init,
    input logic [abits-1:0] i_raddr,
    input logic [abits-1:0] i_waddr,
    input logic i_up,
    input logic i_down,
    input logic [waybits-1:0] i_lru,
    output logic [waybits-1:0] o_lru
);

localparam int LINES_TOTAL = (2**abits);
localparam int WAYS_TOTAL = (2**waybits);
localparam int LINE_WIDTH = (WAYS_TOTAL * waybits);

typedef struct {
    logic [abits-1:0] radr;
    logic [LINE_WIDTH-1:0] mem[0: LINES_TOTAL - 1];
} lrunway_registers;

lrunway_registers r, rin;

always_comb
begin: comb_proc
    lrunway_registers v;
    logic [LINE_WIDTH-1:0] wb_tbl_rdata;
    logic [abits-1:0] vb_tbl_wadr;
    logic [LINE_WIDTH-1:0] vb_tbl_wdata_init;
    logic [LINE_WIDTH-1:0] vb_tbl_wdata_up;
    logic [LINE_WIDTH-1:0] vb_tbl_wdata_down;
    logic [LINE_WIDTH-1:0] vb_tbl_wdata;
    logic v_we;
    logic shift_ena_up;
    logic shift_ena_down;

    wb_tbl_rdata = 0;
    vb_tbl_wadr = 0;
    vb_tbl_wdata_init = 0;
    vb_tbl_wdata_up = 0;
    vb_tbl_wdata_down = 0;
    vb_tbl_wdata = 0;
    v_we = 0;
    shift_ena_up = 0;
    shift_ena_down = 0;

    v.radr = r.radr;
    for (int i = 0; i < LINES_TOTAL; i++) begin
        v.mem[i] = r.mem[i];
    end

    v.radr = i_raddr;
    wb_tbl_rdata = r.mem[int'(r.radr)];

    v_we = (i_up || i_down || i_init);

    // init table value
    for (int i = 0; i < WAYS_TOTAL; i++) begin
        vb_tbl_wdata_init[(i * waybits) +: waybits] = i;
    end

    // LRU next value, last used goes on top
    vb_tbl_wdata_up = wb_tbl_rdata;
    if (wb_tbl_rdata[(LINE_WIDTH - waybits) +: waybits] != i_lru) begin
        vb_tbl_wdata_up[(LINE_WIDTH - waybits) +: waybits] = i_lru;
        shift_ena_up = 1'b1;

        for (int i = (WAYS_TOTAL - 2); i >= 0; i--) begin
            if (shift_ena_up == 1'b1) begin
                vb_tbl_wdata_up[(i * waybits) +: waybits] = wb_tbl_rdata[((i + 1) * waybits) +: waybits];
                if (wb_tbl_rdata[(i * waybits) +: waybits] == i_lru) begin
                    shift_ena_up = 1'b0;
                end
            end
        end
    end

    // LRU next value when invalidate, marked as 'invalid' goes down
    vb_tbl_wdata_down = wb_tbl_rdata;
    if (wb_tbl_rdata[(waybits - 1): 0] != i_lru) begin
        vb_tbl_wdata_down[(waybits - 1): 0] = i_lru;
        shift_ena_down = 1'b1;

        for (int i = 1; i < WAYS_TOTAL; i++) begin
            if (shift_ena_down == 1'b1) begin
                vb_tbl_wdata_down[(i * waybits) +: waybits] = wb_tbl_rdata[((i - 1) * waybits) +: waybits];
                if (wb_tbl_rdata[(i * waybits) +: waybits] == i_lru) begin
                    shift_ena_down = 1'b0;
                end
            end
        end
    end

    if (i_init == 1'b1) begin
        vb_tbl_wdata = vb_tbl_wdata_init;
    end else if (i_up == 1'b1) begin
        vb_tbl_wdata = vb_tbl_wdata_up;
    end else if (i_down == 1'b1) begin
        vb_tbl_wdata = vb_tbl_wdata_down;
    end else begin
        vb_tbl_wdata = '0;
    end

    if (v_we == 1'b1) begin
        v.mem[int'(i_waddr)] = vb_tbl_wdata;
    end
    o_lru = wb_tbl_rdata[(waybits - 1): 0];

    rin.radr = v.radr;
    for (int i = 0; i < LINES_TOTAL; i++) begin
        rin.mem[i] = v.mem[i];
    end
end: comb_proc

always_ff @(posedge i_clk) begin: rg_proc
    r.radr <= rin.radr;
    for (int i = 0; i < LINES_TOTAL; i++) begin
        r.mem[i] <= rin.mem[i];
    end
end: rg_proc

endmodule: lrunway
