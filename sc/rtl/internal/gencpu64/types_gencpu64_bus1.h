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
#include "../ambalib/types_amba.h"

namespace debugger {

// @defgroup slave_id_group AMBA APB slaves generic IDs.
// @details Each module in a SoC has to be indexed by unique identificator.
//          In current implementation it is used sequential indexing for it.
//          Indexes are used to specify a device bus item in a vectors.

// @brief UART0 APB device.
static const int CFG_BUS1_PSLV_UART1 = 0;
// @brief System status and control registers device.
static const int CFG_BUS1_PSLV_PRCI = 1;
// @brief Worjgroup DMI interface.
static const int CFG_BUS1_PSLV_DMI = 2;
// Configuration index of the SD-card control registers.
static const int CFG_BUS1_PSLV_SDCTRL_REG = 3;
// Configuration index of the GPIO (General Purpose In/Out) module.
static const int CFG_BUS1_PSLV_GPIO = 4;
// @brief DDR control register.
static const int CFG_BUS1_PSLV_DDR = 5;
// Configuration index of the Plug-n-Play module.
static const int CFG_BUS1_PSLV_PNP = 6;
// Total number of the APB slaves devices on Bus[1].
static const int CFG_BUS1_PSLV_TOTAL = 7;
// Necessary bus width to store index + 1.
static const int CFG_BUS1_PSLV_LOG2_TOTAL = 3;              // $clog2(CFG_BUS1_PSLV_TOTAL + 1)

typedef sc_vector<sc_signal<apb_in_type>> bus1_apb_in_vector;
typedef sc_vector<sc_signal<apb_out_type>> bus1_apb_out_vector;
typedef sc_vector<sc_signal<mapinfo_type>> bus1_mapinfo_vector;

// Bus 1 device tree
static const mapinfo_type CFG_BUS1_MAP[CFG_BUS1_PSLV_TOTAL] = {
        {
            0x0000000000010000,         // addr_start
            0x0000000000011000          // addr_end
        },// uart1 4KB
        {
            0x0000000000012000,         // addr_start
            0x0000000000013000          // addr_end
        },// PRCI 4KB
        {
            0x000000000001E000,         // addr_start
            0x000000000001F000          // addr_end
        },// dmi 4KB. TODO: change base address
        {
            0x0000000000050000,         // addr_start
            0x0000000000051000          // addr_end
        },// SPI SD-card 4KB
        {
            0x0000000000060000,         // addr_start
            0x0000000000061000          // addr_end
        },// GPIO 4KB
        {
            0x00000000000C0000,         // addr_start
            0x00000000000C1000          // addr_end
        },// DDR MGMT 4KB
        {
            0x00000000000FF000,         // addr_start
            0x0000000000100000          // addr_end
        }// Plug'n'Play 4KB
};

}  // namespace debugger

