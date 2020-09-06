--!
--! Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;

package types_amba4 is

constant CFG_SYSBUS_ADDR_BITS       : integer := 32;
constant CFG_LOG2_SYSBUS_DATA_BYTES : integer := 3;
constant CFG_SYSBUS_ID_BITS         : integer := 5;
constant CFG_SYSBUS_USER_BITS       : integer := 1;

constant CFG_SYSBUS_DATA_BYTES   : integer := (2**CFG_LOG2_SYSBUS_DATA_BYTES);
constant CFG_SYSBUS_DATA_BITS    : integer := 8*CFG_SYSBUS_DATA_BYTES;


--! Definition of number of bits in address bus per one data transaction.
constant CFG_SYSBUS_ADDR_OFFSET  : integer := log2(CFG_SYSBUS_DATA_BYTES);
--! @brief Number of address bits used for device addressing. 
--! @details Default is 12 bits = 4 KB of address space minimum per each 
--!          mapped device.
constant CFG_SYSBUS_CFG_ADDR_BITS : integer := CFG_SYSBUS_ADDR_BITS-12;
--! @brief Global alignment is set 32 bits.
constant CFG_ALIGN_BYTES         : integer := 4;
--! @brief  Number of parallel access to the atomic data.
constant CFG_WORDS_ON_BUS        : integer := CFG_SYSBUS_DATA_BYTES/CFG_ALIGN_BYTES;
--! @}

--! @name   AXI Response values
--! @brief  AMBA 4.0 specified response types from a slave device.
--! @{

--! @brief Normal access success. 
--! @details Indicates that a normal access has been
--! successful. Can also indicate an exclusive access has failed. 
constant AXI_RESP_OKAY     : std_logic_vector(1 downto 0) := "00";
--! @brief Exclusive access okay. 
--! @details Indicates that either the read or write
--! portion of an exclusive access has been successful.
constant AXI_RESP_EXOKAY   : std_logic_vector(1 downto 0) := "01";
--! @brief Slave error. 
--! @details Used when the access has reached the slave successfully,
--! but the slave wishes to return an error condition to the originating
--! master.
constant AXI_RESP_SLVERR   : std_logic_vector(1 downto 0) := "10";
--! @brief Decode error. 
--! @details Generated, typically by an interconnect component,
--! to indicate that there is no slave at the transaction address.
constant AXI_RESP_DECERR   : std_logic_vector(1 downto 0) := "11";
--! @}

--! @name   AXI burst request type.
--! @brief  AMBA 4.0 specified burst operation request types.
--! @{

--! @brief Fixed address burst operation.
--! @details The address is the same for every transfer in the burst 
--!          (FIFO type)
constant AXI_BURST_FIXED   : std_logic_vector(1 downto 0) := "00";
--! @brief Burst operation with address increment.
--! @details The address for each transfer in the burst is an increment of
--!        the address for the previous transfer. The increment value depends 
--!        on the size of the transfer.
constant AXI_BURST_INCR    : std_logic_vector(1 downto 0) := "01";
--! @brief Burst operation with address increment and wrapping.
--! @details A wrapping burst is similar to an incrementing burst, except that
--!          the address wraps around to a lower address if an upper address 
--!          limit is reached
constant AXI_BURST_WRAP    : std_logic_vector(1 downto 0) := "10";
--! @}

--! @name Vendor IDs defintion.
--! @{

--! GNSS Sensor Ltd. vendor identificator.
constant VENDOR_GNSSSENSOR        : std_logic_vector(15 downto 0) := X"00F1"; 
--! @}

