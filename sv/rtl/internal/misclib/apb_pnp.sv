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

module apb_pnp #(
    parameter int cfg_slots = 1,
    parameter logic async_reset = 1'b0,
    parameter logic [31:0] hwid = 32'h20221123,
    parameter int cpu_max = 1,
    parameter int l2cache_ena = 1,
    parameter int plic_irq_max = 127
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    input types_pnp_pkg::soc_pnp_vector i_cfg,              // Device descriptors vector
    output types_pnp_pkg::dev_config_type o_cfg,            // PNP Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB  Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    output logic o_irq
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
typedef struct {
    logic [31:0] fw_id;
    logic [31:0] idt_l;
    logic [31:0] idt_m;
    logic [31:0] malloc_addr_l;
    logic [31:0] malloc_addr_m;
    logic [31:0] malloc_size_l;
    logic [31:0] malloc_size_m;
    logic [31:0] fwdbg1;
    logic [31:0] fwdbg2;
    logic [31:0] fwdbg3;
    logic [31:0] fwdbg4;
    logic [31:0] fwdbg5;
    logic [31:0] fwdbg6;
    logic irq;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_pnp_registers;

const apb_pnp_registers apb_pnp_r_reset = '{
    '0,                                 // fw_id
    '0,                                 // idt_l
    '0,                                 // idt_m
    '0,                                 // malloc_addr_l
    '0,                                 // malloc_addr_m
    '0,                                 // malloc_size_l
    '0,                                 // malloc_size_m
    '0,                                 // fwdbg1
    '0,                                 // fwdbg2
    '0,                                 // fwdbg3
    '0,                                 // fwdbg4
    '0,                                 // fwdbg5
    '0,                                 // fwdbg6
    1'b0,                               // irq
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};
logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
apb_pnp_registers r;
apb_pnp_registers rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_PNP)
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
    apb_pnp_registers v;
    logic [31:0] cfgmap[0: (8 * cfg_slots)-1];
    logic [31:0] vrdata;

    v = r;
    for (int i = 0; i < (8 * cfg_slots); i++) begin
        cfgmap[i] = '0;
    end
    vrdata = '0;

    v.irq = 1'b0;

    for (int i = 0; i < cfg_slots; i++) begin
        cfgmap[(8 * i)] = {22'd0, i_cfg[i].descrtype, i_cfg[i].descrsize};
        cfgmap[((8 * i) + 1)] = {i_cfg[i].vid, i_cfg[i].did};
        cfgmap[((8 * i) + 4)] = i_cfg[i].addr_start[31: 0];
        cfgmap[((8 * i) + 5)] = i_cfg[i].addr_start[63: 32];
        cfgmap[((8 * i) + 6)] = i_cfg[i].addr_end[31: 0];
        cfgmap[((8 * i) + 7)] = i_cfg[i].addr_end[63: 32];
    end

    if (wb_req_addr[11: 2] == 9'd0) begin
        vrdata = hwid;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.irq = 1'b1;
        end
    end else if (wb_req_addr[11: 2] == 9'd1) begin
        vrdata = r.fw_id;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fw_id = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd2) begin
        vrdata[31: 28] = cpu_max[3: 0];
        vrdata[24] = l2cache_ena;
        vrdata[15: 8] = cfg_slots[7: 0];
        vrdata[7: 0] = plic_irq_max[7: 0];
    end else if (wb_req_addr[11: 2] == 9'd3) begin
        vrdata = 32'd0;
    end else if (wb_req_addr[11: 2] == 9'd4) begin
        vrdata = r.idt_l;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.idt_l = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd5) begin
        vrdata = r.idt_m;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.idt_m = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd6) begin
        vrdata = r.malloc_addr_l;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_addr_l = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd7) begin
        vrdata = r.malloc_addr_m;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_addr_m = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd8) begin
        vrdata = r.malloc_size_l;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_size_l = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd9) begin
        vrdata = r.malloc_size_m;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_size_m = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd10) begin
        vrdata = r.fwdbg1;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg1 = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd11) begin
        vrdata = r.fwdbg2;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg2 = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd12) begin
        vrdata = r.fwdbg3;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg3 = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd13) begin
        vrdata = r.fwdbg4;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg4 = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd14) begin
        vrdata = r.fwdbg5;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg5 = wb_req_wdata;
        end
    end else if (wb_req_addr[11: 2] == 9'd15) begin
        vrdata = r.fwdbg6;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg6 = wb_req_wdata;
        end
    end else if ((wb_req_addr[11: 2] >= 9'd16)
                && (wb_req_addr[11: 2] < (16 + (8 * cfg_slots)))) begin
        vrdata = cfgmap[(int'(wb_req_addr[11: 2]) - 16)];
    end

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = apb_pnp_r_reset;
    end

    v.resp_valid = w_req_valid;
    v.resp_rdata = vrdata;
    v.resp_err = 1'b0;
    o_irq = r.irq;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= apb_pnp_r_reset;
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

endmodule: apb_pnp
