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

module clint #(
    parameter int cpu_total = 4,
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_pnp_pkg::dev_config_type o_cfg,            // Device descriptor
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // AXI Slave to Bridge interface
    output types_amba_pkg::axi4_slave_out_type o_xslvo,     // AXI Bridge to Slave interface
    output logic [63:0] o_mtimer,                           // Shadow read-only access from Harts
    output logic [cpu_total-1:0] o_msip,                    // Machine mode Softare Pending Interrupt
    output logic [cpu_total-1:0] o_mtip                     // Machine mode Timer Pending Interrupt
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
typedef struct {
    logic msip;
    logic mtip;
    logic [63:0] mtimecmp;
} clint_cpu_type;

typedef struct {
    logic [63:0] mtime;
    clint_cpu_type hart[0: cpu_total - 1];
    logic [63:0] rdata;
    logic resp_valid;
} clint_registers;

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic [7:0] wb_req_size;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;
logic w_req_ready;
logic w_resp_valid;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_resp_rdata;
logic wb_resp_err;
clint_registers r;
clint_registers rin;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_CLINT)
) xslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(i_xslvi),
    .o_xslvo(o_xslvo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_size(wb_req_size),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(w_req_ready),
    .i_resp_valid(w_resp_valid),
    .i_resp_rdata(wb_resp_rdata),
    .i_resp_err(wb_resp_err)
);

always_comb
begin: comb_proc
    clint_registers v;
    logic [CFG_SYSBUS_DATA_BITS-1:0] vrdata;
    logic [cpu_total-1:0] vb_msip;
    logic [cpu_total-1:0] vb_mtip;
    int regidx;

    v.mtime = r.mtime;
    for (int i = 0; i < cpu_total; i++) begin
        v.hart[i].msip = r.hart[i].msip;
        v.hart[i].mtip = r.hart[i].mtip;
        v.hart[i].mtimecmp = r.hart[i].mtimecmp;
    end
    v.rdata = r.rdata;
    v.resp_valid = r.resp_valid;
    vrdata = '0;
    vb_msip = '0;
    vb_mtip = '0;
    regidx = 0;

    v.mtime = (r.mtime + 1);
    regidx = int'(wb_req_addr[13: 3]);
    v.resp_valid = w_req_valid;

    for (int i = 0; i < cpu_total; i++) begin
        v.hart[i].mtip = 1'b0;
        if (r.mtime >= r.hart[i].mtimecmp) begin
            v.hart[i].mtip = 1'b1;
        end
    end

    case (wb_req_addr[15: 14])
    2'd0: begin
        vrdata[0] = r.hart[regidx].msip;
        vrdata[32] = r.hart[(regidx + 1)].msip;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            if ((|wb_req_wstrb[3: 0]) == 1'b1) begin
                v.hart[regidx].msip = wb_req_wdata[0];
            end
            if ((|wb_req_wstrb[7: 4]) == 1'b1) begin
                v.hart[(regidx + 1)].msip = wb_req_wdata[32];
            end
        end
    end
    2'd1: begin
        vrdata = r.hart[regidx].mtimecmp;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.hart[regidx].mtimecmp = wb_req_wdata;
        end
    end
    2'd2: begin
        if (wb_req_addr[13: 3] == 11'h7FF) begin
            vrdata = r.mtime;                               // [RO]
        end
    end
    default: begin
    end
    endcase
    v.rdata = vrdata;

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v.mtime = '0;
        for (int i = 0; i < cpu_total; i++) begin
            v.hart[i].msip = 1'b0;
            v.hart[i].mtip = 1'b0;
            v.hart[i].mtimecmp = 64'd0;
        end
        v.rdata = '0;
        v.resp_valid = 1'b0;
    end

    for (int i = 0; i < cpu_total; i++) begin
        vb_msip[i] = r.hart[i].msip;
        vb_mtip[i] = r.hart[i].mtip;
    end

    w_req_ready = 1'b1;
    w_resp_valid = r.resp_valid;
    wb_resp_rdata = r.rdata;
    wb_resp_err = 1'b0;
    o_msip = vb_msip;
    o_mtip = vb_mtip;
    o_mtimer = r.mtime;

    rin.mtime = v.mtime;
    for (int i = 0; i < cpu_total; i++) begin
        rin.hart[i].msip = v.hart[i].msip;
        rin.hart[i].mtip = v.hart[i].mtip;
        rin.hart[i].mtimecmp = v.hart[i].mtimecmp;
    end
    rin.rdata = v.rdata;
    rin.resp_valid = v.resp_valid;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r.mtime <= '0;
                for (int i = 0; i < cpu_total; i++) begin
                    r.hart[i].msip <= 1'b0;
                    r.hart[i].mtip <= 1'b0;
                    r.hart[i].mtimecmp <= 64'd0;
                end
                r.rdata <= '0;
                r.resp_valid <= 1'b0;
            end else begin
                r.mtime <= rin.mtime;
                for (int i = 0; i < cpu_total; i++) begin
                    r.hart[i].msip <= rin.hart[i].msip;
                    r.hart[i].mtip <= rin.hart[i].mtip;
                    r.hart[i].mtimecmp <= rin.hart[i].mtimecmp;
                end
                r.rdata <= rin.rdata;
                r.resp_valid <= rin.resp_valid;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r.mtime <= rin.mtime;
            for (int i = 0; i < cpu_total; i++) begin
                r.hart[i].msip <= rin.hart[i].msip;
                r.hart[i].mtip <= rin.hart[i].mtip;
                r.hart[i].mtimecmp <= rin.hart[i].mtimecmp;
            end
            r.rdata <= rin.rdata;
            r.resp_valid <= rin.resp_valid;
        end

    end: async_r_dis
endgenerate

endmodule: clint
