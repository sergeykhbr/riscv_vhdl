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
#pragma once

#include <systemc.h>

namespace debugger {

// Cache config:
static const int CFG_SDCACHE_ADDR_BITS = 48;
static const int CFG_LOG2_SDCACHE_LINEBITS = 5;             // 2=2KB (4 x (8x64))
static const int CFG_LOG2_SDCACHE_BYTES_PER_LINE = 6;       // 64 Bytes
static const int SDCACHE_BYTES_PER_LINE = (1 << CFG_LOG2_SDCACHE_BYTES_PER_LINE);
static const int SDCACHE_LINE_BITS = (8 * SDCACHE_BYTES_PER_LINE);

static const int SDCACHE_FL_VALID = 0;
static const int SDCACHE_FL_DIRTY = 1;
static const int SDCACHE_FL_TOTAL = 2;
// 
// 
static const uint8_t CMD0 = 0;                              // GO_IDLE_STATE: Reset card to idle state. Response - (4.7.4)
static const uint8_t CMD2 = 2;                              // ALL_SEND_CID: ask to send CID number
static const uint8_t CMD3 = 3;                              // SEND_RELATIVE_ADDRE: Ask to publish (RCA) relative address
static const uint8_t CMD8 = 8;                              // SEND_IF_COND: Card interface condition. Response R7 (4.9.6).
static const uint8_t CMD11 = 11;                            // VOLTAGE_SWITCH: Switch to 1.8V bus signaling level
static const uint8_t CMD17 = 17;                            // READ_SINGLE_BLOCK: Read block size of SET_BLOCKLEN
static const uint8_t CMD24 = 24;                            // WRITE_SINGLE_BLOCK: Write block size of SET_BLOCKLEN
static const uint8_t ACMD41 = 41;
static const uint8_t CMD55 = 55;                            // APP_CMD: application specific commands
static const uint8_t CMD58 = 58;                            // READ_OCR: Read OCR register in SPI mode
// 
static const uint8_t R1 = 1;
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
static const uint8_t R2 = 2;
// 4.9.4 R3 (OCR register, page 140)
//     [47]    Start bit = 1'b0
//     [46]    Transmission bit = 1'b0
//     [45:40] reserved = 6'b111111
//     [39:8]  OCR register = 32'hX
//     [7:1]   reserved = 7'b1111111
//     [0]     End bit = 1'b1
static const uint8_t R3 = 3;
// 4.9.5 R3 (Published RCA response, page 141)
//     [47]    Start bit = 1'b0
//     [46]    Transmission bit = 1'b0
//     [45:40] Command index = 6'b000011
//     [39:25] New published RCA[31:16] of the card
//     [24:8]  status bits {[23,22,12,[12:0]} see Table 4-42
//     [7:1]   CRC7 = 7'hXX
//     [0]     End bit = 1'b1
static const uint8_t R6 = 6;
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
static const uint8_t R7 = 7;
// 
static const bool DIR_OUTPUT = 0;
static const bool DIR_INPUT = 1;

// Card types detected during identification stage
static const uint8_t SDCARD_UNKNOWN = 0;
static const uint8_t SDCARD_VER1X = 1;                      // Ver1.X Standard Capacity
static const uint8_t SDCARD_VER2X_SC = 2;                   // Ver2.00 or higer Standard Capacity
static const uint8_t SDCARD_VER2X_HC = 3;                   // Ver2.00 or higer High or Extended Capacity
static const uint8_t SDCARD_UNUSABLE = 7;
// 
static const uint8_t CMDERR_NONE = 0;
static const uint8_t CMDERR_NO_RESPONSE = 1;
static const uint8_t CMDERR_WRONG_RESP_STARTBIT = 2;
static const uint8_t CMDERR_WRONG_RESP_STOPBIT = 3;

}  // namespace debugger

