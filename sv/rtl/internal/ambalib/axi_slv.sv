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
    parameter logic async_reset = 1'b0,
    parameter int unsigned vid = 0,                         // Vendor ID
    parameter int unsigned did = 0                          // Device ID
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // Base address information from the interconnect port
    output types_pnp_pkg::dev_config_type o_cfg,            // Slave config descriptor
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
import types_pnp_pkg::*;
import axi_slv_pkg::*;

axi_slv_registers r;
axi_slv_registers rin;


always_comb
begin: comb_proc
    axi_slv_registers v;
    logic [11:0] vb_ar_addr_next;
    logic [11:0] vb_aw_addr_next;
    logic [8:0] vb_ar_len_next;
    dev_config_type vcfg;
    axi4_slave_out_type vxslvo;

    v = r;
    vb_ar_addr_next = '0;
    vb_aw_addr_next = '0;
    vb_ar_len_next = '0;
    vcfg = dev_config_none;
    vxslvo = axi4_slave_out_none;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.addr_start;
    vcfg.addr_end = i_mapinfo.addr_end;
    vcfg.vid = vid;
    vcfg.did = did;

    vb_ar_addr_next = (r.req_addr[11: 0] + {4'd0, r.ar_bytes});
    if (r.ar_burst == AXI_BURST_FIXED) begin
        vb_ar_addr_next = r.req_addr[11: 0];
    end else if (r.ar_burst == AXI_BURST_WRAP) begin
        // Wrap suppported only 2, 4, 8 or 16 Bytes. See ARMDeveloper spec.
        if (r.ar_bytes == 2) begin
            vb_ar_addr_next[11: 1] = r.req_addr[11: 1];
        end else if (r.ar_bytes == 4) begin
            vb_ar_addr_next[11: 2] = r.req_addr[11: 2];
        end else if (r.ar_bytes == 8) begin
            vb_ar_addr_next[11: 3] = r.req_addr[11: 3];
        end else if (r.ar_bytes == 16) begin
            vb_ar_addr_next[11: 4] = r.req_addr[11: 4];
        end else if (r.ar_bytes == 32) begin
            // Optional (not in ARM spec)
            vb_ar_addr_next[11: 5] = r.req_addr[11: 5];
        end
    end
    vb_ar_len_next = (r.ar_len - 1);

    vb_aw_addr_next = (r.req_addr[11: 0] + {4'd0, r.aw_bytes});
    if (r.aw_burst == AXI_BURST_FIXED) begin
        vb_aw_addr_next = r.req_addr[11: 0];
    end else if (r.aw_burst == AXI_BURST_WRAP) begin
        // Wrap suppported only 2, 4, 8 or 16 Bytes. See ARMDeveloper spec.
        if (r.aw_bytes == 2) begin
            vb_aw_addr_next[11: 1] = r.req_addr[11: 1];
        end else if (r.aw_bytes == 4) begin
            vb_aw_addr_next[11: 2] = r.req_addr[11: 2];
        end else if (r.aw_bytes == 8) begin
            vb_aw_addr_next[11: 3] = r.req_addr[11: 3];
        end else if (r.aw_bytes == 16) begin
            vb_aw_addr_next[11: 4] = r.req_addr[11: 4];
        end else if (r.aw_bytes == 32) begin
            // Optional (not in ARM spec)
            vb_aw_addr_next[11: 5] = r.req_addr[11: 5];
        end
    end

    if ((i_xslvi.ar_valid & r.ar_ready) == 1'b1) begin
        v.ar_ready = 1'b0;
    end
    if ((i_xslvi.aw_valid & r.aw_ready) == 1'b1) begin
        v.aw_ready = 1'b0;
    end
    if ((i_xslvi.w_valid & r.w_ready) == 1'b1) begin
        v.w_ready = 1'b0;
    end
    if ((i_xslvi.r_ready & r.r_valid) == 1'b1) begin
        v.r_err = 1'b0;
        v.r_last = 1'b0;
        v.r_valid = 1'b0;
    end
    if ((i_xslvi.b_ready & r.b_valid) == 1'b1) begin
        v.b_err = 1'b0;
        v.b_valid = 1'b0;
    end
    if ((r.req_valid & i_req_ready) == 1'b1) begin
        v.req_valid = 1'b0;
    end

    // Reading channel (write first):
    case (r.rstate)
    State_r_idle: begin
        v.ar_addr = (i_xslvi.ar_bits.addr - i_mapinfo.addr_start);
        v.ar_len = ({1'b0, i_xslvi.ar_bits.len} + 9'd1);
        v.ar_burst = i_xslvi.ar_bits.burst;
        v.ar_bytes = XSizeToBytes(i_xslvi.ar_bits.size);
        v.ar_last = (~(|i_xslvi.ar_bits.len));
        v.ar_id = i_xslvi.ar_id;
        v.ar_user = i_xslvi.ar_user;
        if ((r.ar_ready == 1'b1) && (i_xslvi.ar_valid == 1'b1)) begin
            if ((i_xslvi.aw_valid == 1'b1) || ((|r.wstate) == 1'b1)) begin
                v.rstate = State_r_wait_writing;
            end else begin
                v.rstate = State_r_addr;
                v.req_valid = 1'b1;
                v.req_write = 1'b0;
                v.req_addr = (i_xslvi.ar_bits.addr - i_mapinfo.addr_start);
                v.req_last = (~(|i_xslvi.ar_bits.len));
                v.req_bytes = XSizeToBytes(i_xslvi.ar_bits.size);
            end
        end else begin
            v.ar_ready = 1'b1;
        end
    end
    State_r_addr: begin
        v.req_valid = i_xslvi.r_ready;
        if ((r.req_valid == 1'b1) && (i_req_ready == 1'b1)) begin
            if (r.ar_len > 9'h001) begin
                v.ar_len = (r.ar_len - 1);
                v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_ar_addr_next};
                v.req_last = (~(|vb_ar_len_next[8: 1]));
                v.rstate = State_r_data;
            end else begin
                v.req_valid = 1'b0;
                v.ar_len = 9'd0;
                v.ar_last = 1'b1;
                v.rstate = State_r_last;
            end
        end
    end
    State_r_data: begin
        v.req_valid = i_xslvi.r_ready;
        if ((r.req_valid == 1'b1) && (i_req_ready == 1'b1)) begin
            if (r.ar_len > 9'h001) begin
                v.ar_len = vb_ar_len_next;
                v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_ar_addr_next};
                v.req_last = (~(|vb_ar_len_next[8: 1]));
            end else begin
                v.ar_len = 9'd0;
                v.req_last = 1'b1;
            end
        end
        if ((r.req_valid & r.req_last & i_req_ready) == 1'b1) begin
            v.rstate = State_r_last;
            v.req_valid = 1'b0;
        end
        if (i_resp_valid == 1'b1) begin
            if ((r.r_valid == 1'b1) && (i_xslvi.r_ready == 1'b0)) begin
                // We already requested the last value but previous was not accepted yet
                v.r_data_buf = i_resp_rdata;
                v.r_err_buf = i_resp_err;
                v.r_last_buf = (r.req_valid & r.req_last & i_req_ready);
                v.rstate = State_r_buf;
            end else begin
                v.r_valid = 1'b1;
                v.r_last = 1'b0;
                v.r_data = i_resp_rdata;
                v.r_err = i_resp_err;
            end
        end
    end
    State_r_last: begin
        if (i_resp_valid == 1'b1) begin
            if ((r.r_valid == 1'b1) && (i_xslvi.r_ready == 1'b0)) begin
                // We already requested the last value but previous was not accepted yet
                v.r_data_buf = i_resp_rdata;
                v.r_err_buf = i_resp_err;
                v.r_last_buf = 1'b1;
                v.rstate = State_r_buf;
            end else begin
                v.r_valid = 1'b1;
                v.r_last = 1'b1;
                v.r_data = i_resp_rdata;
                v.r_err = i_resp_err;
            end
        end
        if ((r.r_valid == 1'b1) && (r.r_last == 1'b1) && (i_xslvi.r_ready == 1'b1)) begin
            v.ar_ready = 1'b1;
            v.r_last = 1'b0;
            v.r_valid = 1'b0;                               // We need it in a case of i_resp_valid is always HIGH
            v.rstate = State_r_idle;
        end
    end
    State_r_buf: begin
        if (i_xslvi.r_ready == 1'b1) begin
            v.r_valid = 1'b1;
            v.r_last = r.r_last_buf;
            v.r_data = r.r_data_buf;
            v.r_err = r.r_err_buf;
            if (r.r_last_buf == 1'b1) begin
                v.rstate = State_r_last;
            end else begin
                v.rstate = State_r_data;
            end
        end
    end
    State_r_wait_writing: begin
        if ((((|r.wstate) == 1'b0) && (i_xslvi.aw_valid == 1'b0))
                || ((r.req_valid & r.req_last & i_req_ready) == 1'b1)) begin
            // End of writing, start reading
            v.req_valid = 1'b1;
            v.req_write = 1'b0;
            v.req_addr = r.ar_addr;
            v.req_bytes = r.ar_bytes;
            v.req_last = r.ar_last;
            v.rstate = State_r_addr;
        end
    end
    default: begin
        v.rstate = State_r_idle;
    end
    endcase

    // Writing channel:
    case (r.wstate)
    State_w_idle: begin
        v.w_ready = 1'b1;
        v.aw_addr = (i_xslvi.aw_bits.addr - i_mapinfo.addr_start);
        v.aw_burst = i_xslvi.aw_bits.burst;
        v.aw_bytes = XSizeToBytes(i_xslvi.aw_bits.size);
        v.w_last = (~(|i_xslvi.aw_bits.len));
        v.aw_id = i_xslvi.aw_id;
        v.aw_user = i_xslvi.aw_user;
        if ((r.aw_ready == 1'b1) && (i_xslvi.aw_valid == 1'b1)) begin
            v.req_wdata = i_xslvi.w_data;
            v.req_wstrb = i_xslvi.w_strb;
            if ((r.w_ready == 1'b1) && (i_xslvi.w_valid == 1'b1)) begin
                // AXI Light support:
                v.wstate = State_w_pipe;
                v.w_last = i_xslvi.w_last;
                if ((|r.rstate) == 1'b1) begin
                    // Postpone writing
                    v.w_ready = 1'b0;
                    v.wstate = State_w_wait_reading_light;
                end else begin
                    // Start writing now
                    v.req_addr = (i_xslvi.aw_bits.addr - i_mapinfo.addr_start);
                    v.req_bytes = XSizeToBytes(i_xslvi.aw_bits.size);
                    v.req_last = i_xslvi.w_last;
                    v.req_write = 1'b1;
                    v.req_valid = 1'b1;
                    v.w_ready = i_req_ready;
                end
            end else if ((|r.rstate) == 1'b1) begin
                v.wstate = State_w_wait_reading;
                v.w_ready = 1'b0;
            end else begin
                v.req_addr = (i_xslvi.aw_bits.addr - i_mapinfo.addr_start);
                v.req_bytes = XSizeToBytes(i_xslvi.aw_bits.size);
                v.wstate = State_w_req;
                v.w_ready = 1'b1;
                v.req_write = 1'b1;
            end
        end else begin
            v.aw_ready = 1'b1;
        end
    end
    State_w_req: begin
        if (i_xslvi.w_valid == 1'b1) begin
            v.w_ready = (i_req_ready & (~i_xslvi.w_last));
            v.req_valid = 1'b1;
            v.req_wdata = i_xslvi.w_data;
            v.req_wstrb = i_xslvi.w_strb;
            v.req_last = i_xslvi.w_last;
            v.wstate = State_w_pipe;
        end
    end
    State_w_pipe: begin
        v.w_ready = ((i_req_ready | i_resp_valid) & (~r.req_last));
        if ((r.w_ready == 1'b1) && (i_xslvi.w_valid == 1'b1)) begin
            v.req_valid = 1'b1;
            v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_aw_addr_next};
            v.req_wdata = i_xslvi.w_data;
            v.req_wstrb = i_xslvi.w_strb;
            v.req_last = i_xslvi.w_last;
        end
        if ((r.req_valid == 1'b1) && (i_req_ready == 1'b1) && (r.req_last == 1'b1)) begin
            v.req_last = 1'b0;
            v.wstate = State_w_resp;
        end else if ((i_resp_valid == 1'b1) && (i_xslvi.w_valid == 1'b0) && (r.req_valid == 1'b0)) begin
            v.w_ready = 1'b1;
            v.req_addr = {r.req_addr[(CFG_SYSBUS_ADDR_BITS - 1): 12], vb_aw_addr_next};
            v.wstate = State_w_req;
        end
    end
    State_w_resp: begin
        if (i_resp_valid == 1'b1) begin
            v.b_valid = 1'b1;
            v.b_err = i_resp_err;
            v.w_last = 1'b0;
            v.wstate = State_b;
        end
    end
    State_w_wait_reading: begin
        // ready to accept new data (no latched data)
        if (((|r.rstate) == 1'b0) || ((r.r_valid & r.r_last & i_xslvi.r_ready) == 1'b1)) begin
            v.w_ready = 1'b1;
            v.req_write = 1'b1;
            v.req_addr = r.aw_addr;
            v.req_bytes = r.aw_bytes;
            v.wstate = State_w_req;
        end
    end
    State_w_wait_reading_light: begin
        // Not ready to accept new data before writing the last one
        if (((|r.rstate) == 1'b0) || ((r.r_valid & r.r_last & i_xslvi.r_ready) == 1'b1)) begin
            v.req_valid = 1'b1;
            v.req_write = 1'b1;
            v.req_addr = r.aw_addr;
            v.req_bytes = r.aw_bytes;
            v.req_last = r.w_last;
            v.wstate = State_w_pipe;
        end
    end
    State_b: begin
        if ((r.b_valid == 1'b1) && (i_xslvi.b_ready == 1'b1)) begin
            v.b_valid = 1'b0;
            v.b_err = 1'b0;
            v.aw_ready = 1'b1;
            v.w_ready = 1'b1;                               // AXI light
            v.wstate = State_w_idle;
        end
    end
    default: begin
        v.wstate = State_w_idle;
    end
    endcase

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = axi_slv_r_reset;
    end

    o_req_valid = r.req_valid;
    o_req_last = r.req_last;
    o_req_addr = r.req_addr;
    o_req_size = r.req_bytes;
    o_req_write = r.req_write;
    o_req_wdata = r.req_wdata;
    o_req_wstrb = r.req_wstrb;

    vxslvo.ar_ready = r.ar_ready;
    vxslvo.r_valid = r.r_valid;
    vxslvo.r_id = r.ar_id;
    vxslvo.r_user = r.ar_user;
    vxslvo.r_resp = {r.r_err, 1'b0};
    vxslvo.r_data = r.r_data;
    vxslvo.r_last = r.r_last;
    vxslvo.aw_ready = r.aw_ready;
    vxslvo.w_ready = r.w_ready;
    vxslvo.b_valid = r.b_valid;
    vxslvo.b_id = r.aw_id;
    vxslvo.b_user = r.aw_user;
    vxslvo.b_resp = {r.b_err, 1'b0};
    o_xslvo = vxslvo;
    o_cfg = vcfg;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= axi_slv_r_reset;
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

endmodule: axi_slv