--! @name Master Device IDs definition:
--! @{

--! Empty master slot device
constant MST_DID_EMPTY            : std_logic_vector(15 downto 0) := X"7755";
--! RISC-V "Rocket-chip" core Cached TileLink master device.
constant RISCV_CACHED_TILELINK    : std_logic_vector(15 downto 0) := X"0500";
--! RISC-V "Rocket-chip" core Uncached TileLink master device.
constant RISCV_UNCACHED_TILELINK  : std_logic_vector(15 downto 0) := X"0501";
--! Ethernet MAC master device.
constant GAISLER_ETH_MAC_MASTER   : std_logic_vector(15 downto 0) := X"0502";
--! Ethernet MAC master debug interface (EDCL).
constant GAISLER_ETH_EDCL_MASTER  : std_logic_vector(15 downto 0) := X"0503";
--! "River" CPU Device workgroup.
constant RISCV_RIVER_WORKGROUP    : std_logic_vector(15 downto 0) := X"0505";
--! DMI debug register access to bus through the SBA interface.
constant RISCV_RIVER_DMI          : std_logic_vector(15 downto 0) := X"0506";
--! UART with DMA: Test Access Point (TAP)
constant GNSSSENSOR_UART_TAP      : std_logic_vector(15 downto 0) := X"050A";
--! JTAG Test Access Point (TAP)
constant GNSSSENSOR_JTAG_TAP      : std_logic_vector(15 downto 0) := X"050B";
--! @}

--! @name Slave Device IDs definition:
--! @{

--! Empty slave slot device
constant SLV_DID_EMPTY           : std_logic_vector(15 downto 0) := X"5577";
--! GNSS Engine Stub device
constant GNSS_SUB_SYSTEM         : std_logic_vector(15 downto 0) := X"0067";
--! GNSS Engine Stub device
constant GNSSSENSOR_ENGINE_STUB   : std_logic_vector(15 downto 0) := X"0068";
--! Fast Search Engines Device ID provided by gnsslib
constant GNSSSENSOR_FSE_V2_GPS    : std_logic_vector(15 downto 0) := X"0069";
--! Boot ROM Device ID
constant GNSSSENSOR_ROM           : std_logic_vector(15 downto 0) := X"0071";
--! Internal SRAM block Device ID
constant GNSSSENSOR_SRAM          : std_logic_vector(15 downto 0) := X"0073";
--! Configuration Registers Module Device ID provided by gnsslib
constant GNSSSENSOR_PNP           : std_logic_vector(15 downto 0) := X"0074";
--! SD-card controller Device ID provided by gnsslib
constant GNSSSENSOR_SPI_FLASH     : std_logic_vector(15 downto 0) := X"0075";
--! General purpose IOs Device ID provided by gnsslib
constant GNSSSENSOR_GPIO          : std_logic_vector(15 downto 0) := X"0076";
--! RF front-end controller Device ID provided by gnsslib
constant GNSSSENSOR_RF_CONTROL    : std_logic_vector(15 downto 0) := X"0077";
--! GNSS Engine Device ID provided by gnsslib
constant GNSSSENSOR_ENGINE        : std_logic_vector(15 downto 0) := X"0078";
--! rs-232 UART Device ID
constant GNSSSENSOR_UART          : std_logic_vector(15 downto 0) := X"007a";
--! Accelerometer Device ID provided by gnsslib
constant GNSSSENSOR_ACCELEROMETER : std_logic_vector(15 downto 0) := X"007b";
--! Gyroscope Device ID provided by gnsslib
constant GNSSSENSOR_GYROSCOPE     : std_logic_vector(15 downto 0) := X"007c";
--! Interrupt controller
constant GNSSSENSOR_IRQCTRL       : std_logic_vector(15 downto 0) := X"007d";
--! Ethernet MAC inherited from Gaisler greth module.
constant GNSSSENSOR_ETHMAC        : std_logic_vector(15 downto 0) := X"007f";
--! Debug Support Unit device id.
constant GNSSSENSOR_DSU           : std_logic_vector(15 downto 0) := X"0080";
--! GP Timers device id.
constant GNSSSENSOR_GPTIMERS      : std_logic_vector(15 downto 0) := X"0081";
--! ADC samples recorder
constant GNSSSENSOR_ADC_RECORDER  : std_logic_vector(15 downto 0) := X"0082";
-- OTP Memory 8KB bank
constant GNSSSENSOR_OTP_8KB       : std_logic_vector(15 downto 0) := X"0083";
--! @}

