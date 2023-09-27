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
localparam bit [5:0] CMD2 = 6'h02;                          // ALL_SEND_CID: ask to send CID number
localparam bit [5:0] CMD3 = 6'h03;                          // SEND_RELATIVE_ADDRE: Ask to publish (RCA) relative address
localparam bit [5:0] CMD8 = 6'h08;                          // SEND_IF_COND: Card interface condition. Response R7 (4.9.6).
localparam bit [5:0] CMD11 = 6'h0b;                         // VOLTAGE_SWITCH: Switch to 1.8V bus signaling level
localparam bit [5:0] ACMD41 = 6'h29;
localparam bit [5:0] CMD55 = 6'h37;                         // APP_CMD: application specific commands
localparam bit [5:0] CMD58 = 6'h3a;                         // READ_OCR: Read OCR register in SPI mode
// 
localparam bit [2:0] R1 = 3'h1;
// 4.9.3 R2 (CID, CSD register, page 140)
//     [135]     Start bit = 1'b0
//     [134]     Transmission bit = 1'b0
//     [133:128] reserved = 6'b111111
//     [127:120] Manufacturer ID = 8'hX
//     [119:104] OEM/Application ID = 16'hX
//     [103:64]  Product name = 40'hX
//     [63:56]   Product revision = 8'hX
//     [55:24]   Product serial number = 32'hX
//     [23:20]   reserved = 4'h0
//     [19:8]    Manufacturer date = 12'hX
//     [7:1]     CRC7 = 7'hXX
//     [0]       End bit = 1'b1
localparam bit [2:0] R2 = 3'h2;
// 4.9.4 R3 (OCR register, page 140)
//     [47]    Start bit = 1'b0
//     [46]    Transmission bit = 1'b0
//     [45:40] reserved = 6'b111111
//     [39:8]  OCR register = 32'hX
//     [7:1]   reserved = 7'b1111111
//     [0]     End bit = 1'b1
localparam bit [2:0] R3 = 3'h3;
// 4.9.5 R^ (Published RCA response, page 141)
//     [47]    Start bit = 1'b0
//     [46]    Transmission bit = 1'b0
//     [45:40] Command index = 6'b000011
//     [39:25] New published RCA[31:16] of the card
//     [24:8]  status bits {[23,22,12,[12:0]} see Table 4-42
//     [7:1]   CRC7 = 7'hXX
//     [0]     End bit = 1'b1
localparam bit [2:0] R6 = 3'h6;
// 4.9.6 R7 (Card interface condition, page 142)
//     [47]    Start bit = 1'b0
//     [46]    Transmission bit = 1'b0
//     [45:40] Command index = 6'b001000
//     [39:22] Reserved bits = 18'h0
//     [21]    PCIe 1.2V support = 1'bX
//     [20]    PCIe Response = 1'bX
//     [19:16] Voltage accepted = 4'hX
//     [15:8]  Echo-back of check pattern = 8'hXX
//     [7:1]   CRC7 = 7'hXX
//     [0]     End bit = 1'b1
localparam bit [2:0] R7 = 3'h7;
// 
localparam bit DIR_OUTPUT = 1'h0;
localparam bit DIR_INPUT = 1'h1;

// Card types detected during identification stage
localparam bit [2:0] SDCARD_UNKNOWN = 3'h0;
localparam bit [2:0] SDCARD_VER1X = 3'h1;                   // Ver1.X Standard Capacity
localparam bit [2:0] SDCARD_VER2X_SC = 3'h2;                // Ver2.00 or higer Standard Capacity
localparam bit [2:0] SDCARD_VER2X_HC = 3'h3;                // Ver2.00 or higer High or Extended Capacity
localparam bit [2:0] SDCARD_UNUSABLE = 3'h7;
// 
localparam bit [3:0] CMDERR_NONE = 4'h0;
localparam bit [3:0] CMDERR_NO_RESPONSE = 4'h1;
localparam bit [3:0] CMDERR_WRONG_RESP_STARTBIT = 4'h2;
localparam bit [3:0] CMDERR_WRONG_RESP_STOPBIT = 4'h3;

endpackage: sdctrl_cfg_pkg
