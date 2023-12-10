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


// @name Vendor IDs defintion.
localparam bit [15:0] VENDOR_GNSSSENSOR = 16'h00F1;
localparam bit [15:0] VENDOR_OPTIMITECH = 16'h00F2;

// @name Master Device IDs definition:
// Empty master slot device
localparam bit [15:0] MST_DID_EMPTY = 16'h7755;
// "River" CPU Device ID.
localparam bit [15:0] RISCV_RIVER_CPU = 16'h0505;
// "Wasserfall" CPU Device ID.
localparam bit [15:0] RISCV_RIVER_WORKGROUP = 16'h0506;
// UART with DMA: Test Access Point (TAP)
localparam bit [15:0] GNSSSENSOR_UART_TAP = 16'h050A;
// JTAG Test Access Point (TAP) with SBA interface (DMA without progbuf)
localparam bit [15:0] OPTIMITECH_JTAG_SBA = 16'h050B;

// @name Slave Device IDs definition:
// Empty slave slot device
localparam bit [15:0] SLV_DID_EMPTY = 16'h5577;
// Boot ROM Device ID
localparam bit [15:0] OPTIMITECH_ROM = 16'h0071;
// Internal SRAM block Device ID
localparam bit [15:0] OPTIMITECH_SRAM = 16'h0073;
// Configuration Registers Module
localparam bit [15:0] OPTIMITECH_PNP = 16'h0074;
// SD-card controller
localparam bit [15:0] OPTIMITECH_SPI_FLASH = 16'h0075;
// General purpose IOs
localparam bit [15:0] OPTIMITECH_GPIO = 16'h0076;
// rs-232 UART Device ID
localparam bit [15:0] OPTIMITECH_UART = 16'h007A;
// Core local interrupt controller
localparam bit [15:0] OPTIMITECH_CLINT = 16'h0083;
// External interrupt controller
localparam bit [15:0] OPTIMITECH_PLIC = 16'h0084;
// AXI to APB Brdige
localparam bit [15:0] OPTIMITECH_AXI2APB_BRIDGE = 16'h0085;
// AXI interconnect
localparam bit [15:0] OPTIMITECH_AXI_INTERCONNECT = 16'h0086;
// APB PLL and Register Interface
localparam bit [15:0] OPTIMITECH_PRCI = 16'h0087;
// DDR controller status registers
localparam bit [15:0] OPTIMITECH_DDRCTRL = 16'h0088;
// SD-card controller control registers
localparam bit [15:0] OPTIMITECH_SDCTRL_REG = 16'h0089;
// SD-card controller memory
localparam bit [15:0] OPTIMITECH_SDCTRL_MEM = 16'h008B;
// RIVER debug registers:
localparam bit [15:0] OPTIMITECH_RIVER_DMI = 16'h008A;

// Plug'n'Play descriptor localparams.
// Undefined type of the descriptor (empty device).
localparam bit [1:0] PNP_CFG_TYPE_INVALID = 2'h0;
// AXI slave device standard descriptor.
localparam bit [1:0] PNP_CFG_TYPE_MASTER = 2'h1;
// AXI master device standard descriptor.
localparam bit [1:0] PNP_CFG_TYPE_SLAVE = 2'h2;
// @brief Size in bytes of the standard slave descriptor..
// @details Firmware uses this value instead of sizeof(slave_config_type).
localparam bit [7:0] PNP_CFG_DEV_DESCR_BYTES = 8'h10;

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
localparam int SOC_PNP_SDCTRL_REG = 14;
localparam int SOC_PNP_SDCTRL_MEM = 15;
localparam int SOC_PNP_TOTAL = 16;

// @brief   Plug-n-play descriptor structure for connected device.
// @details Each device must generates this datatype output that
//          is connected directly to the 'pnp' slave module on system bus.
typedef struct {
    // Descriptor size in bytes.
    logic [7:0] descrsize;
    // Descriptor type.
    logic [1:0] descrtype;
    // Base Address.
    logic [63:0] addr_start;
    // End of the base address.
    logic [63:0] addr_end;
    // Vendor ID.
    logic [15:0] vid;
    // Device ID.
    logic [15:0] did;
} dev_config_type;

// @brief Default config value for empty slot.
const dev_config_type dev_config_none = '{PNP_CFG_DEV_DESCR_BYTES, PNP_CFG_TYPE_SLAVE, '0, '0, VENDOR_GNSSSENSOR, SLV_DID_EMPTY};

typedef dev_config_type soc_pnp_vector[0:SOC_PNP_TOTAL - 1];

endpackage: types_pnp_pkg