--! @name Decoder of the transaction size.
--! @{

--! Burst length size decoder
constant XSIZE_TOTAL : integer := 8;
--! Definition of the AXI bytes converter.
type xsize_type is array (0 to XSIZE_TOTAL-1) of integer;
--! Decoder of the transaction bytes from AXI format to Bytes.
constant XSizeToBytes : xsize_type := (
   0 => 1,
   1 => 2,
   2 => 4,
   3 => 8,
   4 => 16,
   5 => 32,
   6 => 64,
   7 => 128
);
--! @}

--! @name Plug'n'Play descriptor constants.
--! @{
--! Undefined type of the descriptor (empty device).
constant PNP_CFG_TYPE_INVALID   : std_logic_vector := "00";
--! AXI slave device standard descriptor.
constant PNP_CFG_TYPE_MASTER  : std_logic_vector := "01";
--! AXI master device standard descriptor.
constant PNP_CFG_TYPE_SLAVE : std_logic_vector := "10";
--! @brief Size in bytes of the standard slave descriptor..
--! @details Firmware uses this value instead of sizeof(nasti_slave_config_type).
constant PNP_CFG_SLAVE_DESCR_BYTES : std_logic_vector(7 downto 0) := X"10";
--! @brief Size in bytes of the standard master descriptor.
--! @details Firmware uses this value instead of sizeof(nasti_master_config_type).
constant PNP_CFG_MASTER_DESCR_BYTES : std_logic_vector(7 downto 0) := X"08";
--! @}


--! @brief   Plug-n-play descriptor structure for slave device.
--! @details Each slave device must generates this datatype output that
--!          is connected directly to the 'pnp' slave module on system bus.
type axi4_slave_config_type is record
    --! Descriptor size in bytes.
    descrsize : std_logic_vector(7 downto 0);
    --! Descriptor type.
    descrtype : std_logic_vector(1 downto 0);
    --! Descriptor size in bytes.
    irq_idx : std_logic_vector(7 downto 0);
    --! Base address value.
    xaddr  : std_logic_vector(CFG_SYSBUS_CFG_ADDR_BITS-1 downto 0);
    --! Maskable bits of the base address.
    xmask  : std_logic_vector(CFG_SYSBUS_CFG_ADDR_BITS-1 downto 0);
    --! Vendor ID.
    vid    : std_logic_vector(15 downto 0);
    --! Device ID.
    did    : std_logic_vector(15 downto 0);
end record;

--! @brief Default slave config value.
--! @default This value corresponds to an empty device and often used
--!          as assignment of outputs for the disabled device.
constant axi4_slave_config_none : axi4_slave_config_type := (
    PNP_CFG_SLAVE_DESCR_BYTES, PNP_CFG_TYPE_SLAVE, (others => '0'), 
    (others => '0'), (others => '0'), VENDOR_GNSSSENSOR, SLV_DID_EMPTY);


--! @brief   Plug-n-play descriptor structure for master device.
--! @details Each master device must generates this datatype output that
--!          is connected directly to the 'pnp' slave module on system bus.
type axi4_master_config_type is record
    --! Descriptor size in bytes.
    descrsize : std_logic_vector(7 downto 0);
    --! Descriptor type.
    descrtype : std_logic_vector(1 downto 0);
    --! Vendor ID.
    vid    : std_logic_vector(15 downto 0);
    --! Device ID.
    did    : std_logic_vector(15 downto 0);
end record;

--! @brief Default master config value.
constant axi4_master_config_none : axi4_master_config_type := (
    PNP_CFG_MASTER_DESCR_BYTES, PNP_CFG_TYPE_MASTER, 
    VENDOR_GNSSSENSOR, MST_DID_EMPTY);

constant ARCACHE_DEVICE_NON_BUFFERABLE : std_logic_vector(3 downto 0) := "0000";
constant ARCACHE_WRBACK_READ_ALLOCATE : std_logic_vector(3 downto 0) := "1111";

