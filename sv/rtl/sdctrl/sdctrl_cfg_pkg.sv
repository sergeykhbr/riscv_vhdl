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
package sdctrl_cfg_pkg;


// 
// 
localparam bit [5:0] CMD0 = 6'h00;                          // GO_IDLE_STATE: Reset card to idle state. Response - (4.7.4)
localparam bit [5:0] CMD8 = 6'h08;                          // SEND_IF_COND: Card interface condition. Response R7 (4.9.6).
// 
localparam bit [2:0] R1 = 3'h1;
localparam bit [2:0] R2 = 3'h2;
localparam bit [2:0] R3 = 3'h3;
localparam bit [2:0] R6 = 3'h6;
// 
localparam bit DIR_OUTPUT = 1'h0;
localparam bit DIR_INPUT = 1'h1;
// 
localparam bit [3:0] CMDERR_NONE = 4'h0;
localparam bit [3:0] CMDERR_NO_RESPONSE = 4'h1;
localparam bit [3:0] CMDERR_WRONG_RESP_STARTBIT = 4'h2;
localparam bit [3:0] CMDERR_WRONG_RESP_STOPBIT = 4'h3;

endpackage: sdctrl_cfg_pkg
