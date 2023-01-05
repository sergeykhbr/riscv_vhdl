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

static const int CFG_SYSBUS_ADDR_BITS = 48;
static const int CFG_LOG2_SYSBUS_DATA_BYTES = 3;
static const int CFG_SYSBUS_ID_BITS = 5;
static const int CFG_SYSBUS_USER_BITS = 1;

static const int CFG_SYSBUS_DATA_BYTES = (1 << CFG_LOG2_SYSBUS_DATA_BYTES);
static const int CFG_SYSBUS_DATA_BITS = (8 * CFG_SYSBUS_DATA_BYTES);

// @name Vendor IDs defintion.
static const uint16_t VENDOR_GNSSSENSOR = 0x00F1;
static const uint16_t VENDOR_OPTIMITECH = 0x00F2;

// @name Master Device IDs definition:
// Empty master slot device
static const uint16_t MST_DID_EMPTY = 0x7755;
// Ethernet MAC master device.
static const uint16_t GAISLER_ETH_MAC_MASTER = 0x0502;
// Ethernet MAC master debug interface (EDCL).
static const uint16_t GAISLER_ETH_EDCL_MASTER = 0x0503;
// "River" CPU Device ID.
static const uint16_t RISCV_RIVER_CPU = 0x0505;
// "Wasserfall" CPU Device ID.
static const uint16_t RISCV_RIVER_WORKGROUP = 0x0506;
// UART with DMA: Test Access Point (TAP)
static const uint16_t GNSSSENSOR_UART_TAP = 0x050A;
// JTAG Test Access Point (TAP) with SBA interface (DMA without progbuf)
static const uint16_t OPTIMITECH_JTAG_SBA = 0x050B;

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
static const uint16_t OPTIMITECH_UART = 0x007a;
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
// SPI controller (SD-card in SPI mode)
static const uint16_t OPTIMITECH_SPI = 0x0089;
// RIVER debug registers:
static const uint16_t OPTIMITECH_RIVER_DMI = 0x008a;

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

// @brief Map information for the memory mapped device.
class mapinfo_type {
 public:
    mapinfo_type() {
        // Base Address.
        addr_start = 0;
        // Maskable bits of the base address.
        addr_end = 0;
    }

    mapinfo_type(uint64_t addr_start_,
                 uint64_t addr_end_) {
        addr_start = addr_start_;
        addr_end = addr_end_;
    }

    inline bool operator == (const mapinfo_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.addr_start == addr_start
            && rhs.addr_end == addr_end;
        return ret;
    }

    inline mapinfo_type& operator = (const mapinfo_type &rhs) {
        addr_start = rhs.addr_start;
        addr_end = rhs.addr_end;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const mapinfo_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.addr_start, NAME + "_addr_start");
        sc_trace(tf, v.addr_end, NAME + "_addr_end");
    }

    inline friend ostream &operator << (ostream &os,
                                        mapinfo_type const &v) {
        os << "("
        << v.addr_start << ","
        << v.addr_end << ")";
        return os;
    }

 public:
    // Base Address.;
    uint64_t addr_start;
    // Maskable bits of the base address.;
    uint64_t addr_end;
};

// @brief Empty entry value for the map info table
static const mapinfo_type mapinfo_none;

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
        addr_start = 0ull;
        // End of the base address.
        addr_end = 0ull;
        // Vendor ID.
        vid = VENDOR_GNSSSENSOR;
        // Device ID.
        did = SLV_DID_EMPTY;
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
    // Descriptor size in bytes.;
    sc_uint<8> descrsize;
    // Descriptor type.;
    sc_uint<2> descrtype;
    // Base Address.;
    sc_uint<64> addr_start;
    // End of the base address.;
    sc_uint<64> addr_end;
    // Vendor ID.;
    sc_uint<16> vid;
    // Device ID.;
    sc_uint<16> did;
};

// @brief Default config value for empty slot.
static const dev_config_type dev_config_none;

// Burst length size decoder
static const int XSIZE_TOTAL = 8;
// Decoder of the transaction bytes from AXI format to Bytes.
static sc_uint<XSIZE_TOTAL> XSizeToBytes(sc_uint<3> xsize) {
    sc_uint<XSIZE_TOTAL> ret;

    ret = (1 << xsize.to_int());
    return ret;
}

// @brief Normal access success.
// @details Indicates that a normal access has been
// successful. Can also indicate an exclusive access has failed.
static const uint8_t AXI_RESP_OKAY = 0;
// @brief Exclusive access okay.
// @details Indicates that either the read or write
// portion of an exclusive access has been successful.
static const uint8_t AXI_RESP_EXOKAY = 1;
// @brief Slave error.
// @details Used the access has reached the slave successfully,
// but the slave wishes to return an error condition to the originating
// master.
static const uint8_t AXI_RESP_SLVERR = 2;
// @brief Decode error.
// @details Generated, typically by an interconnect component,
// to indicate that there is no slave at the transaction address.
static const uint8_t AXI_RESP_DECERR = 3;