constant AWCACHE_DEVICE_NON_BUFFERABLE : std_logic_vector(3 downto 0) := "0000";
constant AWCACHE_WRBACK_WRITE_ALLOCATE : std_logic_vector(3 downto 0) := "1111";

-- see table C3-7 Permitted read address control signal combinations
--
--    read  |  cached  |  unique  |
--     0    |    0     |    *     |    ReadNoSnoop
--     0    |    1     |    0     |    ReadShared
--     0    |    1     |    1     |    ReadMakeUnique
constant ARSNOOP_READ_NO_SNOOP : std_logic_vector(3 downto 0) := "0000";
constant ARSNOOP_READ_SHARED : std_logic_vector(3 downto 0) := "0001";
constant ARSNOOP_READ_MAKE_UNIQUE : std_logic_vector(3 downto 0) := "1100";

-- see table C3-8 Permitted read address control signal combinations
--
--   write  |  cached  |  unique  |
--     1    |    0     |    *     |    WriteNoSnoop
--     1    |    1     |    1     |    WriteLineUnique
--     1    |    1     |    0     |    WriteBack
constant AWSNOOP_WRITE_NO_SNOOP : std_logic_vector(2 downto 0) := "000";
constant AWSNOOP_WRITE_LINE_UNIQUE : std_logic_vector(2 downto 0) := "001";
constant AWSNOOP_WRITE_BACK : std_logic_vector(2 downto 0) := "011";

-- see table C3-19
constant AC_SNOOP_READ_UNIQUE : std_logic_vector(3 downto 0) := "0111";
constant AC_SNOOP_MAKE_INVALID : std_logic_vector(3 downto 0) := "1101";


--! @brief AMBA AXI4 compliant data structure.
type axi4_metadata_type is record
  --! @brief Read address.
  --! @details The read address gives the address of the first transfer
  --!          in a read burst transaction.
  addr   : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
  --! @brief   Burst length.
  --! @details This signal indicates the exact number of transfers in 
  --!          a burst. This changes between AXI3 and AXI4. nastiXLenBits=8 so
  --!          this is an AXI4 implementation.
  --!              Burst_Length = len[7:0] + 1
  len    : std_logic_vector(7 downto 0);
  --! @brief   Burst size.
  --! @details This signal indicates the size of each transfer 
  --!          in the burst: 0=1 byte; ..., 6=64 bytes; 7=128 bytes;
  size   : std_logic_vector(2 downto 0);
  --! @brief   Read response.
  --! @details This signal indicates the status of the read transfer. 
  --! The responses are:
  --!      0b00 FIXED - In a fixed burst, the address is the same for every transfer 
  --!                  in the burst. Typically is used for FIFO.
  --!      0b01 INCR - Incrementing. In an incrementing burst, the address for each
  --!                  transfer in the burst is an increment of the address for the 
  --!                  previous transfer. The increment value depends on the size of 
  --!                  the transfer.
  --!      0b10 WRAP - A wrapping burst is similar to an incrementing burst, except 
  --!                  that the address wraps around to a lower address if an upper address 
  --!                  limit is reached.
  --!      0b11 resrved.
  burst  : std_logic_vector(1 downto 0);
  --! @brief   Lock type.
  --! @details Not supported in AXI4.
  lock   : std_logic;
  --! @brief   Memory type.
  --! @details See table for write and read transactions.
  cache  : std_logic_vector(3 downto 0);
  --! @brief   Protection type.
  --! @details This signal indicates the privilege and security level 
  --!          of the transaction, and whether the transaction is a data access
  --!          or an instruction access:
  --!  [0] :   0 = Unpriviledge access
  --!          1 = Priviledge access
  --!  [1] :   0 = Secure access
  --!          1 = Non-secure access
  --!  [2] :   0 = Data access
  --!          1 = Instruction access
  prot   : std_logic_vector(2 downto 0);
  --! @brief   Quality of Service, QoS. 
  --! @details QoS identifier sent for each read transaction. 
  --!          Implemented only in AXI4:
  --!              0b0000 - default value. Indicates that the interface is 
  --!                       not participating in any QoS scheme.
  qos    : std_logic_vector(3 downto 0);
  --! @brief Region identifier.
  --! @details Permits a single physical interface on a slave to be used for 
  --!          multiple logical interfaces. Implemented only in AXI4. This is 
  --!          similar to the banks implementation in Leon3 without address 
  --!          decoding.
  region : std_logic_vector(3 downto 0);
