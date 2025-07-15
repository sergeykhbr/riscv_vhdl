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

// @name Vendor IDs defintion.
static const uint16_t VENDOR_GNSSSENSOR = 0x00F1;
static const uint16_t VENDOR_OPTIMITECH = 0x00F2;

// @name Master Device IDs definition:
// Empty master slot device
static const uint16_t MST_DID_EMPTY = 0x7755;
// "River" CPU Device ID.
static const uint16_t RISCV_RIVER_CPU = 0x0505;
// "Wasserfall" CPU Device ID.
static const uint16_t RISCV_RIVER_WORKGROUP = 0x0506;
// UART with DMA: Test Access Point (TAP)
static const uint16_t GNSSSENSOR_UART_TAP = 0x050A;
// JTAG Test Access Point (TAP) with SBA interface (DMA without progbuf)
static const uint16_t OPTIMITECH_JTAG_SBA = 0x050B;
// PCIE DMA engine
static const uint16_t OPTIMITECH_PCIE_DMA = 0x050C;
// HDMI DMA engine
static const uint16_t OPTIMITECH_HDMI_DMA = 0x050D;

// @name Slave Device IDs definition:
// Empty slave slot device
static const uint16_t SLV_DID_EMPTY = 0x5577;
// Boot ROM Device ID
static const uint16_t OPTIMITECH_ROM = 0x0071;
// Internal SRAM block Device ID
static const uint16_t OPTIMITECH_SRAM = 0x0073;
// Configuration Registers Module
static const uint16_t OPTIMITECH_PNP = 0x0074;
// SD-card controller
static const uint16_t OPTIMITECH_SPI_FLASH = 0x0075;
// General purpose IOs
static const uint16_t OPTIMITECH_GPIO = 0x0076;
// rs-232 UART Device ID
static const uint16_t OPTIMITECH_UART = 0x007A;
// Core local interrupt controller
static const uint16_t OPTIMITECH_CLINT = 0x0083;
// External interrupt controller
static const uint16_t OPTIMITECH_PLIC = 0x0084;
// AXI to APB Brdige
static const uint16_t OPTIMITECH_AXI2APB_BRIDGE = 0x0085;
// AXI interconnect
static const uint16_t OPTIMITECH_AXI_INTERCONNECT = 0x0086;
// APB PLL and Register Interface
static const uint16_t OPTIMITECH_PRCI = 0x0087;
// DDR controller status registers
static const uint16_t OPTIMITECH_DDRCTRL = 0x0088;
// SD-card controller control registers
static const uint16_t OPTIMITECH_SDCTRL_REG = 0x0089;
// SD-card controller memory
static const uint16_t OPTIMITECH_SDCTRL_MEM = 0x008B;
// RIVER debug registers:
static const uint16_t OPTIMITECH_RIVER_DMI = 0x008A;
// PCIE end-point APB controller:
static const uint16_t OPTIMITECH_PCIE_CTRL = 0x008C;
// I2C master interface meant for ADV7511 HDMI transmitter
static const uint16_t OPTIMITECH_I2C = 0x008D;

// Plug'n'Play descriptor localparams.
// Undefined type of the descriptor (empty device).
static const uint8_t PNP_CFG_TYPE_INVALID = 0x0;
// AXI slave device standard descriptor.
static const uint8_t PNP_CFG_TYPE_MASTER = 0x1;
// AXI master device standard descriptor.
static const uint8_t PNP_CFG_TYPE_SLAVE = 0x2;
// @brief Size in bytes of the standard slave descriptor..
// @details Firmware uses this value instead of sizeof(slave_config_type).
static const uint8_t PNP_CFG_DEV_DESCR_BYTES = 0x10;

