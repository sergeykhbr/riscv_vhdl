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
package apb_i2c_pkg;

import types_amba_pkg::*;
import types_pnp_pkg::*;

// SPI states
localparam bit [7:0] STATE_IDLE = 8'h00;
localparam bit [7:0] STATE_START = 8'h01;
localparam bit [7:0] STATE_HEADER = 8'h02;
localparam bit [7:0] STATE_ACK_HEADER = 8'h04;
localparam bit [7:0] STATE_RX_DATA = 8'h08;
localparam bit [7:0] STATE_ACK_DATA = 8'h10;
localparam bit [7:0] STATE_TX_DATA = 8'h20;
localparam bit [7:0] STATE_WAIT_ACK_DATA = 8'h40;
localparam bit [7:0] STATE_STOP = 8'h80;

localparam bit PIN_DIR_INPUT = 1'b1;
localparam bit PIN_DIR_OUTPUT = 1'b0;

typedef struct {
    logic [15:0] scaler;
    logic [15:0] scaler_cnt;
    logic [15:0] setup_time;                                // Interval after negedge of the clock pulsse
    logic level;
    logic [6:0] addr;                                       // I2C multiplexer
    logic R_nW;                                             // 0=Write; 1=read
    logic [31:0] payload;
    logic [7:0] state;
    logic start;
    logic sda_dir;
    logic [18:0] shiftreg;                                  // 1start+7adr+1rw+1ack+8data+ack
    logic [7:0] rxbyte;
    logic [2:0] bit_cnt;
    logic [3:0] byte_cnt;
    logic ack;
    logic err_ack_header;
    logic err_ack_data;
    logic irq;
    logic ie;
    logic nreset;                                           // Active LOW (by default), could be any
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_i2c_registers;

const apb_i2c_registers apb_i2c_r_reset = '{
    '0,                                 // scaler
    '0,                                 // scaler_cnt
    16'h0001,                           // setup_time
    1'b1,                               // level
    7'h74,                              // addr
    1'b0,                               // R_nW
    '0,                                 // payload
    STATE_IDLE,                         // state
    1'b0,                               // start
    PIN_DIR_OUTPUT,                     // sda_dir
    '1,                                 // shiftreg
    '0,                                 // rxbyte
    '0,                                 // bit_cnt
    '0,                                 // byte_cnt
    1'b0,                               // ack
    1'b0,                               // err_ack_header
    1'b0,                               // err_ack_data
    1'b0,                               // irq
    1'b0,                               // ie
    1'b0,                               // nreset
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};
endpackage: apb_i2c_pkg
