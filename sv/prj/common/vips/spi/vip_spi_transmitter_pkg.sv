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
package vip_spi_transmitter_pkg;


localparam bit [1:0] state_cmd = 2'd0;
localparam bit [1:0] state_addr = 2'd1;
localparam bit [1:0] state_data = 2'd2;

typedef struct {
    logic [1:0] state;
    logic sclk;
    logic [31:0] rxshift;
    logic [31:0] txshift;
    logic [3:0] bitcnt;
    logic [2:0] bytecnt;
    logic byterdy;
    logic req_valid;
    logic req_write;
    logic [31:0] req_addr;
    logic [31:0] req_wdata;
} vip_spi_transmitter_registers;

const vip_spi_transmitter_registers vip_spi_transmitter_r_reset = '{
    state_cmd,                          // state
    1'b0,                               // sclk
    '1,                                 // rxshift
    '1,                                 // txshift
    '0,                                 // bitcnt
    '0,                                 // bytecnt
    1'b0,                               // byterdy
    1'b0,                               // req_valid
    1'b0,                               // req_write
    '0,                                 // req_addr
    '0                                  // req_wdata
};

endpackage: vip_spi_transmitter_pkg
