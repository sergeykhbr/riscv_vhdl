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
package types_amba_pkg;


localparam int CFG_SYSBUS_ADDR_BITS = 48;
localparam int CFG_LOG2_SYSBUS_DATA_BYTES = 3;
localparam int CFG_SYSBUS_ID_BITS = 5;
localparam int CFG_SYSBUS_USER_BITS = 1;

localparam int CFG_SYSBUS_DATA_BYTES = (2**CFG_LOG2_SYSBUS_DATA_BYTES);
localparam int CFG_SYSBUS_DATA_BITS = (8 * CFG_SYSBUS_DATA_BYTES);

// @name Vendor IDs defintion.
localparam bit [15:0] VENDOR_GNSSSENSOR = 16'h00f1;
localparam bit [15:0] VENDOR_OPTIMITECH = 16'h00f2;

// @name Master Device IDs definition:
// Empty master slot device
localparam bit [15:0] MST_DID_EMPTY = 16'h7755;
// Ethernet MAC master device.
localparam bit [15:0] GAISLER_ETH_MAC_MASTER = 16'h0502;
// Ethernet MAC master debug interface (EDCL).
localparam bit [15:0] GAISLER_ETH_EDCL_MASTER = 16'h0503;
// "River" CPU Device ID.
localparam bit [15:0] RISCV_RIVER_CPU = 16'h0505;
// "Wasserfall" CPU Device ID.
localparam bit [15:0] RISCV_RIVER_WORKGROUP = 16'h0506;
// UART with DMA: Test Access Point (TAP)
localparam bit [15:0] GNSSSENSOR_UART_TAP = 16'h050a;
// JTAG Test Access Point (TAP) with SBA interface (DMA without progbuf)
localparam bit [15:0] OPTIMITECH_JTAG_SBA = 16'h050b;

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
localparam bit [15:0] OPTIMITECH_UART = 16'h007a;
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
// SPI controller (SD-card in SPI mode)
localparam bit [15:0] OPTIMITECH_SPI = 16'h0089;
// RIVER debug registers:
localparam bit [15:0] OPTIMITECH_RIVER_DMI = 16'h008a;

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

// @brief Map information for the memory mapped device.
typedef struct {
    // Base Address.;
    longint unsigned addr_start;
    // Maskable bits of the base address.;
    longint unsigned addr_end;
} mapinfo_type;

// @brief Empty entry value for the map info table
const mapinfo_type mapinfo_none = '{
    '0,  // addr_start
    '0  // addr_end
};

// @brief   Plug-n-play descriptor structure for connected device.
// @details Each device must generates this datatype output that
//          is connected directly to the 'pnp' slave module on system bus.
typedef struct {
    // Descriptor size in bytes.;
    logic [7:0] descrsize;
    // Descriptor type.;
    logic [1:0] descrtype;
    // Base Address.;
    logic [63:0] addr_start;
    // End of the base address.;
    logic [63:0] addr_end;
    // Vendor ID.;
    logic [15:0] vid;
    // Device ID.;
    logic [15:0] did;
} dev_config_type;

// @brief Default config value for empty slot.
const dev_config_type dev_config_none = '{
    PNP_CFG_DEV_DESCR_BYTES,  // descrsize
    PNP_CFG_TYPE_SLAVE,  // descrtype
    '0,  // addr_start
    '0,  // addr_end
    VENDOR_GNSSSENSOR,  // vid
    SLV_DID_EMPTY  // did
};