// @brief Fixed address burst operation.
// @details The address is the same for every transfer in the burst
//          (FIFO type)
static const uint8_t AXI_BURST_FIXED = 0;
// @brief Burst operation with address increment.
// @details The address for each transfer in the burst is an increment of
//         the address for the previous transfer. The increment value depends
//        on the size of the transfer.
static const uint8_t AXI_BURST_INCR = 1;
// @brief Burst operation with address increment and wrapping.
// @details A wrapping burst is similar to an incrementing burst, except that
//          the address wraps around to a lower address if an upper address
//          limit is reached
static const uint8_t AXI_BURST_WRAP = 2;
// @}

static const uint8_t ARCACHE_DEVICE_NON_BUFFERABLE = 0x0;
static const uint8_t ARCACHE_WRBACK_READ_ALLOCATE = 0xF;

static const uint8_t AWCACHE_DEVICE_NON_BUFFERABLE = 0x0;
static const uint8_t AWCACHE_WRBACK_WRITE_ALLOCATE = 0xF;

// see table C3-7 Permitted read address control signal combinations
//  
//    read  |  cached  |  unique  |
//     0    |    0     |    *     |    ReadNoSnoop
//     0    |    1     |    0     |    ReadShared
//     0    |    1     |    1     |    ReadMakeUnique
static const uint8_t ARSNOOP_READ_NO_SNOOP = 0x0;
static const uint8_t ARSNOOP_READ_SHARED = 0x1;
static const uint8_t ARSNOOP_READ_MAKE_UNIQUE = 0xC;

// see table C3-8 Permitted read address control signal combinations
//  
//   write  |  cached  |  unique  |
//     1    |    0     |    *     |    WriteNoSnoop
//     1    |    1     |    1     |    WriteLineUnique
//     1    |    1     |    0     |    WriteBack
static const uint8_t AWSNOOP_WRITE_NO_SNOOP = 0x0;
static const uint8_t AWSNOOP_WRITE_LINE_UNIQUE = 0x1;
static const uint8_t AWSNOOP_WRITE_BACK = 0x3;

// see table C3-19
static const uint8_t AC_SNOOP_READ_UNIQUE = 0x7;
static const uint8_t AC_SNOOP_MAKE_INVALID = 0xD;

class axi4_metadata_type {
 public:
    axi4_metadata_type() {
        addr = 0ull;
        // @brief   Burst length.
        // @details This signal indicates the exact number of transfers in
        //          a burst. This changes between AXI3 and AXI4. nastiXLenBits=8 so
        //          this is an AXI4 implementation.
        //              Burst_Length = len[7:0] + 1
        len = 0;
        // @brief   Burst size.
        // @details This signal indicates the size of each transfer
        //          in the burst: 0=1 byte; ..., 6=64 bytes; 7=128 bytes;
        size = 0;
        // @brief   Read response.
        // @details This signal indicates the status of the read transfer.
        // The responses are:
        //      0b00 FIXED - In a fixed burst, the address is the same for every transfer
        //                  in the burst. Typically is used for FIFO.
        //      0b01 INCR - Incrementing. In an incrementing burst, the address for each
        //                  transfer in the burst is an increment of the address for the
        //                  previous transfer. The increment value depends on the size of
        //                  the transfer.
        //      0b10 WRAP - A wrapping burst is similar to an incrementing burst, except
        //                  that the address wraps around to a lower address if an upper address
        //                  limit is reached.
        //      0b11 resrved.
        burst = AXI_BURST_INCR;
        lock = 0;
        cache = 0;
        // @brief   Protection type.
        // @details This signal indicates the privilege and security level
        //          of the transaction, and whether the transaction is a data access
        //          or an instruction access:
        //  [0] :   0 = Unpriviledge access
        //          1 = Priviledge access
        //  [1] :   0 = Secure access
        //          1 = Non-secure access
        //  [2] :   0 = Data access
        //          1 = Instruction access
        prot = 0;
        // @brief   Quality of Service, QoS.
        // @details QoS identifier sent for each read transaction.
        //          Implemented only in AXI4:
        //              0b0000 - default value. Indicates that the interface is
        //                       not participating in any QoS scheme.
        qos = 0;
        // @brief Region identifier.
        // @details Permits a single physical interface on a slave to be used for
        //          multiple logical interfaces. Implemented only in AXI4. This is
        //          similar to the banks implementation in Leon3 without address
        //          decoding.
        region = 0;
    }

    inline bool operator == (const axi4_metadata_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.addr == addr
            && rhs.len == len
            && rhs.size == size
            && rhs.burst == burst
            && rhs.lock == lock
            && rhs.cache == cache
            && rhs.prot == prot
            && rhs.qos == qos
            && rhs.region == region;
        return ret;
    }

