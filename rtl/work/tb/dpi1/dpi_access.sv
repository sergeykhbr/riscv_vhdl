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
        if (sv_in.req_type == REQ_TYPE_AXI4 && sv_in.req_valid == 1) begin
            if (sv_in.slvi.we == 1) begin
                vslvi.aw_valid = 1'b1;
                vslvi.aw_bits.addr = sv_in.slvi.addr;
                if (i_slvo.aw_ready == 1) begin
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
            v.estate = Bus_r;
            sv_out.req_ready = 1;
        end
    end
    Bus_r: begin
        if (i_slvo.r_valid == 1) begin
            sv_out.resp_valid = 1;
            sv_out.slvo.rdata[0] = i_slvo.r_data;
            v.estate = Bus_Idle;
        end
    end
    Bus_aw: begin
        vslvi.aw_valid = 1'b1;
        vslvi.aw_bits.addr = sv_in.slvi.addr;
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