// Burst length size decoder
localparam int XSIZE_TOTAL = 8;
// Decoder of the transaction bytes from AXI format to Bytes.
function automatic logic [XSIZE_TOTAL-1:0] XSizeToBytes(input logic [2:0] xsize);
logic [XSIZE_TOTAL-1:0] ret;
begin
    ret = (1 << int'(xsize));
    return ret;
end
endfunction: XSizeToBytes


// @brief Normal access success.
// @details Indicates that a normal access has been
// successful. Can also indicate an exclusive access has failed.
localparam bit [1:0] AXI_RESP_OKAY = 2'h0;
// @brief Exclusive access okay.
// @details Indicates that either the read or write
// portion of an exclusive access has been successful.
localparam bit [1:0] AXI_RESP_EXOKAY = 2'h1;
// @brief Slave error.
// @details Used the access has reached the slave successfully,
// but the slave wishes to return an error condition to the originating
// master.
localparam bit [1:0] AXI_RESP_SLVERR = 2'h2;
// @brief Decode error.
// @details Generated, typically by an interconnect component,
// to indicate that there is no slave at the transaction address.
localparam bit [1:0] AXI_RESP_DECERR = 2'h3;

// @brief Fixed address burst operation.
// @details The address is the same for every transfer in the burst
//          (FIFO type)
localparam bit [1:0] AXI_BURST_FIXED = 2'h0;
// @brief Burst operation with address increment.
// @details The address for each transfer in the burst is an increment of
//         the address for the previous transfer. The increment value depends
//        on the size of the transfer.
localparam bit [1:0] AXI_BURST_INCR = 2'h1;
// @brief Burst operation with address increment and wrapping.
// @details A wrapping burst is similar to an incrementing burst, except that
//          the address wraps around to a lower address if an upper address
//          limit is reached
localparam bit [1:0] AXI_BURST_WRAP = 2'h2;
// @}

localparam bit [3:0] ARCACHE_DEVICE_NON_BUFFERABLE = 4'h0;
localparam bit [3:0] ARCACHE_WRBACK_READ_ALLOCATE = 4'hf;

localparam bit [3:0] AWCACHE_DEVICE_NON_BUFFERABLE = 4'h0;
localparam bit [3:0] AWCACHE_WRBACK_WRITE_ALLOCATE = 4'hf;

// see table C3-7 Permitted read address control signal combinations
//  
//    read  |  cached  |  unique  |
//     0    |    0     |    *     |    ReadNoSnoop
//     0    |    1     |    0     |    ReadShared
//     0    |    1     |    1     |    ReadMakeUnique
localparam bit [3:0] ARSNOOP_READ_NO_SNOOP = 4'h0;
localparam bit [3:0] ARSNOOP_READ_SHARED = 4'h1;
localparam bit [3:0] ARSNOOP_READ_MAKE_UNIQUE = 4'hc;

// see table C3-8 Permitted read address control signal combinations
//  
//   write  |  cached  |  unique  |
//     1    |    0     |    *     |    WriteNoSnoop
//     1    |    1     |    1     |    WriteLineUnique
//     1    |    1     |    0     |    WriteBack
localparam bit [2:0] AWSNOOP_WRITE_NO_SNOOP = 3'h0;
localparam bit [2:0] AWSNOOP_WRITE_LINE_UNIQUE = 3'h1;
localparam bit [2:0] AWSNOOP_WRITE_BACK = 3'h3;

// see table C3-19
localparam bit [3:0] AC_SNOOP_READ_UNIQUE = 4'h7;
localparam bit [3:0] AC_SNOOP_MAKE_INVALID = 4'hd;

typedef struct {
    logic [CFG_SYSBUS_ADDR_BITS-1:0] addr;
    // @brief   Burst length.;
    // @details This signal indicates the exact number of transfers in;
    //          a burst. This changes between AXI3 and AXI4. nastiXLenBits=8 so;
    //          this is an AXI4 implementation.;
    //              Burst_Length = len[7:0] + 1;
    logic [7:0] len;
    // @brief   Burst size.;
    // @details This signal indicates the size of each transfer;
    //          in the burst: 0=1 byte; ..., 6=64 bytes; 7=128 bytes;;
    logic [2:0] size;
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
    logic [1:0] burst;
    logic lock;
    logic [3:0] cache;
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
    logic [2:0] prot;
    // @brief   Quality of Service, QoS.;
    // @details QoS identifier sent for each read transaction.;
    //          Implemented only in AXI4:;
    //              0b0000 - default value. Indicates that the interface is;
    //                       not participating in any QoS scheme.;
    logic [3:0] qos;
    // @brief Region identifier.;
    // @details Permits a single physical interface on a slave to be used for;
    //          multiple logical interfaces. Implemented only in AXI4. This is;
    //          similar to the banks implementation in Leon3 without address;
    //          decoding.;
    logic [3:0] region;
} axi4_metadata_type;

const axi4_metadata_type META_NONE = '{
    '0,  // addr
    '0,  // len
    '0,  // size
    AXI_BURST_INCR,  // burst
    1'h0,  // lock
    '0,  // cache
    '0,  // prot
    '0,  // qos
    '0  // region
};


typedef struct {
    logic aw_valid;
    axi4_metadata_type aw_bits;
    logic [CFG_SYSBUS_ID_BITS-1:0] aw_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] aw_user;
    logic w_valid;
    logic [CFG_SYSBUS_DATA_BITS-1:0] w_data;
    logic w_last;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] w_strb;
    logic [CFG_SYSBUS_USER_BITS-1:0] w_user;
    logic b_ready;
    logic ar_valid;
    axi4_metadata_type ar_bits;
    logic [CFG_SYSBUS_ID_BITS-1:0] ar_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] ar_user;
    logic r_ready;
} axi4_master_out_type;

