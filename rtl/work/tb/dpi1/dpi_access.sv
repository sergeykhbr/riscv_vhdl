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
    bit [7:0] burst_cnt;
    bit [7:0] burst_len;
    bit [7:0] wstrb;
    bit [AXI4_BURST_BITS_TOTAL-1:0] wdata;
} registers_t;

registers_t v, r;

sv_in_t sv_in;
sv_out_t sv_out;
axi4_slave_in_type vslvi;

initial begin
    c_task_server_start();
end


always_comb begin
    v = r;

    sv_out.slvo.ready = 0;
    sv_out.slvo.valid = 0;

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
        v.burst_len = sv_in.slvi.len;
        sv_out.slvo.ready = 1;
        if (sv_in.req_type == REQ_TYPE_AXI4 && sv_in.req_valid == 1) begin
            if (sv_in.slvi.we == 1) begin
                vslvi.aw_valid = 1'b1;
                vslvi.aw_bits.addr = sv_in.slvi.addr;
                vslvi.aw_bits.size = 3'd3;   // 8-bytes only (sys bus width)
                vslvi.aw_bits.len = sv_in.slvi.len;
                vslvi.aw_bits.burst = AXI_BURST_FIXED;
                if (sv_in.slvi.len != 0) begin
                    vslvi.aw_bits.burst = AXI_BURST_INCR;
                end

                v.wstrb = sv_in.slvi.wstrb;
                for (int i = 0; i < AXI4_BURST_LEN_MAX; i++) begin
                    v.wdata[64*(i+1)-1 +: 64] = sv_in.slvi.wdata[i];
                end
                if (i_slvo.aw_ready == 1) begin
                    v.estate = Bus_w;
                end else begin
                    v.estate = Bus_aw;
                end
            end else begin
                vslvi.ar_valid = 1'b1;
                vslvi.ar_bits.addr = sv_in.slvi.addr;
                vslvi.ar_bits.size = 3'd3;   // 8-bytes only (sys bus width)
                vslvi.ar_bits.len = sv_in.slvi.len;
                vslvi.ar_bits.burst = AXI_BURST_FIXED;
                if (sv_in.slvi.len != 0) begin
                    vslvi.ar_bits.burst = AXI_BURST_INCR;
                end

                if (i_slvo.ar_ready == 1) begin
                    v.estate = Bus_r;
                end else begin
                    v.estate = Bus_ar;
                end
            end
        end
    end
    Bus_ar: begin
        vslvi.ar_valid = 1'b1;
        if (i_slvo.ar_ready == 1) begin
             v.estate = Bus_r;
        end
    end
    Bus_r: begin
        if (i_slvo.r_valid == 1) begin
            sv_out.slvo.rdata[r.burst_cnt] = i_slvo.r_data;
            if (r.burst_cnt == r.burst_len) begin
                sv_out.slvo.valid = 1;
                v.estate = Bus_Idle;
            end else begin
                v.burst_cnt = r.burst_cnt + 1;
            end
        end
    end
    Bus_aw: begin
        vslvi.aw_valid = 1'b1;
         if (i_slvo.aw_ready == 1) begin
             v.estate = Bus_w;
         end
    end
    Bus_w: begin
        vslvi.w_valid = 1'b1;
        vslvi.w_data = r.wdata[63:0];
        vslvi.w_strb = r.wstrb;
        if (r.burst_cnt == sv_in.slvi.len) begin
            vslvi.w_last = 1'b1;
        end
        if (i_slvo.w_ready == 1) begin
            v.wdata = {64'd0, r.wdata[AXI4_BURST_BITS_TOTAL-1:64]};
            if (r.burst_cnt == r.burst_len) begin
                v.estate = Bus_b;
            end else begin
                v.burst_cnt = r.burst_cnt + 1;
            end
        end
    end
    Bus_b: begin
        if (i_slvo.b_valid == 1) begin
            sv_out.slvo.valid = 1;
            v.estate = Bus_Idle;
        end
    end
    default:
        $display("SV: undefined state: %0d", r.estate);
    endcase

    if (nrst == 0) begin
        v.estate = Bus_Idle;
        v.burst_cnt = 0;
        v.burst_len = 0;
        v.wstrb = 0;
        v.wdata = 0;
    end

    o_slvi <= vslvi;
end

always_ff @(posedge clk) begin
    r <= v;
    sv_out.tm <= $time;
    sv_out.clkcnt <= clkcnt;

    c_task_slv_clk_posedge(sv_out, sv_in);
end


endmodule: virt_dbg