end record;

--! @brief Empty metadata value.
constant META_NONE : axi4_metadata_type := (
  (others =>'0'), X"00", "000", AXI_BURST_INCR, '0', X"0", "000", "0000", "0000"
);

--! @brief Master device output signals
type axi4_master_out_type is record
  --! Write Address channel:
  aw_valid : std_logic;
  --! metadata of the read channel.
  aw_bits : axi4_metadata_type;
  --! Write address ID. Identification tag used for a trasaction ordering.
  aw_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  --! Optional user defined signal in a write address channel.
  aw_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Write Data channel valid flag
  w_valid : std_logic;
  --! Write channel data value
  w_data : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  --! Write Data channel last address in a burst marker.
  w_last : std_logic;
  --! Write Data channel strob signals selecting certain bytes.
  w_strb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  --! Optional user defined signal in write channel.
  w_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Write Response channel accepted by master.
  b_ready : std_logic;
  --! Read Address Channel data valid.
  ar_valid : std_logic;
  --! Read Address channel metadata.
  ar_bits : axi4_metadata_type;
  --! Read address ID. Identification tag used for a trasaction ordering.
  ar_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  --! Optional user defined signal in read address channel.
  ar_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Read Data channel:
  r_ready : std_logic;
end record;

--! @brief   Master device empty value.
--! @warning If the master is not connected to the vector then vector value
--!          MUST BE initialized by this value.
constant axi4_master_out_none : axi4_master_out_type := (
      '0', META_NONE, (others=>'0'), (others => '0'),
      '0', (others=>'0'), '0', (others=>'0'), (others => '0'), 
      '0', '0', META_NONE, (others=>'0'), (others => '0'), '0');


--! @brief Master device input signals.
type axi4_master_in_type is record
  --! Write Address channel.
  aw_ready : std_logic;
  --! Write Data channel.
  w_ready : std_logic;
  --! Write Response channel:
  b_valid : std_logic;
  b_resp : std_logic_vector(1 downto 0);
  b_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  b_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Read Address Channel
  ar_ready : std_logic;
  --! Read valid.
  r_valid : std_logic;
  --! @brief Read response. 
  --! @details This signal indicates the status of the read transfer. 
  --!  The responses are:
  --!      0b00 OKAY - Normal access success. Indicates that a normal access has
  --!                  been successful. Can also indicate an exclusive access
  --!                  has failed.
  --!      0b01 EXOKAY - Exclusive access okay. Indicates that either the read or
  --!                  write portion of an exclusive access has been successful.
  --!      0b10 SLVERR - Slave error. Used when the access has reached the slave 
  --!                  successfully, but the slave wishes to return an error
  --!                  condition to the originating master.
  --!      0b11 DECERR - Decode error. Generated, typically by an interconnect 
  --!                  component, to indicate that there is no slave at the
  --!                  transaction address.
  r_resp : std_logic_vector(1 downto 0);
  --! Read data
  r_data : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  --! @brief  Read last. 
  --! @details This signal indicates the last transfer in a read burst.
  r_last : std_logic;
  --! @brief Read ID tag.
  --! @details This signal is the identification tag for the read data
  --!          group of signals generated by the slave.
  r_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  --! @brief User signal. 
  --! @details Optional User-defined signal in the read channel. Supported 
  --!          only in AXI4.
  r_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
end record;

constant axi4_master_in_none : axi4_master_in_type := (
      '0', '0', '0', AXI_RESP_OKAY, (others=>'0'), (others => '0'),
      '0', '0', AXI_RESP_OKAY, (others=>'0'), '0', (others=>'0'), (others => '0'));


