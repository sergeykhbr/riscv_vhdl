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
package vip_uart_receiver_pkg;


localparam bit [1:0] startbit = 2'h0;
localparam bit [1:0] data = 2'h1;
localparam bit [1:0] stopbit = 2'h2;
localparam bit [1:0] dummy = 2'h3;

typedef struct {
    logic rx;
    logic [1:0] state;
    logic rdy;
    logic [7:0] rdata;
    logic [31:0] sample;
    logic [3:0] bitpos;
    logic [7:0] scratch;
    logic rx_err;
} vip_uart_receiver_registers;

const vip_uart_receiver_registers vip_uart_receiver_r_reset = '{
    1'b0,                               // rx
    startbit,                           // state
    1'b0,                               // rdy
    '0,                                 // rdata
    '0,                                 // sample
    '0,                                 // bitpos
    '0,                                 // scratch
    1'b0                                // rx_err
};

endpackage: vip_uart_receiver_pkg