    inline axi4_metadata_type& operator = (const axi4_metadata_type &rhs) {
        addr = rhs.addr;
        len = rhs.len;
        size = rhs.size;
        burst = rhs.burst;
        lock = rhs.lock;
        cache = rhs.cache;
        prot = rhs.prot;
        qos = rhs.qos;
        region = rhs.region;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_metadata_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.addr, NAME + "_addr");
        sc_trace(tf, v.len, NAME + "_len");
        sc_trace(tf, v.size, NAME + "_size");
        sc_trace(tf, v.burst, NAME + "_burst");
        sc_trace(tf, v.lock, NAME + "_lock");
        sc_trace(tf, v.cache, NAME + "_cache");
        sc_trace(tf, v.prot, NAME + "_prot");
        sc_trace(tf, v.qos, NAME + "_qos");
        sc_trace(tf, v.region, NAME + "_region");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_metadata_type const &v) {
        os << "("
        << v.addr << ","
        << v.len << ","
        << v.size << ","
        << v.burst << ","
        << v.lock << ","
        << v.cache << ","
        << v.prot << ","
        << v.qos << ","
        << v.region << ")";
        return os;
    }

 public:
    sc_uint<CFG_SYSBUS_ADDR_BITS> addr;
    // @brief   Burst length.;
    // @details This signal indicates the exact number of transfers in;
    //          a burst. This changes between AXI3 and AXI4. nastiXLenBits=8 so;
    //          this is an AXI4 implementation.;
    //              Burst_Length = len[7:0] + 1;
    sc_uint<8> len;
    // @brief   Burst size.;
    // @details This signal indicates the size of each transfer;
    //          in the burst: 0=1 byte; ..., 6=64 bytes; 7=128 bytes;;
    sc_uint<3> size;
    // @brief   Read response.;
    // @details This signal indicates the status of the read transfer.;
    // The responses are:;
    //      0b00 FIXED - In a fixed burst, the address is the same for every transfer;
    //                  in the burst. Typically is used for FIFO.;
    //      0b01 INCR - Incrementing. In an incrementing burst, the address for each;
    //                  transfer in the burst is an increment of the address for the;
    //                  previous transfer. The increment value depends on the size of;
    //                  the transfer.;
    //      0b10 WRAP - A wrapping burst is similar to an incrementing burst, except;
    //                  that the address wraps around to a lower address if an upper address;
    //                  limit is reached.;
    //      0b11 resrved.;
    sc_uint<2> burst;
    bool lock;
    sc_uint<4> cache;
    // @brief   Protection type.;
    // @details This signal indicates the privilege and security level;
    //          of the transaction, and whether the transaction is a data access;
    //          or an instruction access:;
    //  [0] :   0 = Unpriviledge access;
    //          1 = Priviledge access;
    //  [1] :   0 = Secure access;
    //          1 = Non-secure access;
    //  [2] :   0 = Data access;
    //          1 = Instruction access;
    sc_uint<3> prot;
    // @brief   Quality of Service, QoS.;
    // @details QoS identifier sent for each read transaction.;
    //          Implemented only in AXI4:;
    //              0b0000 - default value. Indicates that the interface is;
    //                       not participating in any QoS scheme.;
    sc_uint<4> qos;
    // @brief Region identifier.;
    // @details Permits a single physical interface on a slave to be used for;
    //          multiple logical interfaces. Implemented only in AXI4. This is;
    //          similar to the banks implementation in Leon3 without address;
    //          decoding.;
    sc_uint<4> region;
};

static const axi4_metadata_type META_NONE;


class axi4_master_out_type {
 public:
    axi4_master_out_type() {
        aw_valid = 0;
        aw_bits.addr = 0ull;
        aw_bits.len = 0;
        aw_bits.size = 0;
        aw_bits.burst = AXI_BURST_INCR;
        aw_bits.lock = 0;
        aw_bits.cache = 0;
        aw_bits.prot = 0;
        aw_bits.qos = 0;
        aw_bits.region = 0;
        aw_id = 0;
        aw_user = 0;
        w_valid = 0;
        w_data = 0ull;
        w_last = 0;
        w_strb = 0;
        w_user = 0;
        b_ready = 0;
        ar_valid = 0;
        ar_bits.addr = 0ull;
        ar_bits.len = 0;
        ar_bits.size = 0;
        ar_bits.burst = AXI_BURST_INCR;
        ar_bits.lock = 0;
        ar_bits.cache = 0;
        ar_bits.prot = 0;
        ar_bits.qos = 0;
        ar_bits.region = 0;
        ar_id = 0;
        ar_user = 0;
        r_ready = 0;
    }

    inline bool operator == (const axi4_master_out_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.aw_valid == aw_valid
            && rhs.aw_bits.addr == aw_bits.addr
            && rhs.aw_bits.len == aw_bits.len
            && rhs.aw_bits.size == aw_bits.size
            && rhs.aw_bits.burst == aw_bits.burst
            && rhs.aw_bits.lock == aw_bits.lock
            && rhs.aw_bits.cache == aw_bits.cache
            && rhs.aw_bits.prot == aw_bits.prot
            && rhs.aw_bits.qos == aw_bits.qos
            && rhs.aw_bits.region == aw_bits.region
            && rhs.aw_id == aw_id
            && rhs.aw_user == aw_user
            && rhs.w_valid == w_valid
            && rhs.w_data == w_data
            && rhs.w_last == w_last
            && rhs.w_strb == w_strb
            && rhs.w_user == w_user
            && rhs.b_ready == b_ready
            && rhs.ar_valid == ar_valid
            && rhs.ar_bits.addr == ar_bits.addr
            && rhs.ar_bits.len == ar_bits.len
            && rhs.ar_bits.size == ar_bits.size
            && rhs.ar_bits.burst == ar_bits.burst
            && rhs.ar_bits.lock == ar_bits.lock
            && rhs.ar_bits.cache == ar_bits.cache
            && rhs.ar_bits.prot == ar_bits.prot
            && rhs.ar_bits.qos == ar_bits.qos
            && rhs.ar_bits.region == ar_bits.region
            && rhs.ar_id == ar_id
            && rhs.ar_user == ar_user
            && rhs.r_ready == r_ready;
        return ret;
    }

