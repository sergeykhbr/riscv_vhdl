--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

--! Standard library
library ieee;
--! Standard signal types import
use ieee.std_logic_1164.all;
--! Standard numerical types import
use ieee.numeric_std.all;
--! Common constants and data conversion functions library
library commonlib;
--! Import SoC specific types common for all devices
use commonlib.types_common.all;

--! @brief   System bus AMBA AXI(NASTI) types definition.
--! @details This package provides general constants, data structures and
--!          and functions description that define behaviour of all
--!          peripheries devices implementing AXI4 interface.
package types_amba4 is

--! @defgroup scala_cfg_group SCALA generated parameters
--! @ingroup axi4_config_generic_group
--! @details  This constant must correspond to Scala defined ones.
--! @{

--! User defined address ID bitwidth (aw_id / ar_id fields).
constant CFG_ROCKET_ID_BITS      : integer := 5;
--! Data bus bits width.
constant CFG_SYSBUS_DATA_BITS    : integer := 64;
constant CFG_NASTI_DATA_BITS     : integer := CFG_SYSBUS_DATA_BITS;      -- Legacy value to remove
--! Data bus bytes width
constant CFG_SYSBUS_DATA_BYTES   : integer := CFG_SYSBUS_DATA_BITS / 8;
constant CFG_NASTI_DATA_BYTES    : integer := CFG_SYSBUS_DATA_BYTES;     -- Legacy value to remove

--! Address bus bits width.
constant CFG_SYSBUS_ADDR_BITS    : integer := 32;
constant CFG_NASTI_ADDR_BITS     : integer := CFG_SYSBUS_ADDR_BITS;      -- Legacy value to remove
--! Definition of number of bits in address bus per one data transaction.
constant CFG_SYSBUS_ADDR_OFFSET  : integer := log2(CFG_NASTI_DATA_BYTES);
constant CFG_NASTI_ADDR_OFFSET   : integer := CFG_SYSBUS_ADDR_OFFSET;    -- Legacy value to remove
--! @brief Number of address bits used for device addressing. 
--! @details Default is 12 bits = 4 KB of address space minimum per each 
--!          mapped device.
constant CFG_SYSBUS_CFG_ADDR_BITS : integer := CFG_SYSBUS_ADDR_BITS-12;
constant CFG_NASTI_CFG_ADDR_BITS : integer := CFG_SYSBUS_CFG_ADDR_BITS;  -- Legacy value to remove
--! @brief Global alignment is set 32 bits.
constant CFG_ALIGN_BYTES         : integer := 4;
--! @brief  Number of parallel access to the atomic data.
constant CFG_WORDS_ON_BUS        : integer := CFG_SYSBUS_DATA_BYTES/CFG_ALIGN_BYTES;
--! @}

--! @defgroup slave_id_group AMBA AXI slaves generic IDs.
--! @ingroup axi4_config_generic_group
--! @details Each module in a SoC has to be indexed by unique identificator.
--!          In current implementation it is used sequential indexing for it.
--!          Indexes are used to specify a device bus item in a vectors.
--! @{

--! @brief Configuration index of the Boot ROM module visible by the firmware.
constant CFG_NASTI_SLAVE_BOOTROM  : integer := 0; 
--! Configuration index of the Firmware ROM Image module.
constant CFG_NASTI_SLAVE_ROMIMAGE  : integer := CFG_NASTI_SLAVE_BOOTROM+1;
--! Configuration index of the SRAM module visible by the firmware.
constant CFG_NASTI_SLAVE_SRAM     : integer := CFG_NASTI_SLAVE_ROMIMAGE+1;
--! Configuration index of the UART module.
constant CFG_NASTI_SLAVE_UART1    : integer := CFG_NASTI_SLAVE_SRAM+1;
--! Configuration index of the GPIO (General Purpose In/Out) module.
constant CFG_NASTI_SLAVE_GPIO     : integer := CFG_NASTI_SLAVE_UART1+1;
--! Configuration index of the Interrupt Controller module.
constant CFG_NASTI_SLAVE_IRQCTRL  : integer := CFG_NASTI_SLAVE_GPIO+1;
--! Configuration index of the Satellite Navigation Engine.
constant CFG_NASTI_SLAVE_ENGINE   : integer := CFG_NASTI_SLAVE_IRQCTRL+1;
--! Configuration index of the RF front-end controller.
constant CFG_NASTI_SLAVE_RFCTRL   : integer := CFG_NASTI_SLAVE_ENGINE+1;
--! Configuration index of the GPS-CA Fast Search Engine module.
constant CFG_NASTI_SLAVE_FSE_GPS  : integer := CFG_NASTI_SLAVE_RFCTRL+1;
--! Configuration index of the Ethernet MAC module.
constant CFG_NASTI_SLAVE_ETHMAC   : integer := CFG_NASTI_SLAVE_FSE_GPS+1;
--! Configuration index of the Debug Support Unit module.
constant CFG_NASTI_SLAVE_DSU      : integer := CFG_NASTI_SLAVE_ETHMAC+1;
--! Configuration index of the Debug Support Unit module.
constant CFG_NASTI_SLAVE_GPTIMERS : integer := CFG_NASTI_SLAVE_DSU+1;
--! Configuration index of the Plug-n-Play module.
constant CFG_NASTI_SLAVE_PNP      : integer := CFG_NASTI_SLAVE_GPTIMERS+1;
--! Total number of the slaves devices.
constant CFG_NASTI_SLAVES_TOTAL  : integer := CFG_NASTI_SLAVE_PNP+1;  
--! @}

--! @defgroup master_id_group AXI4 masters generic IDs.
--! @ingroup axi4_config_generic_group
--! @details Each master must be assigned to a specific ID that used
--!          as an index in the vector array of AXI master bus.
--! @{

--! Cached TileLinkIO bus.
constant CFG_NASTI_MASTER_CACHED   : integer := 0;
--! Uncached TileLinkIO bus.
constant CFG_NASTI_MASTER_UNCACHED : integer := CFG_NASTI_MASTER_CACHED+1;
--! Ethernet MAC master interface generic index.
constant CFG_NASTI_MASTER_ETHMAC   : integer := CFG_NASTI_MASTER_UNCACHED+1;
--! Tap via UART (debug port) generic index.
constant CFG_NASTI_MASTER_MSTUART    : integer := CFG_NASTI_MASTER_ETHMAC+1;
--! Tap via JTAG generic index.
constant CFG_AXI_MASTER_JTAG : integer := CFG_NASTI_MASTER_MSTUART+1;
--! Total Number of master devices on system bus.
constant CFG_NASTI_MASTER_TOTAL    : integer := CFG_AXI_MASTER_JTAG+1;
--! @}