// @brief   Master device empty value.
// @warning If the master is not connected to the vector begin vector value
//          MUST BE initialized by this value.
const axi4_master_out_type axi4_master_out_none = '{
    1'h0,  // aw_valid
    META_NONE,  // aw_bits
    '0,  // aw_id
    1'h0,  // aw_user
    1'h0,  // w_valid
    '0,  // w_data
    1'h0,  // w_last
    '0,  // w_strb
    1'h0,  // w_user
    1'h0,  // b_ready
    1'h0,  // ar_valid
    META_NONE,  // ar_bits
    '0,  // ar_id
    1'h0,  // ar_user
    1'h0  // r_ready
};

// @brief Master device input signals.
typedef struct {
    logic aw_ready;
    logic w_ready;
    logic b_valid;
    logic [1:0] b_resp;
    logic [CFG_SYSBUS_ID_BITS-1:0] b_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] b_user;
    logic ar_ready;
    logic r_valid;
    logic [1:0] r_resp;
    logic [CFG_SYSBUS_DATA_BITS-1:0] r_data;
    logic r_last;
    logic [CFG_SYSBUS_ID_BITS-1:0] r_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] r_user;
} axi4_master_in_type;

const axi4_master_in_type axi4_master_in_none = '{
    1'h0,  // aw_ready
    1'h0,  // w_ready
    1'h0,  // b_valid
    '0,  // b_resp
    '0,  // b_id
    1'h0,  // b_user
    1'h0,  // ar_ready
    1'h0,  // r_valid
    '0,  // r_resp
    '0,  // r_data
    1'h0,  // r_last
    '0,  // r_id
    1'h0  // r_user
};


typedef struct {
    logic aw_valid;
    axi4_metadata_type aw_bits;
    logic [CFG_SYSBUS_ID_BITS-1:0] aw_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] aw_user;
    logic w_valid;
    logic [CFG_SYSBUS_DATA_BITS-1:0] w_data;
    logic w_last;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] w_strb;
    logic [CFG_SYSBUS_USER_BITS-1:0] w_user;
    logic b_ready;
    logic ar_valid;
    axi4_metadata_type ar_bits;
    logic [CFG_SYSBUS_ID_BITS-1:0] ar_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] ar_user;
    logic r_ready;
} axi4_slave_in_type;

const axi4_slave_in_type axi4_slave_in_none = '{
    1'h0,  // aw_valid
    META_NONE,  // aw_bits
    '0,  // aw_id
    1'h0,  // aw_user
    1'h0,  // w_valid
    '0,  // w_data
    1'h0,  // w_last
    '0,  // w_strb
    1'h0,  // w_user
    1'h0,  // b_ready
    1'h0,  // ar_valid
    META_NONE,  // ar_bits
    '0,  // ar_id
    1'h0,  // ar_user
    1'h0  // r_ready
};

typedef struct {
    logic aw_ready;
    logic w_ready;
    logic b_valid;
    logic [1:0] b_resp;
    logic [CFG_SYSBUS_ID_BITS-1:0] b_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] b_user;
    logic ar_ready;
    logic r_valid;
    logic [1:0] r_resp;
    logic [CFG_SYSBUS_DATA_BITS-1:0] r_data;
    logic r_last;
    logic [CFG_SYSBUS_ID_BITS-1:0] r_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] r_user;
} axi4_slave_out_type;

const axi4_slave_out_type axi4_slave_out_none = '{
    1'h0,  // aw_ready
    1'h0,  // w_ready
    1'h0,  // b_valid
    '0,  // b_resp
    '0,  // b_id
    1'h0,  // b_user
    1'h0,  // ar_ready
    1'h0,  // r_valid
    '0,  // r_resp
    '0,  // r_data
    1'h0,  // r_last
    '0,  // r_id
    1'h0  // r_user
};


typedef struct {
    logic [31:0] paddr;
    logic [2:0] pprot;
    logic pselx;
    logic penable;
    logic pwrite;
    logic [31:0] pwdata;
    logic [3:0] pstrb;
} apb_in_type;

const apb_in_type apb_in_none = '{
    '0,  // paddr
    '0,  // pprot
    1'h0,  // pselx
    1'h0,  // penable
    1'h0,  // pwrite
    '0,  // pwdata
    '0  // pstrb
};

typedef struct {
    logic pready;                                           // when 1 it breaks callback to functional model
    logic [31:0] prdata;
    logic pslverr;
} apb_out_type;

const apb_out_type apb_out_none = '{
    1'h0,  // pready
    '0,  // prdata
    1'h0  // pslverr
};

endpackage: types_amba_pkg
