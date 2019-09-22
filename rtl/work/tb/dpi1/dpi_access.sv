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

module virt_dbg #(
    parameter realtime CLK_PERIOD = 5.0ns
)(
    input clk,
    input [31:0] clkcnt
);

parameter int AXI4_BURST_LEN_MAX = 8;

typedef struct {
    longint rdata[AXI4_BURST_LEN_MAX];
} axi4_slave_out_t;

typedef struct {
    realtime tm;
    int clkcnt;
    int req_ready;
    int resp_valid;
    axi4_slave_out_t slvo;
    int irq_request;
} sv_out_t;

const int REQ_TYPE_SERVER_ERR = -2;
const int REQ_TYPE_STOP_SIM   = -1;
const int REQ_TYPE_INFO       = 1;
const int REQ_TYPE_MOVE_CLOCK = 2;
const int REQ_TYPE_MOVE_TIME  = 3;
const int REQ_TYPE_AXI4       = 4;

typedef struct {
    longint addr;
    longint wdata[AXI4_BURST_LEN_MAX];
    byte we;    // 0=read; 1=write
    byte wstrb;
    byte burst;
    byte len;
} axi4_slave_in_t;

typedef struct {
    int req_valid;
    int req_type;
    int param1;
    axi4_slave_in_t slvi;
} sv_in_t;

enum {
    Bus_Idle,
    Bus_WaitResponse
} estate;
sv_in_t sv_in;
sv_out_t sv_out;

import "DPI-C" context task c_task_server_start();
import "DPI-C" context task c_task_clk_posedge(input sv_out_t sv2c, output sv_in_t c2sv);
export "DPI-C" function sv_func_info;

initial begin
    c_task_server_start();
end

function void sv_func_info(input string info);
begin
    $display("SV: %s", info);
end
endfunction: sv_func_info

always @(posedge clk) begin
    sv_out.tm = $time;
    sv_out.clkcnt = clkcnt;
    sv_out.slvo.rdata[0] = 0;
    sv_out.req_ready = 0;
    sv_out.resp_valid = 0;

    case (estate)
    Bus_Idle: begin
        sv_out.req_ready = 1;
        if (sv_in.req_type == REQ_TYPE_AXI4 && sv_in.req_valid == 1) begin
            estate = Bus_WaitResponse;
        end
    end
    Bus_WaitResponse: begin
        sv_out.resp_valid = 1;
        sv_out.slvo.rdata[0] = 'hfeedfacecafef00d;
        estate = Bus_Idle;
    end
    default:
        $display("SV: undefined state: %0d", estate);
    endcase
    c_task_clk_posedge(sv_out, sv_in);
end


endmodule: virt_dbg
