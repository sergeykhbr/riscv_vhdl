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

module axi2apb #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // Base address information from the interconnect port
    output types_amba_pkg::dev_config_type o_cfg,           // Slave config descriptor
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // AXI4 Interconnect Bridge interface
    output types_amba_pkg::axi4_slave_out_type o_xslvo,     // AXI4 Bridge to Interconnect interface
    input types_amba_pkg::apb_out_type i_apbmi,             // APB Slave to Bridge master-in/slave-out interface
    output types_amba_pkg::apb_in_type o_apbmo              // APB Bridge to master-out/slave-in interface
);

import types_amba_pkg::*;
import axi2apb_pkg::*;

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;
logic w_req_ready;
axi2apb_registers r, rin;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_AXI2APB_BRIDGE)
) axi0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(i_xslvi),
    .o_xslvo(o_xslvo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(w_req_ready),
    .i_resp_valid(r.pvalid),
    .i_resp_rdata(r.prdata),
    .i_resp_err(r.pslverr)
);


always_comb
begin: comb_proc
    axi2apb_registers v;
    apb_in_type vapbmo;

    vapbmo = apb_in_none;

    v = r;

    w_req_ready = 1'b0;
    v.pvalid = 1'b0;

    case (r.state)
    State_Idle: begin
        w_req_ready = 1'b1;
        v.pslverr = 1'b0;
        v.penable = 1'b0;
        v.pselx = 1'b0;
        v.xsize = '0;
        if (w_req_valid == 1'b1) begin
            v.pwrite = w_req_write;
            v.pselx = 1'b1;
            v.paddr = {wb_req_addr[31: 2], 2'h0};
            v.pprot = '0;
            v.pwdata = wb_req_wdata;
            v.pstrb = wb_req_wstrb;
            v.state = State_setup;
            v.xsize = (&wb_req_wstrb);
            if (w_req_last == 1'b0) begin
                v.state = State_out;                        // Burst is not supported
                v.pselx = 1'b0;
                v.pslverr = 1'b1;
                v.prdata = '1;
            end
        end
    end
    State_setup: begin
        v.penable = 1'b1;
        v.state = State_access;
    end
    State_access: begin
        v.pslverr = i_apbmi.pslverr;
        if (i_apbmi.pready == 1'b1) begin
            v.penable = 1'b0;
            v.pselx = 1'b0;
            v.pwrite = 1'b0;
            if (r.paddr[2] == 1'b0) begin
                v.prdata = {r.prdata[63: 32], i_apbmi.prdata};
            end else begin
                v.prdata = {i_apbmi.prdata, r.prdata[31: 0]};
            end
            if ((|r.xsize) == 1'b1) begin
                v.xsize = (r.xsize - 1);
                v.paddr = (r.paddr + 4);
                v.state = State_setup;
            end else begin
                v.pvalid = 1'b1;
                v.state = State_out;
            end
        end
    end
    State_out: begin
        v.pvalid = 1'b0;
        v.pslverr = 1'b0;
        v.state = State_Idle;
    end
    default: begin
    end
    endcase

    vapbmo.paddr = r.paddr;
    vapbmo.pwrite = r.pwrite;
    if (r.paddr[2] == 1'b0) begin
        vapbmo.pwdata = r.pwdata[31: 0];
        vapbmo.pstrb = r.pstrb[3: 0];
    end else begin
        vapbmo.pwdata = r.pwdata[63: 32];
        vapbmo.pstrb = r.pstrb[7: 4];
    end
    vapbmo.pselx = r.pselx;
    vapbmo.penable = r.penable;
    vapbmo.pprot = r.pprot;

    if (~async_reset && i_nrst == 1'b0) begin
        v = axi2apb_r_reset;
    end

    o_apbmo = vapbmo;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= axi2apb_r_reset;
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

endmodule: axi2apb
