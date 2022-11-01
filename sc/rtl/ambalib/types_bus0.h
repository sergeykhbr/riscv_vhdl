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

// 
// 
// 
// 
static const int CFG_BUS0_XSLV_TOTAL = 9;
// 
// @defgroup master_id_group AXI4 masters generic IDs.
// @details Each master must be assigned to a specific ID that used
//          as an index in the vector array of AXI master bus.
// 
// Total number of CPU limited by config CFG_TOTAL_CPU_MAX
static const int CFG_BUS0_XMST_CPU0 = 0;
// DMA master interface generic index.
static const int CFG_BUS0_XMST_DMA = 1;
// Total Number of master devices on system bus.
static const int CFG_BUS0_XMST_TOTAL = 2;
// 
typedef sc_vector<sc_signal<axi4_slave_config_type>> bus0_xslv_cfg_vector;
typedef sc_vector<sc_signal<axi4_master_config_type>> bus0_xmst_cfg_vector;
typedef sc_vector<sc_signal<axi4_master_in_type>> bus0_xmst_in_vector;
typedef sc_vector<sc_signal<axi4_master_out_type>> bus0_xmst_out_vector;
typedef sc_vector<sc_signal<axi4_slave_in_type>> bus0_xslv_in_vector;
typedef sc_vector<sc_signal<axi4_slave_out_type>> bus0_xslv_out_vector;

}  // namespace debugger

