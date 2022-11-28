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

module apb_slv #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned vid = 0,                         // Vendor ID
    parameter int unsigned did = 0                          // Device ID
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // Base address information from the interconnect port
    output types_amba_pkg::dev_config_type o_cfg,           // Slave config descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB  Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    output logic o_req_valid,
    output logic [31:0] o_req_addr,
    output logic o_req_write,
    output logic [31:0] o_req_wdata,
    input logic i_resp_valid,
    input logic [31:0] i_resp_rdata,
    input logic i_resp_err
);

import types_amba_pkg::*;
import apb_slv_pkg::*;

apb_slv_registers r, rin;

always_comb
begin: comb_proc
    apb_slv_registers v;
    logic [31:0] vb_rdata;
    dev_config_type vcfg;
    apb_out_type vapbo;

    vb_rdata = 0;
    vcfg = dev_config_none;
    vapbo = apb_out_none;

    v = r;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.addr_start;
    vcfg.addr_end = i_mapinfo.addr_end;
    vcfg.vid = vid;
    vcfg.did = did;

    v.req_valid = 1'b0;

    case (r.state)
    State_Idle: begin
        v.resp_valid = 1'b0;
        v.resp_err = 1'b0;
        if (i_apbi.pselx == 1'b1) begin
            v.state = State_Request;
            v.req_valid = 1'b1;
            v.req_addr = i_apbi.paddr;
            v.req_write = i_apbi.pwrite;
            v.req_wdata = i_apbi.pwdata;
        end
    end
    State_Request: begin
        // One clock wait state:
        v.state = State_WaitResp;
    end
    State_WaitResp: begin
        v.resp_valid = i_resp_valid;
        if (i_resp_valid == 1'b1) begin
            v.resp_rdata = i_resp_rdata;
            v.resp_err = i_resp_err;
            v.state = State_Resp;
        end
    end
    State_Resp: begin
        if (i_apbi.penable == 1'b1) begin
            v.state = State_Idle;
            v.resp_valid = 1'b0;
        end
    end
    default: begin
    end
    endcase

    if (~async_reset && i_nrst == 1'b0) begin
        v = apb_slv_r_reset;
    end

    o_req_valid = r.req_valid;
    o_req_addr = r.req_addr;
    o_req_write = r.req_write;
    o_req_wdata = r.req_wdata;

    vapbo.pready = r.resp_valid;
    vapbo.prdata = r.resp_rdata;
    vapbo.pslverr = r.resp_err;
    o_apbo = vapbo;
    o_cfg = vcfg;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= apb_slv_r_reset;
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

endmodule: apb_slv
