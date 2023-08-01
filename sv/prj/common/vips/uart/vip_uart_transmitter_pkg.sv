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
package vip_uart_transmitter_pkg;


localparam bit [1:0] idle = 2'h0;
localparam bit [1:0] startbit = 2'h1;
localparam bit [1:0] data = 2'h2;
localparam bit [1:0] stopbit = 2'h3;

typedef struct {
    logic [1:0] state;
    logic [31:0] sample;
    logic txdata_rdy;
    logic [7:0] txdata;
    logic [8:0] shiftreg;
    logic [3:0] bitpos;
    logic overflow;
} vip_uart_transmitter_registers;

const vip_uart_transmitter_registers vip_uart_transmitter_r_reset = '{
    idle,                               // state
    '0,                                 // sample
    1'b0,                               // txdata_rdy
    '0,                                 // txdata
    '1,                                 // shiftreg
    '0,                                 // bitpos
    1'b0                                // overflow
};

endpackage: vip_uart_transmitter_pkg
