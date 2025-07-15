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

module gencpu64_axictrl_bus0 #(
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // Bus clock
    input logic i_nrst,                                     // Reset: active LOW
    output types_pnp_pkg::dev_config_type o_cfg,            // Slave config descriptor
    input types_gencpu64_bus0_pkg::bus0_xmst_out_vector i_xmsto,// AXI4 masters output vector
    output types_gencpu64_bus0_pkg::bus0_xmst_in_vector o_xmsti,// AXI4 masters input vector
    input types_gencpu64_bus0_pkg::bus0_xslv_out_vector i_xslvo,// AXI4 slaves output vectors
    output types_gencpu64_bus0_pkg::bus0_xslv_in_vector o_xslvi,// AXI4 slaves input vectors
    output types_gencpu64_bus0_pkg::bus0_mapinfo_vector o_mapinfo// AXI devices memory mapping information
);

import types_pnp_pkg::*;
import types_amba_pkg::*;
import types_gencpu64_bus0_pkg::*;
import gencpu64_axictrl_bus0_pkg::*;

axi4_slave_in_type wb_def_xslvi;
axi4_slave_out_type wb_def_xslvo;
mapinfo_type wb_def_mapinfo;
logic w_def_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_def_req_addr;
logic [7:0] wb_def_req_size;
logic w_def_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_def_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_def_req_wstrb;
logic w_def_req_last;
logic w_def_req_ready;
logic w_def_resp_valid;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_def_resp_rdata;
logic w_def_resp_err;
gencpu64_axictrl_bus0_registers r;
gencpu64_axictrl_bus0_registers rin;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_AXI_INTERCONNECT)
) xdef0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(wb_def_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(wb_def_xslvi),
    .o_xslvo(wb_def_xslvo),
    .o_req_valid(w_def_req_valid),
    .o_req_addr(wb_def_req_addr),
    .o_req_size(wb_def_req_size),
    .o_req_write(w_def_req_write),
    .o_req_wdata(wb_def_req_wdata),
    .o_req_wstrb(wb_def_req_wstrb),
    .o_req_last(w_def_req_last),
    .i_req_ready(w_def_req_ready),
    .i_resp_valid(w_def_resp_valid),
    .i_resp_rdata(wb_def_resp_rdata),
    .i_resp_err(w_def_resp_err)
);

