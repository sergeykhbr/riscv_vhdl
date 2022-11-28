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
#include "types_amba.h"

namespace debugger {

// @defgroup slave_id_group AMBA APB slaves generic IDs.
// @details Each module in a SoC has to be indexed by unique identificator.
//          In current implementation it is used sequential indexing for it.
//          Indexes are used to specify a device bus item in a vectors.

// @brief Worjgroup DMI interface.
static const int CFG_BUS1_PSLV_DMI = 0;
// @brief UART0 APB device.
static const int CFG_BUS1_PSLV_UART1 = 1;
// Total number of the APB slaves devices on Bus[1].
static const int CFG_BUS1_PSLV_TOTAL = 2;

// @defgroup master_id_group APB masters generic IDs.
// @details Each master must be assigned to a specific ID that used
//          as an index in the vector array of APB master bus.
// 
// Bus[0] master interface
static const int CFG_BUS1_PMST_PARENT = 0;
// Total Number of master devices that have access to APB Bus[1].
static const int CFG_BUS1_PMST_TOTAL = 1;

typedef sc_vector<sc_signal<apb_in_type>> bus1_pslv_in_vector;
typedef sc_vector<sc_signal<apb_out_type>> bus1_pslv_out_vector;
typedef sc_vector<sc_signal<apb_out_type>> bus1_pmst_in_vector;
typedef sc_vector<sc_signal<apb_in_type>> bus1_pmst_out_vector;
typedef sc_vector<sc_signal<mapinfo_type>> bus1_mapinfo_vector;

// Bus 1 device tree
static const mapinfo_type CFG_BUS1_MAP[CFG_BUS1_PSLV_TOTAL] = {
    {0x000001001E000, 0x000001001F000},                     // 0, dmi 4KB. TODO: change base address
    {0x0000010010000, 0x0000010011000}                      // 1, uart1 4KB
};

}  // namespace debugger

