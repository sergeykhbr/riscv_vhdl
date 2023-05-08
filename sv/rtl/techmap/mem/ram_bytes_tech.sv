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

module ram_bytes_tech #(
    parameter int abits = 16,
    parameter int log2_dbytes = 3
)
(
    input logic i_clk,                                      // CPU clock
    input logic [abits-1:0] i_addr,
    input logic i_wena,
    input logic [(2**log2_dbytes)-1:0] i_wstrb,
    input logic [(8 * (2**log2_dbytes))-1:0] i_wdata,
    output logic [(8 * (2**log2_dbytes))-1:0] o_rdata
);

localparam int dbytes = (2**log2_dbytes);
localparam int dbits = (8 * (2**log2_dbytes));

logic [(abits - log2_dbytes)-1:0] wb_addr;
logic wb_wena[0: dbytes - 1];
logic [7:0] wb_wdata[0: dbytes - 1];
logic [7:0] wb_rdata[0: dbytes - 1];

for (genvar i = 0; i < dbytes; i++) begin: memgen
    ram_tech #(
        .abits((abits - log2_dbytes)),
        .dbits(8)
    ) mem (
        .i_clk(i_clk),
        .i_addr(wb_addr),
        .i_wena(wb_wena[i]),
        .i_wdata(wb_wdata[i]),
        .o_rdata(wb_rdata[i])
    );

end: memgen

always_comb
begin: comb_proc
    logic [dbits-1:0] vb_rdata;

    vb_rdata = 0;

    wb_addr = i_addr[(abits - 1): log2_dbytes];
    for (int i = 0; i < dbytes; i++) begin
        wb_wena[i] = (i_wena && i_wstrb[i]);
        wb_wdata[i] = i_wdata[(8 * i) +: 8];
        vb_rdata[(8 * i) +: 8] = wb_rdata[i];
    end

    o_rdata = vb_rdata;
end: comb_proc

endmodule: ram_bytes_tech
