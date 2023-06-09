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
package types_pnp_pkg;

import types_amba_pkg::*;


// Plug-and-Play device descriptors array connected to pnp module:
localparam int SOC_PNP_XCTRL0 = 0;
localparam int SOC_PNP_GROUP0 = 1;
localparam int SOC_PNP_BOOTROM = 2;
localparam int SOC_PNP_SRAM = 3;
localparam int SOC_PNP_DDR_AXI = 4;
localparam int SOC_PNP_DDR_APB = 5;
localparam int SOC_PNP_PRCI = 6;
localparam int SOC_PNP_GPIO = 7;
localparam int SOC_PNP_CLINT = 8;
localparam int SOC_PNP_PLIC = 9;
localparam int SOC_PNP_PNP = 10;
localparam int SOC_PNP_PBRIDGE0 = 11;
localparam int SOC_PNP_DMI = 12;
localparam int SOC_PNP_UART1 = 13;
localparam int SOC_PNP_SPI = 14;
localparam int SOC_PNP_TOTAL = 15;

typedef dev_config_type soc_pnp_vector[0:SOC_PNP_TOTAL - 1];

endpackage: types_pnp_pkg