--! @defgroup irq_id_group AXI4 interrupt generic IDs.
--! @ingroup axi4_config_generic_group
--! @details Unique indentificator of the interrupt pin also used
--!          as an index in the interrupts bus.
--! @{

--! Zero interrupt index must be unused.
constant CFG_IRQ_UNUSED         : integer := 0;
--! UART_A interrupt pin.
constant CFG_IRQ_UART1          : integer := 1;
--! Ethernet MAC interrupt pin.
constant CFG_IRQ_ETHMAC         : integer := 2;
--! GP Timers interrupt pin
constant CFG_IRQ_GPTIMERS       : integer := 3;
--! GNSS Engine IRQ pin that generates 1 msec pulses.
constant CFG_IRQ_GNSSENGINE     : integer := 4;
--! Total number of used interrupts in a system
constant CFG_IRQ_TOTAL          : integer := 5;
--! @}

--! @name   AXI Response values
--! @brief  AMBA 4.0 specified response types from a slave device.
--! @{

--! @brief Normal access success. 
--! @details Indicates that a normal access has been
--! successful. Can also indicate an exclusive access has failed. 
constant NASTI_RESP_OKAY     : std_logic_vector(1 downto 0) := "00";
--! @brief Exclusive access okay. 
--! @details Indicates that either the read or write
--! portion of an exclusive access has been successful.
constant NASTI_RESP_EXOKAY   : std_logic_vector(1 downto 0) := "01";
--! @brief Slave error. 
--! @details Used when the access has reached the slave successfully,
--! but the slave wishes to return an error condition to the originating
--! master.
constant NASTI_RESP_SLVERR   : std_logic_vector(1 downto 0) := "10";
--! @brief Decode error. 
--! @details Generated, typically by an interconnect component,
--! to indicate that there is no slave at the transaction address.
constant NASTI_RESP_DECERR   : std_logic_vector(1 downto 0) := "11";
--! @}