--! @brief Slave device AMBA AXI input signals.
type axi4_slave_in_type is record
  --! Write Address channel:
  aw_valid : std_logic;
  aw_bits : axi4_metadata_type;
  aw_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  aw_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Write Data channel:
  w_valid : std_logic;
  w_data : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  w_last : std_logic;
  w_strb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  w_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Write Response channel:
  b_ready : std_logic;
  --! Read Address Channel:
  ar_valid : std_logic;
  ar_bits : axi4_metadata_type;
  ar_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  ar_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Read Data channel:
  r_ready : std_logic;
end record;

constant axi4_slave_in_none : axi4_slave_in_type := (
      '0', META_NONE, (others=>'0'), (others => '0'), '0',
      (others=>'0'), '0', (others=>'0'), (others => '0'), '0', '0', META_NONE,
      (others=>'0'), (others => '0'), '0');


--! @brief Slave device AMBA AXI output signals.
type axi4_slave_out_type is record
  --! Write Address channel:
  aw_ready : std_logic;
  --! Write Data channel:
  w_ready : std_logic;
  --! Write Response channel:
  b_valid : std_logic;
  b_resp : std_logic_vector(1 downto 0);
  b_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  b_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
  --! Read Address Channel
  ar_ready : std_logic;
  --! Read Data channel:
  r_valid : std_logic;
  --! @brief Read response.
  --! @details This signal indicates the status of the read transfer. 
  --!  The responses are:
  --!      0b00 OKAY - Normal access success. Indicates that a normal access has
  --!                  been successful. Can also indicate an exclusive access
  --!                  has failed.
  --!      0b01 EXOKAY - Exclusive access okay. Indicates that either the read or
  --!                  write portion of an exclusive access has been successful.
  --!      0b10 SLVERR - Slave error. Used when the access has reached the slave 
  --!                  successfully, but the slave wishes to return an error
  --!                  condition to the originating master.
  --!      0b11 DECERR - Decode error. Generated, typically by an interconnect 
  --!                  component, to indicate that there is no slave at the
  --!                  transaction address.
  r_resp : std_logic_vector(1 downto 0);
  --! Read data
  r_data : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  --! Read last. This signal indicates the last transfer in a read burst.
  r_last : std_logic;
  --! @brief Read ID tag. 
  --! @details This signal is the identification tag for the read data
  --!           group of signals generated by the slave.
  r_id   : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
  --! @brief User signal. 
  --! @details Optinal User-defined signal in the read channel. Supported 
  --!          only in AXI4.
  r_user : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
end record;

--! @brief Slave output signals connected to system bus.
--! @details If the slave is not connected to the vector then vector value
--! MUST BE initialized by this value.
constant axi4_slave_out_none : axi4_slave_out_type := (
      '0', '0', '0', AXI_RESP_EXOKAY, (others=>'0'), (others => '0'),
      '0', '0', AXI_RESP_EXOKAY, (others=>'1'), 
      '0', (others=>'0'), (others => '0'));


--! Array of addresses providing word aligned access.
type global_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);

  type dma_state_type is (
     DMA_STATE_IDLE,
     DMA_STATE_R_WAIT_RESP,
     DMA_STATE_R_WAIT_NEXT,
     DMA_STATE_W,
     DMA_STATE_W_WAIT_REQ,
     DMA_STATE_B
  );

--! @brief Master device to DMA engine request signals
  type dma_request_type is record
    valid : std_logic; -- response is valid
    ready : std_logic; -- ready to accept response
    write : std_logic;
    addr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    bytes : std_logic_vector(10 downto 0);
    size  : std_logic_vector(2 downto 0); -- 010=4 bytes; 011=8 bytes
    wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  end record;

--! @brief DMA engine to Master device response signals
  type dma_response_type is record
    ready : std_logic;  -- ready to accespt request
    valid : std_logic;  -- response is valid
    rdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  end record;

--! DMA engine registers bank
  type dma_bank_type is record
    state : dma_state_type;
    addr2 : std_logic;	          -- addr[2] bits to select low/high dword
    len   : integer range 0 to 255; -- burst (length-1)
    op32  : std_logic;
    wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  end record;

  constant DMA_BANK_RESET : dma_bank_type := (DMA_STATE_IDLE, '0', 0, '0', (others => '0'));

