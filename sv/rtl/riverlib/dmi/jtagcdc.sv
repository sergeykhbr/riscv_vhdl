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

module jtagcdc #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,
    input logic i_nrst,                                     // full reset including dmi (usually via reset button)
    // tck clock
    input logic i_dmi_req_valid,
    input logic i_dmi_req_write,
    input logic [6:0] i_dmi_req_addr,
    input logic [31:0] i_dmi_req_data,
    input logic i_dmi_reset,
    input logic i_dmi_hardreset,
    // system clock
    input logic i_dmi_req_ready,
    output logic o_dmi_req_valid,
    output logic o_dmi_req_write,
    output logic [6:0] o_dmi_req_addr,
    output logic [31:0] o_dmi_req_data,
    output logic o_dmi_reset,
    output logic o_dmi_hardreset
);

import jtagcdc_pkg::*;

jtagcdc_registers r, rin;

always_comb
begin: comb_proc
    jtagcdc_registers v;
    logic [CDC_REG_WIDTH-1:0] vb_bus;

    vb_bus = 0;

    v = r;

    vb_bus = {i_dmi_hardreset,
            i_dmi_reset,
            i_dmi_req_addr,
            i_dmi_req_data,
            i_dmi_req_write,
            i_dmi_req_valid};

    v.l1 = vb_bus;
    v.l2 = r.l1;
    if ((r.l2[0] && (~r.req_valid) && (~r.req_accepted)) == 1'b1) begin
        // To avoid request repeading
        v.req_valid = 1'b1;
        v.req_write = r.l2[1];
        v.req_data = r.l2[33: 2];
        v.req_addr = r.l2[40: 34];
        v.req_reset = r.l2[41];
        v.req_hardreset = r.l2[42];
    end else if (i_dmi_req_ready == 1'b1) begin
        v.req_valid = 1'b0;
    end
    if ((r.l2[0] && r.req_valid && i_dmi_req_ready) == 1'b1) begin
        v.req_accepted = 1'b1;
    end else if (r.l2[0] == 1'b0) begin
        v.req_accepted = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = jtagcdc_r_reset;
    end

    o_dmi_req_valid = r.req_valid;
    o_dmi_req_write = r.req_write;
    o_dmi_req_data = r.req_data;
    o_dmi_req_addr = r.req_addr;
    o_dmi_reset = r.req_reset;
    o_dmi_hardreset = r.req_hardreset;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= jtagcdc_r_reset;
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

endmodule: jtagcdc