--! @name   AXI burst request type.
--! @brief  AMBA 4.0 specified burst operation request types.
--! @{

--! @brief Fixed address burst operation.
--! @details The address is the same for every transfer in the burst 
--!          (FIFO type)
constant NASTI_BURST_FIXED   : std_logic_vector(1 downto 0) := "00";
--! @brief Burst operation with address increment.
--! @details The address for each transfer in the burst is an increment of
--!        the address for the previous transfer. The increment value depends 
--!        on the size of the transfer.
constant NASTI_BURST_INCR    : std_logic_vector(1 downto 0) := "01";
--! @brief Burst operation with address increment and wrapping.
--! @details A wrapping burst is similar to an incrementing burst, except that
--!          the address wraps around to a lower address if an upper address 
--!          limit is reached
constant NASTI_BURST_WRAP    : std_logic_vector(1 downto 0) := "10";
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
--! "River" CPU Device ID.
constant RISCV_RIVER_CPU          : std_logic_vector(15 downto 0) := X"0505";
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
constant GNSSSENSOR_ENGINE_STUB   : std_logic_vector(15 downto 0) := X"0068";
--! Fast Search Engines Device ID provided by gnsslib
constant GNSSSENSOR_FSE_V2_GPS    : std_logic_vector(15 downto 0) := X"0069";
--! Boot ROM Device ID
constant GNSSSENSOR_BOOTROM       : std_logic_vector(15 downto 0) := X"0071";
--! FW ROM image Device ID
constant GNSSSENSOR_FWIMAGE       : std_logic_vector(15 downto 0) := X"0072";
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
type nasti_slave_config_type is record
    --! Descriptor size in bytes.
    descrsize : std_logic_vector(7 downto 0);
    --! Descriptor type.
    descrtype : std_logic_vector(1 downto 0);
    --! Descriptor size in bytes.
    irq_idx : std_logic_vector(7 downto 0);
    --! Base address value.
    xaddr  : std_logic_vector(CFG_NASTI_CFG_ADDR_BITS-1 downto 0);
    --! Maskable bits of the base address.
    xmask  : std_logic_vector(CFG_NASTI_CFG_ADDR_BITS-1 downto 0);
    --! Vendor ID.
    vid    : std_logic_vector(15 downto 0);
    --! Device ID.
    did    : std_logic_vector(15 downto 0);
end record;

--! @brief   Arrays of the plug-n-play descriptors.
--! @details Configuration bus vector from all slaves to plug'n'play
--!          NASTI device (pnp).
type nasti_slave_cfg_vector is array (0 to CFG_NASTI_SLAVES_TOTAL-1) 
       of nasti_slave_config_type;

--! @brief Default slave config value.
--! @default This value corresponds to an empty device and often used
--!          as assignment of outputs for the disabled device.
constant nasti_slave_config_none : nasti_slave_config_type := (
    PNP_CFG_SLAVE_DESCR_BYTES, PNP_CFG_TYPE_SLAVE, (others => '0'), 
    (others => '0'), (others => '0'), VENDOR_GNSSSENSOR, SLV_DID_EMPTY);


--! @brief   Plug-n-play descriptor structure for master device.
--! @details Each master device must generates this datatype output that
--!          is connected directly to the 'pnp' slave module on system bus.
type nasti_master_config_type is record
    --! Descriptor size in bytes.
    descrsize : std_logic_vector(7 downto 0);
    --! Descriptor type.
    descrtype : std_logic_vector(1 downto 0);
    --! Vendor ID.
    vid    : std_logic_vector(15 downto 0);
    --! Device ID.
    did    : std_logic_vector(15 downto 0);
end record;

--! @brief   Arrays of the plug-n-play descriptors.
--! @details Configuration bus vector from all slaves to plug'n'play
--!          NASTI device (pnp).
type nasti_master_cfg_vector is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
       of nasti_master_config_type;

--! @brief Default master config value.
constant nasti_master_config_none : nasti_master_config_type := (
    PNP_CFG_MASTER_DESCR_BYTES, PNP_CFG_TYPE_MASTER, 
    VENDOR_GNSSSENSOR, MST_DID_EMPTY);


--! @brief AMBA AXI4 compliant data structure.
type nasti_metadata_type is record
  --! @brief Read address.
  --! @details The read address gives the address of the first transfer
  --!          in a read burst transaction.
  addr   : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
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
constant META_NONE : nasti_metadata_type := (
  (others =>'0'), X"00", "000", NASTI_BURST_INCR, '0', X"0", "000", "0000", "0000"
);

--! @brief Master device output signals
type nasti_master_out_type is record
  --! Write Address channel:
  aw_valid : std_logic;
  --! metadata of the read channel.
  aw_bits : nasti_metadata_type;
  --! Write address ID. Identification tag used for a trasaction ordering.
  aw_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  --! Optional user defined signal in a write address channel.
  aw_user : std_logic;
  --! Write Data channel valid flag
  w_valid : std_logic;
  --! Write channel data value
  w_data : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  --! Write Data channel last address in a burst marker.
  w_last : std_logic;
  --! Write Data channel strob signals selecting certain bytes.
  w_strb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
  --! Optional user defined signal in write channel.
  w_user : std_logic;
  --! Write Response channel accepted by master.
  b_ready : std_logic;
  --! Read Address Channel data valid.
  ar_valid : std_logic;
  --! Read Address channel metadata.
  ar_bits : nasti_metadata_type;
  --! Read address ID. Identification tag used for a trasaction ordering.
  ar_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  --! Optional user defined signal in read address channel.
  ar_user : std_logic;
  --! Read Data channel:
  r_ready : std_logic;
end record;

--! @brief   Master device empty value.
--! @warning If the master is not connected to the vector then vector value
--!          MUST BE initialized by this value.
constant nasti_master_out_none : nasti_master_out_type := (
      '0', META_NONE, (others=>'0'), '0',
      '0', (others=>'0'), '0', (others=>'0'), '0', 
      '0', '0', META_NONE, (others=>'0'), '0', '0');

--! @brief Array of all masters outputs.
type nasti_master_out_vector is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
       of nasti_master_out_type;


--! @brief Master device input signals.
type nasti_master_in_type is record
  --! Write Address channel.
  aw_ready : std_logic;
  --! Write Data channel.
  w_ready : std_logic;
  --! Write Response channel:
  b_valid : std_logic;
  b_resp : std_logic_vector(1 downto 0);
  b_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  b_user : std_logic;
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
  r_data : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  --! @brief  Read last. 
  --! @details This signal indicates the last transfer in a read burst.
  r_last : std_logic;
  --! @brief Read ID tag.
  --! @details This signal is the identification tag for the read data
  --!          group of signals generated by the slave.
  r_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  --! @brief User signal. 
  --! @details Optional User-defined signal in the read channel. Supported 
  --!          only in AXI4.
  r_user : std_logic;
end record;

constant nasti_master_in_none : nasti_master_in_type := (
      '0', '0', '0', NASTI_RESP_OKAY, (others=>'0'), '0',
      '0', '0', NASTI_RESP_OKAY, (others=>'0'), '0', (others=>'0'), '0');

type nasti_master_in_vector is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
       of nasti_master_in_type;


--! @brief Slave device AMBA AXI input signals.
type nasti_slave_in_type is record
  --! Write Address channel:
  aw_valid : std_logic;
  aw_bits : nasti_metadata_type;
  aw_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  aw_user : std_logic;
  --! Write Data channel:
  w_valid : std_logic;
  w_data : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  w_last : std_logic;
  w_strb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
  w_user : std_logic;
  --! Write Response channel:
  b_ready : std_logic;
  --! Read Address Channel:
  ar_valid : std_logic;
  ar_bits : nasti_metadata_type;
  ar_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  ar_user : std_logic;
  --! Read Data channel:
  r_ready : std_logic;
end record;

constant nasti_slave_in_none : nasti_slave_in_type := (
      '0', META_NONE, (others=>'0'), '0', '0',
      (others=>'0'), '0', (others=>'0'), '0', '0', '0', META_NONE,
      (others=>'0'), '0', '0');

type nasti_slave_in_vector is array (0 to CFG_NASTI_SLAVES_TOTAL-1) 
       of nasti_slave_in_type;


--! @brief Slave device AMBA AXI output signals.
type nasti_slave_out_type is record
  --! Write Address channel:
  aw_ready : std_logic;
  --! Write Data channel:
  w_ready : std_logic;
  --! Write Response channel:
  b_valid : std_logic;
  b_resp : std_logic_vector(1 downto 0);
  b_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  b_user : std_logic;
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
  r_data : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  --! Read last. This signal indicates the last transfer in a read burst.
  r_last : std_logic;
  --! @brief Read ID tag. 
  --! @details This signal is the identification tag for the read data
  --!           group of signals generated by the slave.
  r_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  --! @brief User signal. 
  --! @details Optinal User-defined signal in the read channel. Supported 
  --!          only in AXI4.
  r_user : std_logic;--_vector(0 downto 0);
end record;

--! @brief Slave output signals connected to system bus.
--! @details If the slave is not connected to the vector then vector value
--! MUST BE initialized by this value.
constant nasti_slave_out_none : nasti_slave_out_type := (
      '0', '0', '0', NASTI_RESP_EXOKAY,
      (others=>'0'), '0', '0', '0', NASTI_RESP_EXOKAY, (others=>'1'), 
      '0', (others=>'0'), '0');

--! Array of all slaves outputs.
type nasti_slaves_out_vector is array (0 to CFG_NASTI_SLAVES_TOTAL-1) 
       of nasti_slave_out_type;

--! Array of addresses providing word aligned access.
type global_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);

--! Slave device states during reading value operation.
type nasti_slave_rstatetype is (rwait, rtrans);
--! Slave device states during writting data operation.
type nasti_slave_wstatetype is (wwait, wtrans);

