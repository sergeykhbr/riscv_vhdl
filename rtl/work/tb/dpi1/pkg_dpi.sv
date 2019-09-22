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

package pkg_dpi;

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

import "DPI-C" context task c_task_server_start();
import "DPI-C" context task c_task_clk_posedge(input sv_out_t sv2c, output sv_in_t c2sv);
export "DPI-C" function sv_func_info;

function void sv_func_info(input string info);
begin
    $display("SV: %s", info);
end
endfunction: sv_func_info

endpackage

