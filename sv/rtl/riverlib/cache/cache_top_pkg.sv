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
package cache_top_pkg;

import river_cfg_pkg::*;

localparam int DATA_PATH = 0;
localparam int CTRL_PATH = 1;
localparam int QUEUE_WIDTH = (CFG_CPU_ADDR_BITS  // o_req_mem_addr
        + REQ_MEM_TYPE_BITS  // o_req_mem_type
        + 3  // o_req_mem_size
        + 1  // i_resp_mem_path
);

typedef struct {
    logic req_mem_valid;
    logic [REQ_MEM_TYPE_BITS-1:0] req_mem_type;
    logic [2:0] req_mem_size;
    logic [CFG_CPU_ADDR_BITS-1:0] req_mem_addr;
    logic [L1CACHE_BYTES_PER_LINE-1:0] req_mem_strob;
    logic [L1CACHE_LINE_BITS-1:0] req_mem_wdata;
    logic [CFG_CPU_ADDR_BITS-1:0] mpu_addr;
    logic [CFG_CPU_ADDR_BITS-1:0] resp_addr;
} CacheOutputType;


endpackage: cache_top_pkg
