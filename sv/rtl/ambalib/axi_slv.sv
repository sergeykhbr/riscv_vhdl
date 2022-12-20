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

module axi_slv #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned vid = 0,                         // Vendor ID
    parameter int unsigned did = 0                          // Device ID
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // Base address information from the interconnect port
    output types_amba_pkg::dev_config_type o_cfg,           // Slave config descriptor
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // AXI Slave input interface
    output types_amba_pkg::axi4_slave_out_type o_xslvo,     // AXI Slave output interface
    output logic o_req_valid,
    output logic [types_amba_pkg::CFG_SYSBUS_ADDR_BITS-1:0] o_req_addr,
    output logic [7:0] o_req_size,
    output logic o_req_write,
    output logic [types_amba_pkg::CFG_SYSBUS_DATA_BITS-1:0] o_req_wdata,
    output logic [types_amba_pkg::CFG_SYSBUS_DATA_BYTES-1:0] o_req_wstrb,
    output logic o_req_last,
    input logic i_req_ready,
    input logic i_resp_valid,
    input logic [types_amba_pkg::CFG_SYSBUS_DATA_BITS-1:0] i_resp_rdata,
    input logic i_resp_err
);

import types_amba_pkg::*;
import axi_slv_pkg::*;

axi_slv_registers r, rin;

