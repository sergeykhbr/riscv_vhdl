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

typedef struct {
    realtime tm;
    int clkcnt;
} dpi_data_t;

typedef struct {
    int req_type;
    int param1;
} request_t;

const int REQ_TYPE_MOVE_CLOCK = 1;
const int REQ_TYPE_MOVE_TIME  = 2;
const int REQ_TYPE_STOP_SIM   = -1;

import "DPI-C" context task c_task_server_start();
export "DPI-C" task     sv_task_process_request;
export "DPI-C" function sv_func_get_data;

initial begin
    c_task_server_start();
end


function void sv_func_get_data(output dpi_data_t d);
begin
    d.tm = $time;
    d.clkcnt = clkcnt;
end
endfunction: sv_func_get_data

task sv_task_process_request(input request_t r);
   int i;
begin
    if (r.req_type == REQ_TYPE_MOVE_CLOCK) begin
        $display("SV: time before wait: %0d", $time);
        i = 0;
        do begin
             #CLK_PERIOD
             i = i + 1;
        end while (i < r.param1);
        $display("SV: time after wait: %0d", $time);
    end else if (r.req_type == REQ_TYPE_STOP_SIM) begin
        $stop(0);
    end
end
endtask: sv_task_process_request


endmodule: virt_dbg