    inline axi4_master_out_type& operator = (const axi4_master_out_type &rhs) {
        aw_valid = rhs.aw_valid;
        aw_bits.addr = rhs.aw_bits.addr;
        aw_bits.len = rhs.aw_bits.len;
        aw_bits.size = rhs.aw_bits.size;
        aw_bits.burst = rhs.aw_bits.burst;
        aw_bits.lock = rhs.aw_bits.lock;
        aw_bits.cache = rhs.aw_bits.cache;
        aw_bits.prot = rhs.aw_bits.prot;
        aw_bits.qos = rhs.aw_bits.qos;
        aw_bits.region = rhs.aw_bits.region;
        aw_id = rhs.aw_id;
        aw_user = rhs.aw_user;
        w_valid = rhs.w_valid;
        w_data = rhs.w_data;
        w_last = rhs.w_last;
        w_strb = rhs.w_strb;
        w_user = rhs.w_user;
        b_ready = rhs.b_ready;
        ar_valid = rhs.ar_valid;
        ar_bits.addr = rhs.ar_bits.addr;
        ar_bits.len = rhs.ar_bits.len;
        ar_bits.size = rhs.ar_bits.size;
        ar_bits.burst = rhs.ar_bits.burst;
        ar_bits.lock = rhs.ar_bits.lock;
        ar_bits.cache = rhs.ar_bits.cache;
        ar_bits.prot = rhs.ar_bits.prot;
        ar_bits.qos = rhs.ar_bits.qos;
        ar_bits.region = rhs.ar_bits.region;
        ar_id = rhs.ar_id;
        ar_user = rhs.ar_user;
        r_ready = rhs.r_ready;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_master_out_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_valid, NAME + "_aw_valid");
        sc_trace(tf, v.aw_bits.addr, NAME + "_aw_bits_addr");
        sc_trace(tf, v.aw_bits.len, NAME + "_aw_bits_len");
        sc_trace(tf, v.aw_bits.size, NAME + "_aw_bits_size");
        sc_trace(tf, v.aw_bits.burst, NAME + "_aw_bits_burst");
        sc_trace(tf, v.aw_bits.lock, NAME + "_aw_bits_lock");
        sc_trace(tf, v.aw_bits.cache, NAME + "_aw_bits_cache");
        sc_trace(tf, v.aw_bits.prot, NAME + "_aw_bits_prot");
        sc_trace(tf, v.aw_bits.qos, NAME + "_aw_bits_qos");
        sc_trace(tf, v.aw_bits.region, NAME + "_aw_bits_region");
        sc_trace(tf, v.aw_id, NAME + "_aw_id");
        sc_trace(tf, v.aw_user, NAME + "_aw_user");
        sc_trace(tf, v.w_valid, NAME + "_w_valid");
        sc_trace(tf, v.w_data, NAME + "_w_data");
        sc_trace(tf, v.w_last, NAME + "_w_last");
        sc_trace(tf, v.w_strb, NAME + "_w_strb");
        sc_trace(tf, v.w_user, NAME + "_w_user");
        sc_trace(tf, v.b_ready, NAME + "_b_ready");
        sc_trace(tf, v.ar_valid, NAME + "_ar_valid");
        sc_trace(tf, v.ar_bits.addr, NAME + "_ar_bits_addr");
        sc_trace(tf, v.ar_bits.len, NAME + "_ar_bits_len");
        sc_trace(tf, v.ar_bits.size, NAME + "_ar_bits_size");
        sc_trace(tf, v.ar_bits.burst, NAME + "_ar_bits_burst");
        sc_trace(tf, v.ar_bits.lock, NAME + "_ar_bits_lock");
        sc_trace(tf, v.ar_bits.cache, NAME + "_ar_bits_cache");
        sc_trace(tf, v.ar_bits.prot, NAME + "_ar_bits_prot");
        sc_trace(tf, v.ar_bits.qos, NAME + "_ar_bits_qos");
        sc_trace(tf, v.ar_bits.region, NAME + "_ar_bits_region");
        sc_trace(tf, v.ar_id, NAME + "_ar_id");
        sc_trace(tf, v.ar_user, NAME + "_ar_user");
        sc_trace(tf, v.r_ready, NAME + "_r_ready");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_master_out_type const &v) {
        os << "("
        << v.aw_valid << ","
        << v.aw_bits.addr << ","
        << v.aw_bits.len << ","
        << v.aw_bits.size << ","
        << v.aw_bits.burst << ","
        << v.aw_bits.lock << ","
        << v.aw_bits.cache << ","
        << v.aw_bits.prot << ","
        << v.aw_bits.qos << ","
        << v.aw_bits.region << ","
        << v.aw_id << ","
        << v.aw_user << ","
        << v.w_valid << ","
        << v.w_data << ","
        << v.w_last << ","
        << v.w_strb << ","
        << v.w_user << ","
        << v.b_ready << ","
        << v.ar_valid << ","
        << v.ar_bits.addr << ","
        << v.ar_bits.len << ","
        << v.ar_bits.size << ","
        << v.ar_bits.burst << ","
        << v.ar_bits.lock << ","
        << v.ar_bits.cache << ","
        << v.ar_bits.prot << ","
        << v.ar_bits.qos << ","
        << v.ar_bits.region << ","
        << v.ar_id << ","
        << v.ar_user << ","
        << v.r_ready << ")";
        return os;
    }

 public:
    bool aw_valid;
    axi4_metadata_type aw_bits;
    sc_uint<CFG_SYSBUS_ID_BITS> aw_id;
    sc_uint<CFG_SYSBUS_USER_BITS> aw_user;
    bool w_valid;
    sc_uint<CFG_SYSBUS_DATA_BITS> w_data;
    bool w_last;
    sc_uint<CFG_SYSBUS_DATA_BYTES> w_strb;
    sc_uint<CFG_SYSBUS_USER_BITS> w_user;
    bool b_ready;
    bool ar_valid;
    axi4_metadata_type ar_bits;
    sc_uint<CFG_SYSBUS_ID_BITS> ar_id;
    sc_uint<CFG_SYSBUS_USER_BITS> ar_user;
    bool r_ready;
};

