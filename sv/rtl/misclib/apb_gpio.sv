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

module apb_gpio #(
    parameter bit async_reset = 1'b0,
    parameter int width = 12
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_amba_pkg::dev_config_type o_cfg,           // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    input logic [width-1:0] i_gpio,
    output logic [width-1:0] o_gpio_dir,                    // 1 as input; 0 as output
    output logic [width-1:0] o_gpio,
    output logic [width-1:0] o_irq
);

import types_amba_pkg::*;
typedef struct {
    logic [width-1:0] input_val;
    logic [width-1:0] input_en;
    logic [width-1:0] output_en;
    logic [width-1:0] output_val;
    logic [width-1:0] ie;
    logic [width-1:0] ip;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_gpio_registers;

const apb_gpio_registers apb_gpio_r_reset = '{
    '0,                                 // input_val
    '1,                                 // input_en
    '0,                                 // output_en
    '0,                                 // output_val
    '0,                                 // ie
    '0,                                 // ip
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
apb_gpio_registers r, rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_GPIO)
) pslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .i_resp_valid(r.resp_valid),
    .i_resp_rdata(r.resp_rdata),
    .i_resp_err(r.resp_err)
);


always_comb
begin: comb_proc
    apb_gpio_registers v;
    logic [31:0] vb_rdata;

    vb_rdata = 0;

    v = r;

    v.input_val = (i_gpio & r.input_en);

    // Registers access:
    case (wb_req_addr[11: 2])
    10'h000: begin                                          // 0x00: RO input_val
        vb_rdata[(width - 1): 0] = r.input_val;
    end
    10'h001: begin                                          // 0x04: input_en
        vb_rdata[(width - 1): 0] = r.input_en;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.input_en = wb_req_wdata[(width - 1): 0];
        end
    end
    10'h002: begin                                          // 0x08: output_en
        vb_rdata[(width - 1): 0] = r.output_en;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.output_en = wb_req_wdata[(width - 1): 0];
        end
    end
    10'h003: begin                                          // 0x0C: output_val
        vb_rdata[(width - 1): 0] = r.output_val;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.output_val = wb_req_wdata[(width - 1): 0];
        end
    end
    10'h004: begin                                          // 0x10: ie
        vb_rdata[(width - 1): 0] = r.ie;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.ie = wb_req_wdata[(width - 1): 0];
        end
    end
    10'h005: begin                                          // 0x14: ip
        vb_rdata[(width - 1): 0] = r.ip;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.ip = wb_req_wdata[(width - 1): 0];
        end
    end
    default: begin
    end
    endcase

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if (~async_reset && i_nrst == 1'b0) begin
        v = apb_gpio_r_reset;
    end

    o_gpio_dir = r.input_en;
    o_gpio = r.output_val;
    o_irq = (r.ie & r.ip);

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= apb_gpio_r_reset;
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

endmodule: apb_gpio