always_comb
begin: comb_proc
    gencpu64_axictrl_bus0_registers v;
    axi4_master_in_type vmsti[0: CFG_BUS0_XMST_TOTAL-1];
    axi4_master_out_type vmsto[0: CFG_BUS0_XMST_TOTAL-1];
    axi4_slave_in_type vslvi[0: CFG_BUS0_XSLV_TOTAL-1];
    axi4_slave_out_type vslvo[0: CFG_BUS0_XSLV_TOTAL-1];
    logic [(CFG_BUS0_XMST_TOTAL * CFG_BUS0_XSLV_TOTAL)-1:0] vb_ar_select;
    logic [((CFG_BUS0_XMST_TOTAL + 1) * CFG_BUS0_XSLV_TOTAL)-1:0] vb_ar_available;
    logic [CFG_BUS0_XMST_TOTAL-1:0] vb_ar_hit;
    logic [(CFG_BUS0_XMST_TOTAL * CFG_BUS0_XSLV_TOTAL)-1:0] vb_aw_select;
    logic [((CFG_BUS0_XMST_TOTAL + 1) * CFG_BUS0_XSLV_TOTAL)-1:0] vb_aw_available;
    logic [CFG_BUS0_XMST_TOTAL-1:0] vb_aw_hit;
    logic [(CFG_BUS0_XMST_TOTAL * CFG_BUS0_XSLV_LOG2_TOTAL)-1:0] vb_w_select;
    logic [CFG_BUS0_XMST_TOTAL-1:0] vb_w_active;

    v = r;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        vmsti[i] = axi4_master_in_none;
    end
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        vmsto[i] = axi4_master_out_none;
    end
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        vslvi[i] = axi4_slave_in_none;
    end
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        vslvo[i] = axi4_slave_out_none;
    end
    vb_ar_select = '0;
    vb_ar_available = '1;
    vb_ar_hit = '0;
    vb_aw_select = '0;
    vb_aw_available = '1;
    vb_aw_hit = '0;
    vb_w_select = '0;
    vb_w_active = '0;


    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        vmsto[i] = i_xmsto[i];                              // Cannot read vector item from port in systemc
        vmsti[i] = axi4_master_in_none;
    end

    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        vslvo[i] = i_xslvo[i];                              // Cannot read vector item from port in systemc
        vslvi[i] = axi4_slave_in_none;
    end
    // Local unmapped slots:
    vslvo[(CFG_BUS0_XSLV_TOTAL - 1)] = wb_def_xslvo;

    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        for (int ii = 0; ii < (CFG_BUS0_XSLV_TOTAL - 1); ii++) begin
            // Connect AR channel
            if ((CFG_BUS0_MAP[ii].addr_start[(CFG_SYSBUS_ADDR_BITS - 1): 12] <= vmsto[i].ar_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12])
                    && (vmsto[i].ar_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12] < CFG_BUS0_MAP[ii].addr_end[(CFG_SYSBUS_ADDR_BITS - 1): 12])) begin
                vb_ar_hit[i] = vmsto[i].ar_valid;
                vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] = (vmsto[i].ar_valid && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
                // Update availability bit for the next master and this slave:
                vb_ar_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = ((~vmsto[i].ar_valid) && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
            end else begin
                vb_ar_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)];
            end

            // Connect AW channel
            if ((CFG_BUS0_MAP[ii].addr_start[(CFG_SYSBUS_ADDR_BITS - 1): 12] <= vmsto[i].aw_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12])
                    && (vmsto[i].aw_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12] < CFG_BUS0_MAP[ii].addr_end[(CFG_SYSBUS_ADDR_BITS - 1): 12])) begin
                vb_aw_hit[i] = vmsto[i].aw_valid;
                vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] = (vmsto[i].aw_valid && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
                // Update availability bit for the next master and this slave:
                vb_aw_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = ((~vmsto[i].aw_valid) && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
            end else begin
                vb_aw_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)];
            end
        end
    end

    // access to unmapped slave:
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        if ((vmsto[i].ar_valid == 1'b1) && (vb_ar_hit[i] == 1'b0)) begin
            vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = (vmsto[i].ar_valid && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);
        end
        vb_ar_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = ((~vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))])
                && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);

        if ((vmsto[i].aw_valid == 1'b1) && (vb_aw_hit[i] == 1'b0)) begin
            vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = (vmsto[i].aw_valid && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);
        end
        vb_aw_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = ((~vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))])
                && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);
    end

    vb_w_select = r.w_select;
    vb_w_active = r.w_active;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        for (int ii = 0; ii < CFG_BUS0_XSLV_TOTAL; ii++) begin
            if (vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] == 1'b1) begin
                vmsti[i].ar_ready = vslvo[ii].ar_ready;
                vslvi[ii].ar_valid = vmsto[i].ar_valid;
                vslvi[ii].ar_bits = vmsto[i].ar_bits;
                vslvi[ii].ar_user = vmsto[i].ar_user;
                vslvi[ii].ar_id = {vmsto[i].ar_id, i};
            end
            if (vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] == 1'b1) begin
                vmsti[i].aw_ready = vslvo[ii].aw_ready;
                vslvi[ii].aw_valid = vmsto[i].aw_valid;
                vslvi[ii].aw_bits = vmsto[i].aw_bits;
                vslvi[ii].aw_user = vmsto[i].aw_user;
                vslvi[ii].aw_id = {vmsto[i].aw_id, i};
                if (vslvo[ii].aw_ready == 1'b1) begin
                    // Switch W-channel index to future w-transaction without id
                    vb_w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL] = ii;
                    vb_w_active[i] = 1'b1;
                end
            end
        end
    end
    v.w_select = vb_w_select;

    // W-channel
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        if ((vmsto[i].w_valid == 1'b1) && (r.w_active[i] == 1'b1)) begin
            vmsti[i].w_ready = vslvo[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_ready;
            vslvi[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_valid = vmsto[i].w_valid;
            vslvi[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_data = vmsto[i].w_data;
            vslvi[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_strb = vmsto[i].w_strb;
            vslvi[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_last = vmsto[i].w_last;
            vslvi[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_user = vmsto[i].w_user;
            if ((vmsto[i].w_last == 1'b1)
                    && (vslvo[int'(r.w_select[(i * CFG_BUS0_XSLV_LOG2_TOTAL) +: CFG_BUS0_XSLV_LOG2_TOTAL])].w_ready == 1'b1)) begin
                vb_w_active[i] = 1'b0;
            end
        end
    end
    v.w_active = vb_w_active;

    // B-channel
    for (int ii = 0; ii < CFG_BUS0_XSLV_TOTAL; ii++) begin
        if (vslvo[ii].b_valid == 1'b1) begin
            vslvi[ii].b_ready = vmsto[int'(vslvo[ii].b_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].b_ready;
            vmsti[int'(vslvo[ii].b_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].b_valid = vslvo[ii].b_valid;
            vmsti[int'(vslvo[ii].b_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].b_resp = vslvo[ii].b_resp;
            vmsti[int'(vslvo[ii].b_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].b_user = vslvo[ii].b_user;
            vmsti[int'(vslvo[ii].b_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].b_id = vslvo[ii].b_id[(CFG_SYSBUS_ID_BITS - 1): CFG_BUS0_XMST_LOG2_TOTAL];
        end
    end

    // R-channel
    for (int ii = 0; ii < CFG_BUS0_XSLV_TOTAL; ii++) begin
        if (vslvo[ii].r_valid == 1'b1) begin
            vslvi[ii].r_ready = vmsto[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_ready;
            vmsti[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_valid = vslvo[ii].r_valid;
            vmsti[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_data = vslvo[ii].r_data;
            vmsti[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_last = vslvo[ii].r_last;
            vmsti[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_resp = vslvo[ii].r_resp;
            vmsti[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_user = vslvo[ii].r_user;
            vmsti[int'(vslvo[ii].r_id[(CFG_BUS0_XMST_LOG2_TOTAL - 1): 0])].r_id = vslvo[ii].r_id[(CFG_SYSBUS_ID_BITS - 1): CFG_BUS0_XMST_LOG2_TOTAL];
        end
    end

    w_def_req_ready = 1'b1;
    v.r_def_valid = w_def_req_valid;
    w_def_resp_valid = r.r_def_valid;
    wb_def_resp_rdata = '1;
    w_def_resp_err = 1'b1;

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = gencpu64_axictrl_bus0_r_reset;
    end

    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        o_xmsti[i] = vmsti[i];
    end
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        o_xslvi[i] = vslvi[i];
        o_mapinfo[i] = CFG_BUS0_MAP[i];
    end
    wb_def_xslvi = vslvi[(CFG_BUS0_XSLV_TOTAL - 1)];
    wb_def_mapinfo = CFG_BUS0_MAP[(CFG_BUS0_XSLV_TOTAL - 1)];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= gencpu64_axictrl_bus0_r_reset;
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

endmodule: gencpu64_axictrl_bus0
