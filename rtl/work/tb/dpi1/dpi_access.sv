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
    axi4_slave_out_t slvo;
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
    string info;
    int req_type;
    int param1;
    axi4_slave_in_t slvi;
} sv_in_t;

sv_out_t sv_out;

import "DPI-C" context task c_task_server_start();
import "DPI-C" context task c_task_clk_posedge(input sv_out_t d);
export "DPI-C" task     sv_task_process_request;
export "DPI-C" function sv_func_get_data;

initial begin
    c_task_server_start();
end

/*
function string bytearr2string(input arr);
 string str1;
  str1 = {
       $sformatf("%h",arr),
         }
  return(str1);
endfunction
*/
function void sv_func_get_data(output sv_out_t d);
begin
    d.tm = $time;
    d.clkcnt = clkcnt;
end
endfunction: sv_func_get_data

task sv_task_process_request(input sv_in_t r);
   int i;
string str1;
begin
    case (r.req_type)
    REQ_TYPE_SERVER_ERR: begin
        $display("SV: server error");
        $stop(0);
    end
    REQ_TYPE_STOP_SIM: begin
        $display("SV: simulation end");
        $stop(0);
    end
    REQ_TYPE_INFO: begin
        $display("SV: %s", r.info);
    end
    REQ_TYPE_MOVE_CLOCK: begin
        i = 0;
        do begin
             #CLK_PERIOD
             i = i + 1;
        end while (i < r.param1);
    end
//    REQ_TYPE_MOVE_TIME: begin
//        do begin
//             #CLK_PERIOD
//        end while (r.param1 < $time);
//    end
    default:
        $display("SV: unsupported request: %0d", r.req_type);
    endcase

end
endtask: sv_task_process_request

always @(posedge clk) begin
    //sv_func_get_data(sv_out);
    sv_out.tm = $time;
    sv_out.clkcnt = clkcnt;
    c_task_clk_posedge(sv_out);
end


endmodule: virt_dbg