// @brief   Master device empty value.
// @warning If the master is not connected to the vector begin vector value
//          MUST BE initialized by this value.
static const axi4_master_out_type axi4_master_out_none;

// @brief Master device input signals.
class axi4_master_in_type {
 public:
    axi4_master_in_type() {
        aw_ready = 0;
        w_ready = 0;
        b_valid = 0;
        b_resp = 0;
        b_id = 0;
        b_user = 0;
        ar_ready = 0;
        r_valid = 0;
        r_resp = 0;
        r_data = 0ull;
        r_last = 0;
        r_id = 0;
        r_user = 0;
    }

    inline bool operator == (const axi4_master_in_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.aw_ready == aw_ready
            && rhs.w_ready == w_ready
            && rhs.b_valid == b_valid
            && rhs.b_resp == b_resp
            && rhs.b_id == b_id
            && rhs.b_user == b_user
            && rhs.ar_ready == ar_ready
            && rhs.r_valid == r_valid
            && rhs.r_resp == r_resp
            && rhs.r_data == r_data
            && rhs.r_last == r_last
            && rhs.r_id == r_id
            && rhs.r_user == r_user;
        return ret;
    }

    inline axi4_master_in_type& operator = (const axi4_master_in_type &rhs) {
        aw_ready = rhs.aw_ready;
        w_ready = rhs.w_ready;
        b_valid = rhs.b_valid;
        b_resp = rhs.b_resp;
        b_id = rhs.b_id;
        b_user = rhs.b_user;
        ar_ready = rhs.ar_ready;
        r_valid = rhs.r_valid;
        r_resp = rhs.r_resp;
        r_data = rhs.r_data;
        r_last = rhs.r_last;
        r_id = rhs.r_id;
        r_user = rhs.r_user;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_master_in_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_ready, NAME + "_aw_ready");
        sc_trace(tf, v.w_ready, NAME + "_w_ready");
        sc_trace(tf, v.b_valid, NAME + "_b_valid");
        sc_trace(tf, v.b_resp, NAME + "_b_resp");
        sc_trace(tf, v.b_id, NAME + "_b_id");
        sc_trace(tf, v.b_user, NAME + "_b_user");
        sc_trace(tf, v.ar_ready, NAME + "_ar_ready");
        sc_trace(tf, v.r_valid, NAME + "_r_valid");
        sc_trace(tf, v.r_resp, NAME + "_r_resp");
        sc_trace(tf, v.r_data, NAME + "_r_data");
        sc_trace(tf, v.r_last, NAME + "_r_last");
        sc_trace(tf, v.r_id, NAME + "_r_id");
        sc_trace(tf, v.r_user, NAME + "_r_user");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_master_in_type const &v) {
        os << "("
        << v.aw_ready << ","
        << v.w_ready << ","
        << v.b_valid << ","
        << v.b_resp << ","
        << v.b_id << ","
        << v.b_user << ","
        << v.ar_ready << ","
        << v.r_valid << ","
        << v.r_resp << ","
        << v.r_data << ","
        << v.r_last << ","
        << v.r_id << ","
        << v.r_user << ")";
        return os;
    }

 public:
    bool aw_ready;
    bool w_ready;
    bool b_valid;
    sc_uint<2> b_resp;
    sc_uint<CFG_SYSBUS_ID_BITS> b_id;
    sc_uint<CFG_SYSBUS_USER_BITS> b_user;
    bool ar_ready;
    bool r_valid;
    sc_uint<2> r_resp;
    sc_uint<CFG_SYSBUS_DATA_BITS> r_data;
    bool r_last;
    sc_uint<CFG_SYSBUS_ID_BITS> r_id;
    sc_uint<CFG_SYSBUS_USER_BITS> r_user;
};

static const axi4_master_in_type axi4_master_in_none;