--! Device's DMA engine template procedure with AXI interface.
--! @param [in]  i_request Device to DMA engine request.
--! @param [out] o_response DMA Engine to Device response.
--! @param [in]  i_bank Bank of registers implemented by master device.
--! @param [out] o_bank Updated value for the master bank of registers.
--! @param [in]  i_msti AMBA to AXI master device signal.
--! @param [out] o_msto AXI master device signal to AMBA controller signals.
procedure procedureAxi4DMA(
      i_request : in dma_request_type;
      o_response : out dma_response_type;
      i_bank : in dma_bank_type;
      o_bank : out dma_bank_type;
      i_msti : in axi4_master_in_type;
      o_msto : out axi4_master_out_type
);

--! AXI4 slave interface.
--! @param [in]  i_xcfg  AXI Slave confguration descriptor defining memory base address.
--! @param [in]  i_xslvi AXI4 slave input interface.
--! @param [out] o_xslvo AXI4 slave output interface.
--! @param [in]  i_ready Memory device is ready to accept request.
--! @param [in]  i_rdata Read data value
--! @param [out] o_re Read enable
--! @param [out] o_rswap Read high word32 from 64-bits bus
--! @param [out] o_radr Memory interface read address array.
--! @param [out] o_wadr Memory interface write address array.
--! @param [in]  o_we Write enable
--! @param [out] o_wstrb Memory interface per byte write enable strobs.
--! @param [out] o_wdata Memory interface write data value.
component axi4_slave is
  generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_xcfg : in axi4_slave_config_type;
    i_xslvi : in axi4_slave_in_type;
    o_xslvo : out axi4_slave_out_type;
    i_ready : in std_logic;
    i_rdata : in std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    o_re : out std_logic;
    o_r32 : out std_logic;
    o_radr : out global_addr_array_type;
    o_wadr : out global_addr_array_type;
    o_we : out std_logic;
    o_wstrb : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    o_wdata : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
  );
end component; 

component axi4_defslv is
  generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_xslvi : in axi4_slave_in_type;
    o_xslvo : out axi4_slave_out_type
  );
end component; 


end; -- package declaration