// @brief   Plug-n-play descriptor structure for connected device.
// @details Each device must generates this datatype output that
//          is connected directly to the 'pnp' slave module on system bus.
class dev_config_type {
 public:
    dev_config_type() {
        // Descriptor size in bytes.
        descrsize = PNP_CFG_DEV_DESCR_BYTES;
        // Descriptor type.
        descrtype = PNP_CFG_TYPE_SLAVE;
        // Base Address.
        addr_start = 0;
        // End of the base address.
        addr_end = 0;
        // Vendor ID.
        vid = VENDOR_GNSSSENSOR;
        // Device ID.
        did = SLV_DID_EMPTY;
    }

    dev_config_type(sc_uint<8> descrsize_,
                    sc_uint<2> descrtype_,
                    sc_uint<64> addr_start_,
                    sc_uint<64> addr_end_,
                    sc_uint<16> vid_,
                    sc_uint<16> did_) {
        descrsize = descrsize_;
        descrtype = descrtype_;
        addr_start = addr_start_;
        addr_end = addr_end_;
        vid = vid_;
        did = did_;
    }

    inline bool operator == (const dev_config_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.descrsize == descrsize
            && rhs.descrtype == descrtype
            && rhs.addr_start == addr_start
            && rhs.addr_end == addr_end
            && rhs.vid == vid
            && rhs.did == did;
        return ret;
    }

    inline dev_config_type& operator = (const dev_config_type &rhs) {
        descrsize = rhs.descrsize;
        descrtype = rhs.descrtype;
        addr_start = rhs.addr_start;
        addr_end = rhs.addr_end;
        vid = rhs.vid;
        did = rhs.did;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const dev_config_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.descrsize, NAME + "_descrsize");
        sc_trace(tf, v.descrtype, NAME + "_descrtype");
        sc_trace(tf, v.addr_start, NAME + "_addr_start");
        sc_trace(tf, v.addr_end, NAME + "_addr_end");
        sc_trace(tf, v.vid, NAME + "_vid");
        sc_trace(tf, v.did, NAME + "_did");
    }

    inline friend ostream &operator << (ostream &os,
                                        dev_config_type const &v) {
        os << "("
        << v.descrsize << ","
        << v.descrtype << ","
        << v.addr_start << ","
        << v.addr_end << ","
        << v.vid << ","
        << v.did << ")";
        return os;
    }

 public:
    // Descriptor size in bytes.
    sc_uint<8> descrsize;
    // Descriptor type.
    sc_uint<2> descrtype;
    // Base Address.
    sc_uint<64> addr_start;
    // End of the base address.
    sc_uint<64> addr_end;
    // Vendor ID.
    sc_uint<16> vid;
    // Device ID.
    sc_uint<16> did;
};

// @brief Default config value for empty slot.
static const dev_config_type dev_config_none;

// Plug-and-Play device descriptors array connected to pnp module:
static const int SOC_PNP_XCTRL0 = 0;
static const int SOC_PNP_GROUP0 = 1;
static const int SOC_PNP_BOOTROM = 2;
static const int SOC_PNP_SRAM = 3;
static const int SOC_PNP_DDR_AXI = 4;
static const int SOC_PNP_DDR_APB = 5;
static const int SOC_PNP_PRCI = 6;
static const int SOC_PNP_GPIO = 7;
static const int SOC_PNP_CLINT = 8;
static const int SOC_PNP_PLIC = 9;
static const int SOC_PNP_PNP = 10;
static const int SOC_PNP_PBRIDGE0 = 11;
static const int SOC_PNP_DMI = 12;
static const int SOC_PNP_UART1 = 13;
static const int SOC_PNP_SDCTRL_REG = 14;
static const int SOC_PNP_SDCTRL_MEM = 15;
static const int SOC_PNP_I2C = 16;
static const int SOC_PNP_PCIE_DMA = 17;
static const int SOC_PNP_PCIE_APB = 18;
static const int SOC_PNP_HDMI_DMA = 19;
static const int SOC_PNP_TOTAL = 20;

typedef sc_vector<sc_signal<dev_config_type>> soc_pnp_vector;

}  // namespace debugger