class axi4_slave_in_type {
 public:
    axi4_slave_in_type() {
        aw_valid = 0;
        aw_bits.addr = 0ull;
        aw_bits.len = 0;
        aw_bits.size = 0;
        aw_bits.burst = AXI_BURST_INCR;
        aw_bits.lock = 0;
        aw_bits.cache = 0;
        aw_bits.prot = 0;
        aw_bits.qos = 0;
        aw_bits.region = 0;
        aw_id = 0;
        aw_user = 0;
        w_valid = 0;
        w_data = 0ull;
        w_last = 0;
        w_strb = 0;
        w_user = 0;
        b_ready = 0;
        ar_valid = 0;
        ar_bits.addr = 0ull;
        ar_bits.len = 0;
        ar_bits.size = 0;
        ar_bits.burst = AXI_BURST_INCR;
        ar_bits.lock = 0;
        ar_bits.cache = 0;
        ar_bits.prot = 0;
        ar_bits.qos = 0;
        ar_bits.region = 0;
        ar_id = 0;
        ar_user = 0;
        r_ready = 0;
    }

    inline bool operator == (const axi4_slave_in_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.aw_valid == aw_valid
            && rhs.aw_bits.addr == aw_bits.addr
            && rhs.aw_bits.len == aw_bits.len
            && rhs.aw_bits.size == aw_bits.size
            && rhs.aw_bits.burst == aw_bits.burst
            && rhs.aw_bits.lock == aw_bits.lock
            && rhs.aw_bits.cache == aw_bits.cache
            && rhs.aw_bits.prot == aw_bits.prot
            && rhs.aw_bits.qos == aw_bits.qos
            && rhs.aw_bits.region == aw_bits.region
            && rhs.aw_id == aw_id
            && rhs.aw_user == aw_user
            && rhs.w_valid == w_valid
            && rhs.w_data == w_data
            && rhs.w_last == w_last
            && rhs.w_strb == w_strb
            && rhs.w_user == w_user
            && rhs.b_ready == b_ready
            && rhs.ar_valid == ar_valid
            && rhs.ar_bits.addr == ar_bits.addr
            && rhs.ar_bits.len == ar_bits.len
            && rhs.ar_bits.size == ar_bits.size
            && rhs.ar_bits.burst == ar_bits.burst
            && rhs.ar_bits.lock == ar_bits.lock
            && rhs.ar_bits.cache == ar_bits.cache
            && rhs.ar_bits.prot == ar_bits.prot
            && rhs.ar_bits.qos == ar_bits.qos
            && rhs.ar_bits.region == ar_bits.region
            && rhs.ar_id == ar_id
            && rhs.ar_user == ar_user
            && rhs.r_ready == r_ready;
        return ret;
    }