--! @brief Template bank of registers for any slave device.
type nasti_slave_bank_type is record
    rstate : nasti_slave_rstatetype;
    wstate : nasti_slave_wstatetype;

    rburst : std_logic_vector(1 downto 0);
    rsize  : integer;
    raddr  : global_addr_array_type;
    rlen   : integer;                       --! AXI4 supports 256 burst operation
    rid    : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
    rresp  : std_logic_vector(1 downto 0);  --! OK=0
    ruser  : std_logic;
    rreorder : std_logic;
    rwaitready : std_logic;                 --! Reading wait state flag: 0=waiting. User's waitstates
    
    wburst : std_logic_vector(1 downto 0);  -- 0=INCREMENT
    wsize  : integer;                       -- code in range 0=1 Bytes upto 7=128 Bytes. 
    waddr  : global_addr_array_type;        --! 4 KB bank
    wlen   : integer;                       --! AXI4 supports 256 burst operation
    wid    : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
    wresp  : std_logic_vector(1 downto 0);  --! OK=0
    wuser  : std_logic;
    wreorder : std_logic;
    b_valid : std_logic;
end record;

--! Reset value of the template bank of registers of a slave device.
constant NASTI_SLAVE_BANK_RESET : nasti_slave_bank_type := (
    rwait, wwait,
    NASTI_BURST_FIXED, 0, (others=>(others=>'0')), 0, (others=>'0'), NASTI_RESP_OKAY, '0', '0', '1',
    NASTI_BURST_FIXED, 0, (others=>(others=>'0')), 0, (others=>'0'), NASTI_RESP_OKAY, '0', '0', '0'
);


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
    addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    bytes : std_logic_vector(10 downto 0);
    size  : std_logic_vector(2 downto 0); -- 010=4 bytes; 011=8 bytes
    wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  end record;

--! @brief DMA engine to Master device response signals
  type dma_response_type is record
    ready : std_logic;  -- ready to accespt request
    valid : std_logic;  -- response is valid
    rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  end record;

--! DMA engine registers bank
  type dma_bank_type is record
    state : dma_state_type;
    addr2 : std_logic;	          -- addr[2] bits to select low/high dword
    len   : integer range 0 to 255; -- burst (length-1)
    op32  : std_logic;
    wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
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
      i_msti : in nasti_master_in_type;
      o_msto : out nasti_master_out_type
);

--! Read/write access state machines implementation for the slave device.
--! @param [in] i Slave input signal passed from system bus.
--! @param [in] cfg Slave confguration descriptor defining memory base address.
--! @param [in] i_bank Bank of registers implemented by each slave device.
--! @param [out] o_bank Updated value for the slave bank of registers.
procedure procedureAxi4(
     i      : in nasti_slave_in_type;
     cfg    : in nasti_slave_config_type;
     i_bank : in nasti_slave_bank_type;
     o_bank : out nasti_slave_bank_type
);

--! AXI4 to Memory interface converter.
--! @param [in] i_ready Memory device is ready to accept request.
--! @param [in] i Slave input signal passed from system bus.
--! @param [in] cfg Slave confguration descriptor defining memory base address.
--! @param [in] i_bank Bank of registers implemented by each slave device.
--! @param [out] o_bank Updated value for the slave bank of registers.
--! @param [out] o_radr Memory interface read address array.
--! @param [out] o_wadr Memory interface write address array.
--! @param [out] o_wstrb Memory interface per byte write enable strobs.
--! @param [out] o_wdata Memory interface write data value.
procedure procedureAxi4toMem (
   i_ready : in std_logic;
   i      : in nasti_slave_in_type;
   cfg    : in nasti_slave_config_type;
   i_bank : in nasti_slave_bank_type;
   o_bank : out nasti_slave_bank_type;
   o_radr : out global_addr_array_type;
   o_wadr : out global_addr_array_type;
   o_wena : out std_logic;
   o_wstrb : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
   o_wdata : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
);

--! Memory interface to AXI4 converter.
--! @param [in] i_ready Slave device read data is ready.
--! @param [in] i_rdata Read data value
--! @param [in] i_bank Bank of registers implemented by each slave device.
--! @param [in] i_slvi AXI4 slave input interface.
--! @param [out] o_slvo AXI4 slave output interface.
procedure procedureMemToAxi4 (
   i_ready : in std_logic;
   i_rdata : in std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
   i_bank : in nasti_slave_bank_type;
   i_slvi : in nasti_slave_in_type;
   o_slvo : out nasti_slave_out_type
);

--! @brief Reordering elements of the address to provide 4-bytes memory access.
--! @param[in] mux Reordering control bits.
--! @param[in] iaddr Input addresses array.
--! @return Reordered addresses array.
function functionAddressReorder(
     mux   : std_logic_vector;
     iaddr : global_addr_array_type)
return global_addr_array_type;

procedure procedureWriteReorder(
     ena : in std_logic;
     mux   : in std_logic_vector(1 downto 0);
     iwaddr : in global_addr_array_type;
     iwstrb : in std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
     iwdata : in std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
     owaddr : out global_addr_array_type;
     owstrb : out std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
     owdata : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
);

--! @brief Complementary to address data reordring.
--! @details This function restore data bus as there wasn't address reordering.
--! @param[in] mux Reordering control bits.
--! @param[in] idata Input data array.
--! @return Restored data array.
function functionDataRestoreOrder(
     mux   : std_logic_vector;
     idata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0))
return std_logic_vector;

--! Convert slave bank registers into bus output signals.
--! @param[in] r      Bank of registers of the slave device.
--! @param[in] rd_val Formed by slave device read data value.
--! @return Slave device output signals connected to system bus.
function functionAxi4Output(
     r : nasti_slave_bank_type;
     rd_val : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0))
return nasti_slave_out_type;