--! Implementation of the declared sub-programs (functions and
--! procedures).
package body types_amba4 is

  --! Device's DMA engine template procedure with AXI interface.
  --! @param [in]  i_request Device to DMA engine request.
  --! @param [out] o_response DMA Engine to Device response.
  --! @param [in]  i_bank Bank of registers implemented by master device.
  --! @param [out] o_bank Updated value for the master bank of registers.
  --! @param [in]  i_msti AMBA to AXI master device signal.
  --! @param [out] o_msto AXI master device signal to AMBA controller signals.
  procedure procedureAxi4DMA(
      i_request : in dma_request_type;
      o_response : out dma_response_type;
      i_bank : in dma_bank_type;
      o_bank : out dma_bank_type;
      i_msti : in axi4_master_in_type;
      o_msto : out axi4_master_out_type
  ) is
    variable tmp_len : integer;
  begin
    o_bank := i_bank;
    o_msto := axi4_master_out_none;
    o_msto.ar_user       := (others => '0');
    o_msto.ar_id         := conv_std_logic_vector(0, CFG_SYSBUS_ID_BITS);
    o_msto.ar_bits.size  := (others => '0');
    o_msto.ar_bits.burst := AXI_BURST_INCR;
    o_msto.aw_user       := (others => '0');
    o_msto.aw_id         := conv_std_logic_vector(0, CFG_SYSBUS_ID_BITS);
    o_msto.aw_bits.size  := (others => '0');
    o_msto.aw_bits.burst := AXI_BURST_INCR;

    o_response.ready := '0';
    o_response.valid := '0';
    o_response.rdata := (others => '0');

    case i_bank.state is
    when DMA_STATE_IDLE =>
        o_msto.ar_valid := i_request.valid and not i_request.write;
        o_msto.aw_valid := i_request.valid and i_request.write;
        tmp_len := conv_integer(i_request.bytes(10 downto 2)) - 1;
        if i_request.valid = '1' and i_request.write = '1' then
            o_msto.aw_bits.addr  := i_request.addr(CFG_SYSBUS_ADDR_BITS-1 downto 3) & "000";
            o_bank.addr2         := i_request.addr(2);
            o_bank.len  := tmp_len;
            o_msto.aw_bits.size  := i_request.size; -- 4/8 bytes
            o_msto.aw_bits.len := conv_std_logic_vector(tmp_len, 8);
            o_bank.wdata := i_request.wdata;
            if i_msti.aw_ready = '1' then
                o_response.ready := '1';
                o_bank.state := DMA_STATE_W;
            end if;
        elsif i_request.valid = '1' and i_request.write = '0' then
            o_msto.ar_bits.addr  := i_request.addr;
            o_bank.addr2         := i_request.addr(2);
            o_bank.len  := tmp_len;
            o_msto.ar_bits.size  := i_request.size; -- 4/8 bytes
            o_msto.ar_bits.len := conv_std_logic_vector(tmp_len, 8);
            if i_msti.ar_ready = '1' then
                o_response.ready := '1';
                o_bank.state := DMA_STATE_R_WAIT_RESP;
            end if;
        end if;
        if i_request.size = "010" then
           o_bank.op32 := '1';
        else
           o_bank.op32 := '0';
        end if;
      
    when DMA_STATE_R_WAIT_RESP =>
        o_msto.r_ready := i_request.ready;
        o_response.valid := i_msti.r_valid;
        if (i_request.ready and i_msti.r_valid) = '1' then
            if i_bank.op32 = '1' and i_bank.addr2 = '1' then
                o_response.rdata := i_msti.r_data(63 downto 32) & i_msti.r_data(31 downto 0);
            else
                o_response.rdata := i_msti.r_data;
            end if;

            if i_msti.r_last = '1' then
                o_bank.state := DMA_STATE_IDLE;
            else
                if i_request.valid = '1' and i_request.write = '0' then
                    o_response.ready := '1';
                else
                    o_bank.state := DMA_STATE_R_WAIT_NEXT;
                end if;
            end if;
        end if;

    when DMA_STATE_R_WAIT_NEXT =>
        if i_request.valid = '1' and i_request.write = '0' then
            o_response.ready := '1';
            o_bank.state := DMA_STATE_R_WAIT_RESP;
        end if;

    when DMA_STATE_W =>
        o_msto.w_valid := '1';
        if i_bank.op32 = '1' then
            case i_bank.addr2 is
            when '0' => o_msto.w_strb := X"0f";
            when '1' => o_msto.w_strb := X"f0";
            when others =>
            end case;
        else
            o_msto.w_strb := X"ff";
        end if;
        o_msto.w_data := i_bank.wdata;
        
        if i_msti.w_ready = '1' then
            if i_bank.len = 0 then
                o_bank.state := DMA_STATE_B;
                o_msto.w_last := '1';
            elsif i_request.valid = '1' and i_request.write = '1' then
                o_bank.len := i_bank.len - 1;
                o_bank.wdata := i_request.wdata;
                o_response.ready := '1';
                -- Address will be incremented on slave side
                --v.waddr2 := not r.waddr2;
            else
                o_bank.state := DMA_STATE_W_WAIT_REQ;
            end if;
        end if;

    when DMA_STATE_W_WAIT_REQ =>
        if i_request.valid = '1' and i_request.write = '1' then
            o_bank.len := i_bank.len - 1;
            o_bank.wdata := i_request.wdata;
            o_response.ready := '1';
            o_bank.state := DMA_STATE_W;
        end if;

    when DMA_STATE_B =>
        o_msto.w_last := '0';
        o_msto.b_ready := '1';
        if i_msti.b_valid = '1' then
            o_bank.state := DMA_STATE_IDLE;
        end if;
    when others =>
    end case;
  end; -- procedure

end; -- package body