    inline axi4_slave_in_type& operator = (const axi4_slave_in_type &rhs) {
        aw_valid = rhs.aw_valid;
        aw_bits.addr = rhs.aw_bits.addr;
        aw_bits.len = rhs.aw_bits.len;
        aw_bits.size = rhs.aw_bits.size;
        aw_bits.burst = rhs.aw_bits.burst;
        aw_bits.lock = rhs.aw_bits.lock;
        aw_bits.cache = rhs.aw_bits.cache;
        aw_bits.prot = rhs.aw_bits.prot;
        aw_bits.qos = rhs.aw_bits.qos;
        aw_bits.region = rhs.aw_bits.region;
        aw_id = rhs.aw_id;
        aw_user = rhs.aw_user;
        w_valid = rhs.w_valid;
        w_data = rhs.w_data;
        w_last = rhs.w_last;
        w_strb = rhs.w_strb;
        w_user = rhs.w_user;
        b_ready = rhs.b_ready;
        ar_valid = rhs.ar_valid;
        ar_bits.addr = rhs.ar_bits.addr;
        ar_bits.len = rhs.ar_bits.len;
        ar_bits.size = rhs.ar_bits.size;
        ar_bits.burst = rhs.ar_bits.burst;
        ar_bits.lock = rhs.ar_bits.lock;
        ar_bits.cache = rhs.ar_bits.cache;
        ar_bits.prot = rhs.ar_bits.prot;
        ar_bits.qos = rhs.ar_bits.qos;
        ar_bits.region = rhs.ar_bits.region;
        ar_id = rhs.ar_id;
        ar_user = rhs.ar_user;
        r_ready = rhs.r_ready;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_slave_in_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_valid, NAME + "_aw_valid");
        sc_trace(tf, v.aw_bits.addr, NAME + "_aw_bits_addr");
        sc_trace(tf, v.aw_bits.len, NAME + "_aw_bits_len");
        sc_trace(tf, v.aw_bits.size, NAME + "_aw_bits_size");
        sc_trace(tf, v.aw_bits.burst, NAME + "_aw_bits_burst");
        sc_trace(tf, v.aw_bits.lock, NAME + "_aw_bits_lock");
        sc_trace(tf, v.aw_bits.cache, NAME + "_aw_bits_cache");
        sc_trace(tf, v.aw_bits.prot, NAME + "_aw_bits_prot");
        sc_trace(tf, v.aw_bits.qos, NAME + "_aw_bits_qos");
        sc_trace(tf, v.aw_bits.region, NAME + "_aw_bits_region");
        sc_trace(tf, v.aw_id, NAME + "_aw_id");
        sc_trace(tf, v.aw_user, NAME + "_aw_user");
        sc_trace(tf, v.w_valid, NAME + "_w_valid");
        sc_trace(tf, v.w_data, NAME + "_w_data");
        sc_trace(tf, v.w_last, NAME + "_w_last");
        sc_trace(tf, v.w_strb, NAME + "_w_strb");
        sc_trace(tf, v.w_user, NAME + "_w_user");
        sc_trace(tf, v.b_ready, NAME + "_b_ready");
        sc_trace(tf, v.ar_valid, NAME + "_ar_valid");
        sc_trace(tf, v.ar_bits.addr, NAME + "_ar_bits_addr");
        sc_trace(tf, v.ar_bits.len, NAME + "_ar_bits_len");
        sc_trace(tf, v.ar_bits.size, NAME + "_ar_bits_size");
        sc_trace(tf, v.ar_bits.burst, NAME + "_ar_bits_burst");
        sc_trace(tf, v.ar_bits.lock, NAME + "_ar_bits_lock");
        sc_trace(tf, v.ar_bits.cache, NAME + "_ar_bits_cache");
        sc_trace(tf, v.ar_bits.prot, NAME + "_ar_bits_prot");
        sc_trace(tf, v.ar_bits.qos, NAME + "_ar_bits_qos");
        sc_trace(tf, v.ar_bits.region, NAME + "_ar_bits_region");
        sc_trace(tf, v.ar_id, NAME + "_ar_id");
        sc_trace(tf, v.ar_user, NAME + "_ar_user");
        sc_trace(tf, v.r_ready, NAME + "_r_ready");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_slave_in_type const &v) {
        os << "("
        << v.aw_valid << ","
        << v.aw_bits.addr << ","
        << v.aw_bits.len << ","
        << v.aw_bits.size << ","
        << v.aw_bits.burst << ","
        << v.aw_bits.lock << ","
        << v.aw_bits.cache << ","
        << v.aw_bits.prot << ","
        << v.aw_bits.qos << ","
        << v.aw_bits.region << ","
        << v.aw_id << ","
        << v.aw_user << ","
        << v.w_valid << ","
        << v.w_data << ","
        << v.w_last << ","
        << v.w_strb << ","
        << v.w_user << ","
        << v.b_ready << ","
        << v.ar_valid << ","
        << v.ar_bits.addr << ","
        << v.ar_bits.len << ","
        << v.ar_bits.size << ","
        << v.ar_bits.burst << ","
        << v.ar_bits.lock << ","
        << v.ar_bits.cache << ","
        << v.ar_bits.prot << ","
        << v.ar_bits.qos << ","
        << v.ar_bits.region << ","
        << v.ar_id << ","
        << v.ar_user << ","
        << v.r_ready << ")";
        return os;
    }

 public:
    bool aw_valid;
    axi4_metadata_type aw_bits;
    sc_uint<CFG_SYSBUS_ID_BITS> aw_id;
    sc_uint<CFG_SYSBUS_USER_BITS> aw_user;
    bool w_valid;
    sc_uint<CFG_SYSBUS_DATA_BITS> w_data;
    bool w_last;
    sc_uint<CFG_SYSBUS_DATA_BYTES> w_strb;
    sc_uint<CFG_SYSBUS_USER_BITS> w_user;
    bool b_ready;
    bool ar_valid;
    axi4_metadata_type ar_bits;
    sc_uint<CFG_SYSBUS_ID_BITS> ar_id;
    sc_uint<CFG_SYSBUS_USER_BITS> ar_user;
    bool r_ready;
};

static const axi4_slave_in_type axi4_slave_in_none;

class axi4_slave_out_type {
 public:
    axi4_slave_out_type() {
        aw_ready = 0;
        w_ready = 0;
        b_valid = 0;
        b_resp = 0;
        b_id = 0;
        b_user = 0;
        ar_ready = 0;
        r_valid = 0;
        r_resp = 0;
        r_data = 0ull;
        r_last = 0;
        r_id = 0;
        r_user = 0;
    }

    inline bool operator == (const axi4_slave_out_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.aw_ready == aw_ready
            && rhs.w_ready == w_ready
            && rhs.b_valid == b_valid
            && rhs.b_resp == b_resp
            && rhs.b_id == b_id
            && rhs.b_user == b_user
            && rhs.ar_ready == ar_ready
            && rhs.r_valid == r_valid
            && rhs.r_resp == r_resp
            && rhs.r_data == r_data
            && rhs.r_last == r_last
            && rhs.r_id == r_id
            && rhs.r_user == r_user;
        return ret;
    }

