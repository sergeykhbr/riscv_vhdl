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

// 
// 
static const uint8_t CMD0 = 0;                              // GO_IDLE_STATE: Reset card to idle state. Response - (4.7.4)
static const uint8_t CMD8 = 8;                              // SEND_IF_COND: Card interface condition. Response R7 (4.9.6).
// 
static const uint8_t R1 = 1;
static const uint8_t R2 = 2;
static const uint8_t R3 = 3;
static const uint8_t R6 = 6;
// 
static const bool DIR_OUTPUT = 0;
static const bool DIR_INPUT = 1;
// 
static const uint8_t CMDERR_NONE = 0;
static const uint8_t CMDERR_NO_RESPONSE = 1;
static const uint8_t CMDERR_WRONG_RESP_STARTBIT = 2;
static const uint8_t CMDERR_WRONG_RESP_STOPBIT = 3;

}  // namespace debugger

