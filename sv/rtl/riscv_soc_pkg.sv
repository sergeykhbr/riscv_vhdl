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
package riscv_soc_pkg;

import types_amba_pkg::*;
import types_pnp_pkg::*;
import types_bus0_pkg::*;
import types_bus1_pkg::*;
import river_cfg_pkg::*;


// Hardware SoC Identificator.
// Read Only unique platform identificator that could be read by FW
localparam bit [31:0] SOC_HW_ID = 32'h20220903;

// UARTx fifo log2(size) in bytes:
localparam int SOC_UART1_LOG2_FIFOSZ = 4;

// Number of available generic IO pins:
localparam int SOC_GPIO0_WIDTH = 12;

// SD-card in SPI mode buffer size. It should be at least log2(512) Bytes:
localparam int SOC_SPI0_LOG2_FIFOSZ = 9;

// Number of contexts in PLIC controller.
// Example FU740: S7 Core0 (M) + 4xU74 Cores (M+S).
localparam int SOC_PLIC_CONTEXT_TOTAL = 9;
// Any number up to 1024. Zero interrupt must be 0.
localparam int SOC_PLIC_IRQ_TOTAL = 73;

endpackage: riscv_soc_pkg