    inline axi4_slave_out_type& operator = (const axi4_slave_out_type &rhs) {
        aw_ready = rhs.aw_ready;
        w_ready = rhs.w_ready;
        b_valid = rhs.b_valid;
        b_resp = rhs.b_resp;
        b_id = rhs.b_id;
        b_user = rhs.b_user;
        ar_ready = rhs.ar_ready;
        r_valid = rhs.r_valid;
        r_resp = rhs.r_resp;
        r_data = rhs.r_data;
        r_last = rhs.r_last;
        r_id = rhs.r_id;
        r_user = rhs.r_user;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_slave_out_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_ready, NAME + "_aw_ready");
        sc_trace(tf, v.w_ready, NAME + "_w_ready");
        sc_trace(tf, v.b_valid, NAME + "_b_valid");
        sc_trace(tf, v.b_resp, NAME + "_b_resp");
        sc_trace(tf, v.b_id, NAME + "_b_id");
        sc_trace(tf, v.b_user, NAME + "_b_user");
        sc_trace(tf, v.ar_ready, NAME + "_ar_ready");
        sc_trace(tf, v.r_valid, NAME + "_r_valid");
        sc_trace(tf, v.r_resp, NAME + "_r_resp");
        sc_trace(tf, v.r_data, NAME + "_r_data");
        sc_trace(tf, v.r_last, NAME + "_r_last");
        sc_trace(tf, v.r_id, NAME + "_r_id");
        sc_trace(tf, v.r_user, NAME + "_r_user");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_slave_out_type const &v) {
        os << "("
        << v.aw_ready << ","
        << v.w_ready << ","
        << v.b_valid << ","
        << v.b_resp << ","
        << v.b_id << ","
        << v.b_user << ","
        << v.ar_ready << ","
        << v.r_valid << ","
        << v.r_resp << ","
        << v.r_data << ","
        << v.r_last << ","
        << v.r_id << ","
        << v.r_user << ")";
        return os;
    }

 public:
    bool aw_ready;
    bool w_ready;
    bool b_valid;
    sc_uint<2> b_resp;
    sc_uint<CFG_SYSBUS_ID_BITS> b_id;
    sc_uint<CFG_SYSBUS_USER_BITS> b_user;
    bool ar_ready;
    bool r_valid;
    sc_uint<2> r_resp;
    sc_uint<CFG_SYSBUS_DATA_BITS> r_data;
    bool r_last;
    sc_uint<CFG_SYSBUS_ID_BITS> r_id;
    sc_uint<CFG_SYSBUS_USER_BITS> r_user;
};

static const axi4_slave_out_type axi4_slave_out_none;


class apb_in_type {
 public:
    apb_in_type() {
        paddr = 0;
        pprot = 0;
        pselx = 0;
        penable = 0;
        pwrite = 0;
        pwdata = 0;
        pstrb = 0;
    }

    inline bool operator == (const apb_in_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.paddr == paddr
            && rhs.pprot == pprot
            && rhs.pselx == pselx
            && rhs.penable == penable
            && rhs.pwrite == pwrite
            && rhs.pwdata == pwdata
            && rhs.pstrb == pstrb;
        return ret;
    }

    inline apb_in_type& operator = (const apb_in_type &rhs) {
        paddr = rhs.paddr;
        pprot = rhs.pprot;
        pselx = rhs.pselx;
        penable = rhs.penable;
        pwrite = rhs.pwrite;
        pwdata = rhs.pwdata;
        pstrb = rhs.pstrb;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const apb_in_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.paddr, NAME + "_paddr");
        sc_trace(tf, v.pprot, NAME + "_pprot");
        sc_trace(tf, v.pselx, NAME + "_pselx");
        sc_trace(tf, v.penable, NAME + "_penable");
        sc_trace(tf, v.pwrite, NAME + "_pwrite");
        sc_trace(tf, v.pwdata, NAME + "_pwdata");
        sc_trace(tf, v.pstrb, NAME + "_pstrb");
    }

    inline friend ostream &operator << (ostream &os,
                                        apb_in_type const &v) {
        os << "("
        << v.paddr << ","
        << v.pprot << ","
        << v.pselx << ","
        << v.penable << ","
        << v.pwrite << ","
        << v.pwdata << ","
        << v.pstrb << ")";
        return os;
    }

 public:
    sc_uint<32> paddr;
    sc_uint<3> pprot;
    bool pselx;
    bool penable;
    bool pwrite;
    sc_uint<32> pwdata;
    sc_uint<4> pstrb;
};

static const apb_in_type apb_in_none;

class apb_out_type {
 public:
    apb_out_type() {
        pready = 0;
        prdata = 0;
        pslverr = 0;
    }

    inline bool operator == (const apb_out_type &rhs) const {
        bool ret = true;
        ret = ret
            && rhs.pready == pready
            && rhs.prdata == prdata
            && rhs.pslverr == pslverr;
        return ret;
    }

    inline apb_out_type& operator = (const apb_out_type &rhs) {
        pready = rhs.pready;
        prdata = rhs.prdata;
        pslverr = rhs.pslverr;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const apb_out_type&v,
                                const std::string &NAME) {
        sc_trace(tf, v.pready, NAME + "_pready");
        sc_trace(tf, v.prdata, NAME + "_prdata");
        sc_trace(tf, v.pslverr, NAME + "_pslverr");
    }

    inline friend ostream &operator << (ostream &os,
                                        apb_out_type const &v) {
        os << "("
        << v.pready << ","
        << v.prdata << ","
        << v.pslverr << ")";
        return os;
    }

 public:
    bool pready;                                            // when 1 it breaks callback to functional model
    sc_uint<32> prdata;
    bool pslverr;
};

static const apb_out_type apb_out_none;

}  // namespace debugger