--! @brief   AXI bus controller. 
--! @param [in] watchdog_memop 
--! @param [in] i_clk System bus clock.
--! @param [in] i_nrst Reset with active LOW level.
--! @param [in] i_slvcfg Slaves configuration vector.
--! @param [in] i_slvo Vector of slaves output signals.
--! @param [in] i_msto Vector of masters output signals.
--! @param [out] o_slvi Vector of slave inputs.
--! @param [out] o_msti Vector of master inputs.
--! @param [out] o_bus_util_w Write access bus utilization
--! @param [out] o_bus_util_r Read access bus utilization
--! @todo    Round-robin priority algorithm.
component axictrl is
  generic (
    async_reset : boolean
  );
  port (
    i_clk    : in std_logic;
    i_nrst   : in std_logic;
    i_slvcfg : in  nasti_slave_cfg_vector;
    i_slvo   : in  nasti_slaves_out_vector;
    i_msto   : in  nasti_master_out_vector;
    o_slvi   : out nasti_slave_in_vector;
    o_msti   : out nasti_master_in_vector;
    o_bus_util_w : out std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    o_bus_util_r : out std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
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
      i_msti : in nasti_master_in_type;
      o_msto : out nasti_master_out_type
  ) is
    variable tmp_len : integer;
  begin
    o_bank := i_bank;
    o_msto := nasti_master_out_none;
    o_msto.ar_user       := '0';
    o_msto.ar_id         := conv_std_logic_vector(0, CFG_ROCKET_ID_BITS);
    o_msto.ar_bits.size  := (others => '0');
    o_msto.ar_bits.burst := NASTI_BURST_INCR;
    o_msto.aw_user       := '0';
    o_msto.aw_id         := conv_std_logic_vector(0, CFG_ROCKET_ID_BITS);
    o_msto.aw_bits.size  := (others => '0');
    o_msto.aw_bits.burst := NASTI_BURST_INCR;

    o_response.ready := '0';
    o_response.valid := '0';
    o_response.rdata := (others => '0');

    case i_bank.state is
    when DMA_STATE_IDLE =>
        o_msto.ar_valid := i_request.valid and not i_request.write;
        o_msto.aw_valid := i_request.valid and i_request.write;
        tmp_len := conv_integer(i_request.bytes(10 downto 2)) - 1;
        if i_request.valid = '1' and i_request.write = '1' then
            o_msto.aw_bits.addr  := i_request.addr(CFG_NASTI_ADDR_BITS-1 downto 3) & "000";
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

  --! Read/write access state machines implementation.
  --! @param [in] i Slave input signal passed from system bus.
  --! @param [in] cfg Slave confguration descriptor defining memory base address.
  --! @param [in] i_bank Bank of registers implemented by each slave device.
  --! @param [out] o_bank Updated value for the slave bank of registers.
  procedure procedureAxi4(
     i      : in nasti_slave_in_type;
     cfg    : in nasti_slave_config_type;
     i_bank : in nasti_slave_bank_type;
     o_bank : out nasti_slave_bank_type
  ) is
  variable traddr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
  variable twaddr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
  begin
    o_bank := i_bank;

    traddr := (i.ar_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
             & i.ar_bits.addr(11 downto 0);

    -- Reading state machine:
    case i_bank.rstate is
    when rwait =>
        if i.ar_valid = '1' and i.aw_valid = '0' and i_bank.wstate = wwait then
            o_bank.rstate := rtrans;

            for n in 0 to CFG_WORDS_ON_BUS-1 loop
              o_bank.raddr(n) := traddr + n*CFG_ALIGN_BYTES;
            end loop;
            o_bank.rsize := XSizeToBytes(conv_integer(i.ar_bits.size));
            o_bank.rburst := i.ar_bits.burst;
            o_bank.rlen := conv_integer(i.ar_bits.len);
            o_bank.rid := i.ar_id;
            o_bank.rresp := NASTI_RESP_OKAY;
            o_bank.ruser := i.ar_user;
            
            --! No Wait States by default for reading operation.
            --!
            --! User can re-assign this value directly in module to implement
            --! reading wait states.
            --! Example: see axi2fse.vhd bridge implementation
            o_bank.rwaitready := '1';
        end if;
    when rtrans =>
        if i.r_ready = '1' and i_bank.rwaitready = '1' then
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
                o_bank.raddr(n) := i_bank.raddr(n) + i_bank.rsize;
            end loop;
            if i_bank.rburst = NASTI_BURST_WRAP then
                for n in 0 to CFG_WORDS_ON_BUS-1 loop
                    -- i_bank.rsize = 8 and i_bank.rlen = 3
                    -- 8 x 4 = 32 bytes 
                    o_bank.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                         := i_bank.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                end loop;
            end if;
            -- End of transaction (or process another one):
            if i_bank.rlen = 0 then
                if i.ar_valid = '1' and i.aw_valid = '0' then
                    for n in 0 to CFG_WORDS_ON_BUS-1 loop
                      o_bank.raddr(n) := traddr + n*CFG_ALIGN_BYTES;
                    end loop;
                    o_bank.rsize := XSizeToBytes(conv_integer(i.ar_bits.size));
                    o_bank.rburst := i.ar_bits.burst;
                    o_bank.rlen := conv_integer(i.ar_bits.len);
                    o_bank.rid := i.ar_id;
                    o_bank.rresp := NASTI_RESP_OKAY;
                    o_bank.ruser := i.ar_user;
                    o_bank.rwaitready := '1';
                else
                    o_bank.rstate := rwait;
                end if;
            else
                o_bank.rlen := i_bank.rlen - 1;
            end if;
        end if;
    end case;

    -- Writing state machine:
    case i_bank.wstate is
    when wwait =>
        if i.aw_valid = '1' and i_bank.rlen = 0 then
            o_bank.wstate := wtrans;
            twaddr := (i.aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
                   & i.aw_bits.addr(11 downto 0);
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
               o_bank.waddr(n) := twaddr + n*CFG_ALIGN_BYTES;
            end loop;
            o_bank.wsize := XSizeToBytes(conv_integer(i.aw_bits.size));
            o_bank.wburst := i.aw_bits.burst;
            o_bank.wlen := conv_integer(i.aw_bits.len);
            o_bank.wid := i.aw_id;
            o_bank.wresp := NASTI_RESP_OKAY;
            o_bank.wuser := i.aw_user;
        end if;
    when wtrans =>
        if i.w_valid = '1' then
            if i_bank.wburst = NASTI_BURST_INCR then
              for n in 0 to CFG_WORDS_ON_BUS-1 loop
                o_bank.waddr(n) := i_bank.waddr(n) + i_bank.wsize;
              end loop;
            end if;
            -- End of transaction:
            if i_bank.wlen = 0 then
                o_bank.b_valid := '1';
                if i.aw_valid = '0' then
                    o_bank.wstate := wwait;
                else
                    twaddr := (i.aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
                           & i.aw_bits.addr(11 downto 0);
                    for n in 0 to CFG_WORDS_ON_BUS-1 loop
                       o_bank.waddr(n) := twaddr + n*CFG_ALIGN_BYTES;
                    end loop;
                    o_bank.wsize := XSizeToBytes(conv_integer(i.aw_bits.size));
                    o_bank.wburst := i.aw_bits.burst;
                    o_bank.wlen := conv_integer(i.aw_bits.len);
                    o_bank.wid := i.aw_id;
                    o_bank.wresp := NASTI_RESP_OKAY;
                    o_bank.wuser := i.aw_user;
                end if;
            else
                o_bank.wlen := i_bank.wlen - 1;
            end if;
        end if;
    end case;

    if i.b_ready = '1' and i_bank.b_valid = '1' then
        if i_bank.wstate = wtrans and i.w_valid = '1' and i_bank.wlen = 0 then
            o_bank.b_valid := '1';
        else
            o_bank.b_valid := '0';
        end if;
    end if;
  end; -- procedure



  --! AXI4 to Memory interface converter.
  --! @param [in] i_ready Memory device is ready to accept request.
  --! @param [in] i Slave input signal passed from system bus.
  --! @param [in] cfg Slave confguration descriptor defining memory base address.
  --! @param [in] i_bank Bank of registers implemented by each slave device.
  --! @param [out] o_bank Updated value for the slave bank of registers.
  --! @param [out] o_radr Memory interface read address array.
  --! @param [out] o_wena Memory interface write enable.
  --! @param [out] o_wadr Memory interface write address array.
  --! @param [out] o_wstrb Memory interface per byte write enable strobs.
  --! @param [out] o_wdata Memory interface write data value.
  procedure procedureAxi4toMem(
     i_ready : in std_logic;
     i      : in nasti_slave_in_type;
     cfg    : in nasti_slave_config_type;
     i_bank : in nasti_slave_bank_type;
     o_bank : out nasti_slave_bank_type;
     o_radr : out global_addr_array_type;
     o_wadr : out global_addr_array_type;
     o_wena : out std_logic;
     o_wstrb : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
     o_wdata : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
  ) is
  variable traddr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
  variable twaddr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
  variable v_radr_mux : global_addr_array_type;
  variable v_wadr_mux : global_addr_array_type;
  variable v_radr : global_addr_array_type;
  variable v_wadr : global_addr_array_type;
  variable v_wena : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  variable v_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  variable v_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  variable v_wreorder : std_logic;
  begin
    o_bank := i_bank;

    traddr := (i.ar_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
             & i.ar_bits.addr(11 downto 0);

    twaddr := (i.aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
             & i.aw_bits.addr(11 downto 0);

    for n in 0 to CFG_WORDS_ON_BUS-1 loop
        v_radr_mux(n) := traddr + n*CFG_ALIGN_BYTES;
        v_wadr_mux(n) := twaddr + n*CFG_ALIGN_BYTES;
    end loop;

    v_radr := i_bank.raddr;
    v_wadr := i_bank.waddr;
    v_wena := (others => '0');
    v_wreorder := i_bank.wreorder;

    -- Reading state machine:
    case i_bank.rstate is
    when rwait =>
        if i.ar_valid = '1' and i_ready = '1'
           and i.aw_valid = '0' and i_bank.wstate = wwait then
            o_bank.rstate := rtrans;

            if i.ar_bits.addr(2) = '0' then
                v_radr := v_radr_mux;
            else
                v_radr(0) := v_radr_mux(1);
                v_radr(1) := v_radr_mux(0);
            end if;
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
                o_bank.raddr(n) := v_radr(n) + XSizeToBytes(conv_integer(i.ar_bits.size));
                if i.ar_bits.burst = NASTI_BURST_WRAP then
                    -- i_bank.rsize = 8 and i_bank.rlen = 3
                    -- 8 x 4 = 32 bytes 
                    o_bank.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                         := v_radr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                end if;
            end loop;
            o_bank.rreorder := i.ar_bits.addr(2);
            o_bank.rsize := XSizeToBytes(conv_integer(i.ar_bits.size));
            o_bank.rburst := i.ar_bits.burst;
            o_bank.rlen := conv_integer(i.ar_bits.len);
            o_bank.rid := i.ar_id;
            o_bank.rresp := NASTI_RESP_OKAY;
            o_bank.ruser := i.ar_user;
        end if;
    when rtrans =>
        if i.r_ready = '1' and i_ready = '1' then
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
                o_bank.raddr(n) := i_bank.raddr(n) + i_bank.rsize;
            end loop;
            if i_bank.rburst = NASTI_BURST_WRAP then
                for n in 0 to CFG_WORDS_ON_BUS-1 loop
                    -- i_bank.rsize = 8 and i_bank.rlen = 3
                    -- 8 x 4 = 32 bytes 
                    o_bank.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                         := i_bank.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                end loop;
            end if;
            -- End of transaction (or process another one):
            if i_bank.rlen = 0 then
                if i.ar_valid = '1' and i.aw_valid = '0' then
                    if i.ar_bits.addr(2) = '0' then
                        v_radr := v_radr_mux;
                    else
                        v_radr(0) := v_radr_mux(1);
                        v_radr(1) := v_radr_mux(0);
                    end if;
                    for n in 0 to CFG_WORDS_ON_BUS-1 loop
                        o_bank.raddr(n) := v_radr(n) + XSizeToBytes(conv_integer(i.ar_bits.size));
                        if i.ar_bits.burst = NASTI_BURST_WRAP then
                            o_bank.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                                := v_radr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                        end if;
                    end loop;
                    o_bank.rreorder := i.ar_bits.addr(2);
                    o_bank.rsize := XSizeToBytes(conv_integer(i.ar_bits.size));
                    o_bank.rburst := i.ar_bits.burst;
                    o_bank.rlen := conv_integer(i.ar_bits.len);
                    o_bank.rid := i.ar_id;
                    o_bank.rresp := NASTI_RESP_OKAY;
                    o_bank.ruser := i.ar_user;
                    o_bank.rwaitready := '1';
                else
                    o_bank.rstate := rwait;
                end if;
            else
                o_bank.rlen := i_bank.rlen - 1;
            end if;
        end if;
    end case;

    -- Writing state machine:
    case i_bank.wstate is
    when wwait =>
        if i.aw_valid = '1' and i_ready = '1' and i_bank.rlen = 0 then
            if i.w_valid = '0' then
                -- Full AXI bus protocol
                o_bank.wstate := wtrans;
                o_bank.wreorder := i.aw_bits.addr(2);
            else
                -- AXI lite (no burst support)
                v_wena := (others => '1');
                v_wreorder := i.aw_bits.addr(2);
            end if;

            if i.aw_bits.addr(2) = '0' then
                v_wadr := v_wadr_mux;
            else
                v_wadr(0) := v_wadr_mux(1);
                v_wadr(1) := v_wadr_mux(0);
            end if;
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
               o_bank.waddr(n) := v_wadr(n);
            end loop;
            o_bank.wsize := XSizeToBytes(conv_integer(i.aw_bits.size));
            o_bank.wburst := i.aw_bits.burst;
            o_bank.wlen := conv_integer(i.aw_bits.len);
            o_bank.wid := i.aw_id;
            o_bank.wresp := NASTI_RESP_OKAY;
            o_bank.wuser := i.aw_user;
        end if;
    when wtrans =>
        v_wena := (others => '1');
        if i.w_valid = '1' and i_ready = '1' then
            if i_bank.wburst = NASTI_BURST_INCR then
              for n in 0 to CFG_WORDS_ON_BUS-1 loop
                o_bank.waddr(n) := i_bank.waddr(n) + i_bank.wsize;
              end loop;
            end if;
            -- End of transaction:
            if i_bank.wlen = 0 then
                o_bank.b_valid := '1';
                if i.aw_valid = '0' then
                    o_bank.wstate := wwait;
                else
                    -- Only Full AXI Bus protocol support here
                    if i.aw_bits.addr(2) = '0' then
                        o_bank.waddr := v_wadr_mux;
                    else
                        o_bank.waddr(0) := v_wadr_mux(1);
                        o_bank.waddr(1) := v_wadr_mux(0);
                    end if;
                    o_bank.wreorder := i.aw_bits.addr(2);
                    o_bank.wsize := XSizeToBytes(conv_integer(i.aw_bits.size));
                    o_bank.wburst := i.aw_bits.burst;
                    o_bank.wlen := conv_integer(i.aw_bits.len);
                    o_bank.wid := i.aw_id;
                    o_bank.wresp := NASTI_RESP_OKAY;
                    o_bank.wuser := i.aw_user;
                end if;
            else
                o_bank.wlen := i_bank.wlen - 1;
            end if;
        end if;
    end case;

    if i.b_ready = '1' and i_bank.b_valid = '1' then
        if i_bank.wstate = wtrans and i.w_valid = '1' and i_bank.wlen = 0 then
            o_bank.b_valid := '1';
        else
            o_bank.b_valid := '0';
        end if;
    end if;

    if v_wreorder = '0' then
        v_wdata := i.w_data;
        v_wstrb := i.w_strb and v_wena;
    else
        v_wdata(31 downto 0) := i.w_data(63 downto 32);
        v_wdata(63 downto 32) := i.w_data(31 downto 0);
        v_wstrb := (i.w_strb(3 downto 0) & i.w_strb(7 downto 4))
               and (v_wena(3 downto 0) & v_wena(7 downto 4));
    end if;

    o_radr := v_radr;
    o_wadr := v_wadr;
    o_wena := v_wena(0);
    o_wdata := v_wdata;
    o_wstrb := v_wstrb;
  end; -- procedure


  --! Memory interface to AXI4 converter.
  --! @param [in] i_ready Slave device read data is ready.
  --! @param [in] i_rdata Read data value
  --! @param [in] i_bank Bank of registers implemented by each slave device.
  --! @param [in] i_slvi AXI4 slave input interface.
  --! @param [out] o_slvo AXI4 slave output interface.
  procedure procedureMemToAxi4(
     i_ready : in std_logic;
     i_rdata : in std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
     i_bank : in nasti_slave_bank_type;
     i_slvi : in nasti_slave_in_type;
     o_slvo : out nasti_slave_out_type
  ) is
  begin
    -- Read transfer:
    o_slvo.aw_ready := i_ready;
    o_slvo.w_ready := '1';
    o_slvo.ar_ready := i_ready;

    if i_slvi.aw_valid = '1' then
        o_slvo.ar_ready := '0';
    end if;

    o_slvo.r_id := i_bank.rid;
    o_slvo.r_last := '0';
    if i_bank.rstate = rtrans then
        if i_bank.rlen = 0 then
            o_slvo.r_last := '1';
        else
            o_slvo.aw_ready := '0';
            o_slvo.w_ready := '0';
            o_slvo.ar_ready := '0';
        end if;
    end if;
    o_slvo.r_resp := i_bank.rresp;
    o_slvo.r_user := i_bank.ruser;
    if i_ready = '1' and i_bank.rstate = rtrans then
        o_slvo.r_valid   := '1';
    else
        o_slvo.r_valid   := '0';
    end if;

    if i_bank.rreorder = '0' then
        o_slvo.r_data := i_rdata;
    else
        o_slvo.r_data := i_rdata(31 downto 0) & i_rdata(63 downto 32);
    end if;

    -- Write transfer:
    if i_bank.wstate = wtrans then
        o_slvo.ar_ready := '0';
        if i_bank.wlen /= 0 then
            o_slvo.aw_ready    := '0';
        end if;
    end if;

    -- Write Handshaking:
    o_slvo.b_id := i_bank.wid;
    o_slvo.b_resp := i_bank.wresp;
    o_slvo.b_user := i_bank.wuser;
    o_slvo.b_valid := i_bank.b_valid;
  end; -- procedure


--! @brief Reordering elements of the address to provide 4-bytes memory access.
--! @param[in] mux Reordering control bits.
--! @param[in] iaddr Input addresses array.
--! @return Reordered addresses array.
function functionAddressReorder(
     mux   : std_logic_vector;
     iaddr : global_addr_array_type)
return global_addr_array_type is
variable oaddr :  global_addr_array_type;
begin
  if CFG_NASTI_DATA_BITS = 128 then
    if mux = "00" then
       oaddr := iaddr;
    elsif mux = "01" then
       oaddr(0) := iaddr(3);
       oaddr(1) := iaddr(0);
       oaddr(2) := iaddr(1);
       oaddr(3) := iaddr(2);
    elsif mux = "10" then
       oaddr(0) := iaddr(2);
       oaddr(1) := iaddr(3);
       oaddr(2) := iaddr(0);
       oaddr(3) := iaddr(1);
    else
       oaddr(0) := iaddr(1);
       oaddr(1) := iaddr(2);
       oaddr(2) := iaddr(3);
       oaddr(3) := iaddr(0);
    end if;
  elsif CFG_NASTI_DATA_BITS = 64 then
    if mux(2) = '0' then
       oaddr := iaddr;
    else
       oaddr(0) := iaddr(1);
       oaddr(1) := iaddr(0);
    end if;
  end if;
  return oaddr;
end;

--! @brief Reordering elements of the write transaction to provide 4-bytes memory access.
procedure procedureWriteReorder(
     ena : in std_logic;
     mux   : in std_logic_vector(1 downto 0);
     iwaddr : in global_addr_array_type;
     iwstrb : in std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
     iwdata : in std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
     owaddr : out global_addr_array_type;
     owstrb : out std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
     owdata : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)) is
begin
  if CFG_NASTI_DATA_BITS = 128 and ena = '1' then
    if mux = "00" then
       owaddr := iwaddr;
       owdata := iwdata;
       owstrb := iwstrb;
    elsif mux = "01" then
       owaddr(0) := iwaddr(3);
       owaddr(1) := iwaddr(0);
       owaddr(2) := iwaddr(1);
       owaddr(3) := iwaddr(2);
       
       owdata(31 downto 0) := iwdata(127 downto 96);
       owdata(127 downto 32) := iwdata(95 downto 0);
       
       owstrb := iwstrb(11 downto 0) & iwstrb(15 downto 12);
    elsif mux = "10" then
       owaddr(0) := iwaddr(2);
       owaddr(1) := iwaddr(3);
       owaddr(2) := iwaddr(0);
       owaddr(3) := iwaddr(1);

       owdata(63 downto 0) := iwdata(127 downto 64);
       owdata(127 downto 64) := iwdata(63 downto 0);
       
       owstrb := iwstrb(7 downto 0) & iwstrb(15 downto 8);
    else
       owaddr(0) := iwaddr(1);
       owaddr(1) := iwaddr(2);
       owaddr(2) := iwaddr(3);
       owaddr(3) := iwaddr(0);

       owdata(95 downto 0) := iwdata(127 downto 32);
       owdata(127 downto 96) := iwdata(31 downto 0);
       
       owstrb := iwstrb(3 downto 0) & iwstrb(15 downto 4);
    end if;
  elsif CFG_NASTI_DATA_BITS = 64 and ena = '1' then
    if mux(0) = '0' then
       owaddr := iwaddr;
       owdata := iwdata;
       owstrb := iwstrb;
    else
       owaddr(0) := iwaddr(1);
       owaddr(1) := iwaddr(0);
       owdata(31 downto 0) := iwdata(63 downto 32);
       owdata(63 downto 32) := iwdata(31 downto 0);
       owstrb := iwstrb(3 downto 0) & iwstrb(7 downto 4);
    end if;
  else
    owaddr := (others => (others => '0'));
    owdata := (others => '0');
    owstrb := (others => '0');
  end if;
end;

--! @brief Complementary to address data reordring.
--! @details This function restore data bus as there wasn't address reordering.
--! @param[in] mux Reordering control bits.
--! @param[in] idata Input data array.
--! @return Restored data array.
function functionDataRestoreOrder(
     mux   : std_logic_vector;
     idata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0))
return std_logic_vector is
variable odata :  std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
begin
  if CFG_NASTI_DATA_BITS = 128 then
    if mux = "00" then
       odata := idata;
    elsif mux = "01" then
       odata(95 downto 0) := idata(127 downto 32);
       odata(127 downto 96) := idata(31 downto 0);
    elsif mux = "10" then
       odata(63 downto 0) := idata(127 downto 64);
       odata(127 downto 64) := idata(63 downto 0);
    else
       odata(31 downto 0) := idata(127 downto 96);
       odata(127 downto 32) := idata(95 downto 0);
    end if;
  elsif CFG_NASTI_DATA_BITS = 64 then
    if mux(2) = '0' then
       odata := idata;
    else 
       odata(31 downto 0) := idata(63 downto 32);
       odata(63 downto 32) := idata(31 downto 0);
    end if;
  end if;
  return odata;
end;

--! @brief Convert bank registers into output signals
--! @param[in] r Registers bank with the AXI4 state machines
--!             implementaitons.
--! @param[in] rd_val Read value from the device's registers bank.
--!                  This value fully depends of device implementation.
--! @return NASTI output signals of the implemented slave device.
function functionAxi4Output(
     r : nasti_slave_bank_type;
     rd_val : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0))
return nasti_slave_out_type is
variable ret :  nasti_slave_out_type;
begin
    -- Read transfer:
    ret.aw_ready    := '1';
    ret.ar_ready    := '1';

    ret.r_id      := r.rid;
    ret.r_last    := '0';
    if r.rstate = rtrans then
      if r.rlen = 0 then
          ret.r_last    := '1';
      else
          ret.aw_ready    := '0';
          ret.ar_ready    := '0';
      end if;
    end if;
    ret.r_resp    := r.rresp;
    ret.r_user    := r.ruser;
    if r.rwaitready = '1' and r.rstate = rtrans then
      ret.r_valid   := '1';
    else
      ret.r_valid   := '0';
    end if;

    ret.r_data := rd_val;

    -- Write transfer:
    if r.wstate = wtrans then
      ret.ar_ready := '0';  -- can't request read address while writing, but write addr possible on last cycle
      ret.w_ready := '1';
      if r.wlen /= 0 then
        ret.aw_ready    := '0';
      end if;
    else
      ret.w_ready := '0';
    end if;

    -- Write Handshaking:
    ret.b_id := r.wid;
    ret.b_resp := r.wresp;
    ret.b_user := r.wuser;
    ret.b_valid := r.b_valid;
    return (ret);
end;

end; -- package body