always_comb
begin: comb_proc
    axi_slv_registers v;
    logic [11:0] vb_req_addr_next;
    logic v_req_last;
    dev_config_type vcfg;
    axi4_slave_out_type vxslvo;

    vb_req_addr_next = 0;
    v_req_last = 0;
    vcfg = dev_config_none;
    vxslvo = axi4_slave_out_none;

    v = r;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.addr_start;
    vcfg.addr_end = i_mapinfo.addr_end;
    vcfg.vid = vid;
    vcfg.did = did;

    vb_req_addr_next = (r.req_addr[11: 0] + r.req_xsize);
    if (r.req_burst == AXI_BURST_FIXED) begin
        vb_req_addr_next = r.req_addr[11: 0];
    end else if (r.req_burst == AXI_BURST_WRAP) begin
        // Wrap suppported only 2, 4, 8 or 16 Bytes. See ARMDeveloper spec.
        if (r.req_xsize == 2) begin
            vb_req_addr_next[11: 1] = r.req_addr[11: 1];
        end else if (r.req_xsize == 4) begin
            vb_req_addr_next[11: 2] = r.req_addr[11: 2];
        end else if (r.req_xsize == 8) begin
            vb_req_addr_next[11: 3] = r.req_addr[11: 3];
        end else if (r.req_xsize == 16) begin
            vb_req_addr_next[11: 4] = r.req_addr[11: 4];
        end else if (r.req_xsize == 32) begin
            // Optional (not in ARM spec)
            vb_req_addr_next[11: 5] = r.req_addr[11: 5];
        end
    end

    v_req_last = (~(|r.req_len));
    v.req_last = v_req_last;

    case (r.state)
    State_Idle: begin
        v.req_valid = 1'b0;
        v.req_write = 1'b0;
        v.resp_valid = 1'b0;
        v.resp_last = 1'b0;
        v.resp_err = 1'b0;
        vxslvo.aw_ready = 1'b1;
        vxslvo.w_ready = 1'b1;                              // No burst AXILite ready
        vxslvo.ar_ready = (~i_xslvi.aw_valid);
        if (i_xslvi.aw_valid == 1'b1) begin
            v.req_addr = i_xslvi.aw_bits.addr;
            v.req_xsize = XSizeToBytes(i_xslvi.aw_bits.size);
            v.req_len = i_xslvi.aw_bits.len;
            v.req_burst = i_xslvi.aw_bits.burst;
            v.req_id = i_xslvi.aw_id;
            v.req_user = i_xslvi.aw_user;
            v.req_wdata = i_xslvi.w_data;                   // AXI Lite compatible
            v.req_wstrb = i_xslvi.w_strb;
            if (i_xslvi.w_valid == 1'b1) begin
                // AXI Lite does not support burst transaction
                v.state = State_last_w;
                v.req_valid = 1'b1;
                v.req_write = 1'b1;
            end else begin
                v.state = State_w;
            end
        end else if (i_xslvi.ar_valid == 1'b1) begin
            v.req_valid = 1'b1;
            v.req_addr = i_xslvi.ar_bits.addr;
            v.req_xsize = XSizeToBytes(i_xslvi.ar_bits.size);
            v.req_len = i_xslvi.ar_bits.len;
            v.req_burst = i_xslvi.ar_bits.burst;
            v.req_id = i_xslvi.ar_id;
            v.req_user = i_xslvi.ar_user;
            v.state = State_addr_r;
        end
    end
    State_w: begin
        vxslvo.w_ready = 1'b1;
        v.req_wdata = i_xslvi.w_data;
        v.req_wstrb = i_xslvi.w_strb;
        if (i_xslvi.w_valid == 1'b1) begin
            v.req_valid = 1'b1;
            v.req_write = 1'b1;
            if ((|r.req_len) == 1'b1) begin
                v.state = State_burst_w;
            end else begin
                v.state = State_last_w;
            end
        end
    end
    State_burst_w: begin
        v.req_valid = i_xslvi.w_valid;
        vxslvo.w_ready = i_resp_valid;
        if ((i_xslvi.w_valid == 1'b1) && (i_resp_valid == 1'b1)) begin
            v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_req_addr_next};
            v.req_wdata = i_xslvi.w_data;
            v.req_wstrb = i_xslvi.w_strb;
            if ((|r.req_len) == 1'b1) begin
                v.req_len = (r.req_len - 1);
            end
            if (r.req_len == 8'h01) begin
                v.state = State_last_w;
            end
        end
    end
    State_last_w: begin
        // Wait cycle: w_ready is zero on the last write because it is laready accepted
        if (i_resp_valid == 1'b1) begin
            v.req_valid = 1'b0;
            v.req_write = 1'b0;
            v.resp_err = i_resp_err;
            v.state = State_b;
        end
    end
    State_b: begin
        vxslvo.b_valid = 1'b1;
        if (i_xslvi.b_ready == 1'b1) begin
            v.state = State_Idle;
        end
    end
    State_addr_r: begin
        // Setup address:
        if (i_req_ready == 1'b1) begin
            if ((|r.req_len) == 1'b1) begin
                v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_req_addr_next};
                v.req_len = (r.req_len - 1);
                v.state = State_addrdata_r;
            end else begin
                v.req_valid = 1'b0;
                v.state = State_data_r;
            end
        end
    end
    State_addrdata_r: begin
        v.resp_valid = i_resp_valid;
        v.resp_rdata = i_resp_rdata;
        v.resp_err = i_resp_err;
        if ((i_resp_valid == 1'b0) || ((|r.req_len) == 1'b0)) begin
            v.req_valid = 1'b0;
            v.state = State_data_r;
        end else if (i_xslvi.r_ready == 1'b0) begin
            // Bus is not ready to accept read data
            v.req_valid = 1'b0;
            v.state = State_out_r;
        end else if (i_req_ready == 1'b0) begin
            // Slave device is not ready to accept burst request
            v.state = State_addr_r;
        end else begin
            v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_req_addr_next};
            v.req_len = (r.req_len - 1);
        end
    end
    State_data_r: begin
        if (i_resp_valid == 1'b1) begin
            v.resp_valid = 1'b1;
            v.resp_rdata = i_resp_rdata;
            v.resp_err = i_resp_err;
            v.resp_last = (~(|r.req_len));
            v.state = State_out_r;
        end
    end
    State_out_r: begin
        if (i_xslvi.r_ready == 1'b1) begin
            v.resp_valid = 1'b0;
            v.resp_last = 1'b0;
            if ((|r.req_len) == 1'b1) begin
                v.req_valid = 1'b1;
                v.state = State_addr_r;
            end else begin
                v.state = State_Idle;
            end
        end
    end
    default: begin
    end
    endcase

    if (~async_reset && i_nrst == 1'b0) begin
        v = axi_slv_r_reset;
    end

    o_req_valid = r.req_valid;
    o_req_last = v_req_last;
    o_req_addr = r.req_addr;
    o_req_size = r.req_xsize;
    o_req_write = r.req_write;
    o_req_wdata = r.req_wdata;
    o_req_wstrb = r.req_wstrb;

    vxslvo.b_id = r.req_id;
    vxslvo.b_user = r.req_user;
    vxslvo.b_resp = {r.resp_err, 1'h0};
    vxslvo.r_valid = r.resp_valid;
    vxslvo.r_id = r.req_id;
    vxslvo.r_user = r.req_user;
    vxslvo.r_resp = {r.resp_err, 1'h0};
    vxslvo.r_data = r.resp_rdata;
    vxslvo.r_last = r.resp_last;
    o_xslvo = vxslvo;
    o_cfg = vcfg;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= axi_slv_r_reset;
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

endmodule: axi_slv
