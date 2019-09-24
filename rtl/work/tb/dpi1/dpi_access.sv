/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

import pkg_amba4::*;
import pkg_dpi::*;

module virt_dbg #(
    parameter realtime CLK_PERIOD = 5.0ns
)(
    input nrst,
    input clk,
    input [31:0] clkcnt,
    output axi4_slave_in_type o_slvi,
    input axi4_slave_out_type i_slvo
);

typedef enum {
    Bus_Idle,
    Bus_ar,
    Bus_r,
    Bus_aw,
    Bus_w,
    Bus_b
} bus_state_t;

typedef struct packed {
    bus_state_t estate;
    bit [CFG_SYSBUS_ADDR_BITS-1:0] addr;
    bit [7:0] burst_cnt;
    bit [7:0] burst_len;
    bit [127:0] burst_buf;
    bit [63:0] w_data;
    bit [7:0] w_strb;
} registers_t;

registers_t v, r;

sv_in_t sv_in;
sv_out_t sv_out;
axi4_slave_in_type vslvi;

initial begin
    c_task_server_start();
end

function void get_burst(input bit [2:1] addr,
                       output bit [7:0] len,
                       output bit [2:0] size,
                       output bit [1:0] burst);
begin
    len = 8'd0;
    size = 3'd3;   // 8-bytes
    burst = AXI_BURST_FIXED;
    if (addr == 2'b11) begin
        len = 8'd1;
        burst = AXI_BURST_INCR;
    end
end
endfunction: get_burst

function longint get_rdata(input bit [2:1] addr,
                           input bit [127:0] buffer);
begin
    case (addr)
    2'b00: begin
        return buffer[63:0];
    end
    2'b01: begin
        return {16'd0, buffer[63:16]};
    end
    2'b10: begin
        return {32'd0, buffer[63:32]};
    end
    2'b11: begin
        return {buffer[47:0], buffer[127:112]};
    end
    default: begin
    end
    endcase
end
endfunction: get_rdata

always_comb begin
    v = r;

    sv_out.slvo.rdata[0] = 0;
    sv_out.req_ready = 0;
    sv_out.resp_valid = 0;

    vslvi.aw_valid = 1'b0;
    vslvi.aw_bits = META_NONE;
    vslvi.w_valid = 1'b0;
    vslvi.w_data = 0;
    vslvi.w_last = 1'b0;
    vslvi.w_strb = 0;
    vslvi.ar_valid = 1'b0;


    case (r.estate)
    Bus_Idle: begin
        v.burst_cnt = 0;
        v.burst_buf = 0;
        if (sv_in.req_type == REQ_TYPE_AXI4 && sv_in.req_valid == 1) begin
            if (sv_in.slvi.we == 1) begin
                vslvi.aw_valid = 1'b1;
                vslvi.aw_bits.addr = sv_in.slvi.addr;
                if (i_slvo.aw_ready == 1) begin
                    v.addr = sv_in.slvi.addr[CFG_SYSBUS_ADDR_BITS-1:0];
                    get_burst(sv_in.slvi.addr[2:1],
                              vslvi.aw_bits.len,
                              vslvi.aw_bits.size,
                              vslvi.aw_bits.burst);
                    v.burst_len = vslvi.aw_bits.len;
                    v.w_data = sv_in.slvi.wdata[0];
                    v.w_strb = sv_in.slvi.wstrb;
                    v.estate = Bus_w;
                    sv_out.req_ready = 1;
                end else begin
                    v.estate = Bus_aw;
                end
            end else begin
                vslvi.ar_valid = 1'b1;
                vslvi.ar_bits.addr = sv_in.slvi.addr;
                if (i_slvo.ar_ready == 1) begin
                    v.addr = sv_in.slvi.addr[CFG_SYSBUS_ADDR_BITS-1:0];
                    get_burst(sv_in.slvi.addr[2:1],
                              vslvi.ar_bits.len,
                              vslvi.ar_bits.size,
                              vslvi.ar_bits.burst);
                    v.burst_len = vslvi.ar_bits.len;
                    v.estate = Bus_r;
                    sv_out.req_ready = 1;
                end else begin
                    v.estate = Bus_ar;
                end
            end
        end
    end
    Bus_ar: begin
        vslvi.ar_valid = 1'b1;
        vslvi.ar_bits.addr = sv_in.slvi.addr;
        if (i_slvo.ar_ready == 1) begin
            v.addr = sv_in.slvi.addr[CFG_SYSBUS_ADDR_BITS-1:0];
            get_burst(sv_in.slvi.addr[2:1],
                      vslvi.ar_bits.len,
                      vslvi.ar_bits.size,
                      vslvi.ar_bits.burst);
             v.burst_len = vslvi.ar_bits.len;
             v.estate = Bus_r;
             sv_out.req_ready = 1;
        end
    end
    Bus_r: begin
        if (i_slvo.r_valid == 1) begin
            v.burst_buf = {r.burst_buf[63:0], i_slvo.r_data};
            if (r.burst_cnt == r.burst_len) begin
                sv_out.resp_valid = 1;
                sv_out.slvo.rdata[0] = get_rdata(r.addr[2:1],
                                                 v.burst_buf);
                v.estate = Bus_Idle;
            end else begin
                v.burst_cnt = r.burst_cnt + 1;
            end
        end
    end
    Bus_aw: begin
        vslvi.aw_valid = 1'b1;
        vslvi.aw_bits.addr = sv_in.slvi.addr;
        v.addr = sv_in.slvi.addr[CFG_SYSBUS_ADDR_BITS-1:0];
        get_burst(sv_in.slvi.addr[2:1],
                  vslvi.aw_bits.len,
                  vslvi.aw_bits.size,
                  vslvi.aw_bits.burst);
         v.burst_len = vslvi.aw_bits.len;
         v.w_data = sv_in.slvi.wdata[0];
         v.w_strb = sv_in.slvi.wstrb;
         if (i_slvo.aw_ready == 1) begin
             v.estate = Bus_w;
             sv_out.req_ready = 1;
         end
    end
    Bus_w: begin
        vslvi.w_valid = 1'b1;
        vslvi.w_data = r.w_data;
        vslvi.w_strb = r.w_strb;
        if (i_slvo.w_ready == 1) begin
            v.estate = Bus_b;
        end
    end
    Bus_b: begin
        if (i_slvo.b_valid == 1) begin
            sv_out.resp_valid = 1;
            v.estate = Bus_Idle;
        end
    end
    default:
        $display("SV: undefined state: %0d", r.estate);
    endcase

    if (nrst == 0) begin
        v.estate = Bus_Idle;
        v.addr = 0;
        v.burst_cnt = 0;
        v.burst_len = 0;
        v.burst_buf = 0;
        v.w_data = 0;
        v.w_strb = 0;
    end

    o_slvi <= vslvi;
end

always_ff @(posedge clk) begin
    r <= v;
    sv_out.tm <= $time;
    sv_out.clkcnt <= clkcnt;

    c_task_clk_posedge(sv_out, sv_in);
end


endmodule: virt_dbg
