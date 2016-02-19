-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Declaration and common methods implementation of the types_nasti
--!            package.
--! @details   This file defines bus interface constants that have to be
--!            used by any periphery device implementation in order 
--!            to provide compatibility in a wide range of possible settings.
--!            For better implementation use the AXI4 register bank and 
--!            implemented tasks from this file.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
--! Common constants and data conversion functions library
library commonlib;
use commonlib.types_common.all;

--! @brief   System bus AXI4/NASTI types definition.
--! @details This package provides general constants, data structures and
--!          and functions description that define behaviour of all
--!          peripheries devices implementing AXI4 interface.
package types_nasti is

--! @name    AXI4 slaves generic IDs.
--! @brief   Unique slave identificator.
--! @details Each module in a SoC has to be indexed by unique identificator.
--!          In current implementation it is used sequential indexing for it.
--!          Indexes are used to specify a device bus item in a vectors.
--!
--!          The first group of the memory devices are cachable in a terms of
--!          the Rocket-chip generator. But actually it doesn't take much sense
--!          because all devices are connected to the one system bus and
--!          implement identical responses. Cachable/Uncachable property
--!          of the transaction maybe used in L1toL2interconnect module of the
--!          Rocket-chip generator.
--! @{

--! Configuration index of the Boot ROM module visible by the firmware.
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
--! Configuration index of the Plug-n-Play module.
constant CFG_NASTI_SLAVE_PNP      : integer := CFG_NASTI_SLAVE_FSE_GPS+1;
--! Total number of the slaves devices.
constant CFG_NASTI_SLAVES_TOTAL  : integer := CFG_NASTI_SLAVE_PNP+1;  
--! @}

--! @name    AXI4 masters generic IDs.
--! @details Each master must be assigned to a specific ID that used
--!          as an index in the vector array of AXI master bus.
--! @{

--! Cached TileLinkIO bus.
constant CFG_NASTI_MASTER_CACHED   : integer := 0;
--! Uncached TileLinkIO bus.
constant CFG_NASTI_MASTER_UNCACHED : integer := CFG_NASTI_MASTER_CACHED+1;
--! Total Number of master devices on system bus.
constant CFG_NASTI_MASTER_TOTAL    : integer := CFG_NASTI_MASTER_UNCACHED+1;
--! @}


--! @name    AXI4 interrupt generic IDs.
--! @details Unique indentificator of the interrupt pin also used
--!          as an index in the interrupts bus.
--! @{

--! GNSS Engine IRQ pin that generates 1 msec pulses.
constant CFG_IRQ_GNSSENGINE      : integer := 0;
--! Total number of used interrupts in a system
constant CFG_IRQ_TOTAL           : integer := CFG_IRQ_GNSSENGINE+1;
--! @}

--! @name   SCALA generated parameters
--! @brief  This constant must correspond to Scala defined ones.
--! @{

constant CFG_ROCKET_ID_BITS      : integer := 5;
constant CFG_NASTI_DATA_BITS     : integer := 128;
constant CFG_NASTI_DATA_BYTES    : integer := CFG_NASTI_DATA_BITS / 8;
constant CFG_NASTI_ADDR_BITS     : integer := 32;
constant CFG_NASTI_ADDR_OFFSET   : integer := log2(CFG_NASTI_DATA_BYTES);
constant CFG_NASTI_CFG_ADDR_BITS : integer := CFG_NASTI_ADDR_BITS-12;
--! @}

--! @name   AXI Response values
--! @brief  AMBA 4.0 specified response types from a slave device.

--! @{
constant NASTI_RESP_OKAY     : std_logic_vector(1 downto 0) := "00";
constant NASTI_RESP_EXOKAY   : std_logic_vector(1 downto 0) := "01";
constant NASTI_RESP_SLVERR   : std_logic_vector(1 downto 0) := "10";
constant NASTI_RESP_DECERR   : std_logic_vector(1 downto 0) := "11";
--! @}

--! @name   AXI burst request type.
--! @brief  AMBA 4.0 specified burst operation request types.

--! @{
constant NASTI_BURST_FIXED   : std_logic_vector(1 downto 0) := "00";
constant NASTI_BURST_INCR    : std_logic_vector(1 downto 0) := "01";
constant NASTI_BURST_WRAP    : std_logic_vector(1 downto 0) := "10";
--! @}

--! Vendor ID of the GNSS Sensor Ltd.
constant VENDOR_GNSSSENSOR        : std_logic_vector(15 downto 0) := X"00F1"; 
--! Device IDs definition:
constant GNSSSENSOR_DUMMY         : std_logic_vector(15 downto 0) := X"5577";--! Dummy device
constant GNSSSENSOR_BOOTROM       : std_logic_vector(15 downto 0) := X"0071";--! Boot ROM Device ID
constant GNSSSENSOR_FWIMAGE       : std_logic_vector(15 downto 0) := X"0072";--! FW ROM image Device ID
constant GNSSSENSOR_SRAM          : std_logic_vector(15 downto 0) := X"0073";--! Internal SRAM block Device ID
constant GNSSSENSOR_PNP           : std_logic_vector(15 downto 0) := X"0074";--! Configuration Registers Module Device ID provided by gnsslib
constant GNSSSENSOR_SPI_FLASH     : std_logic_vector(15 downto 0) := X"0075";--! SD-card controller Device ID provided by gnsslib
constant GNSSSENSOR_GPIO          : std_logic_vector(15 downto 0) := X"0076";--! General purpose IOs Device ID provided by gnsslib
constant GNSSSENSOR_RF_CONTROL    : std_logic_vector(15 downto 0) := X"0077";--! RF front-end controller Device ID provided by gnsslib
constant GNSSSENSOR_ENGINE        : std_logic_vector(15 downto 0) := X"0078";--! GNSS Engine Device ID provided by gnsslib
constant GNSSSENSOR_FSE_V2        : std_logic_vector(15 downto 0) := X"0079";--! Fast Search Engines Device ID provided by gnsslib
constant GNSSSENSOR_UART          : std_logic_vector(15 downto 0) := X"007a";--! rs-232 UART Device ID
constant GNSSSENSOR_ACCELEROMETER : std_logic_vector(15 downto 0) := X"007b";--! Accelerometer Device ID provided by gnsslib
constant GNSSSENSOR_GYROSCOPE     : std_logic_vector(15 downto 0) := X"007c";--! Gyroscope Device ID provided by gnsslib
constant GNSSSENSOR_IRQCTRL       : std_logic_vector(15 downto 0) := X"007d";--! Interrupt controller


--! @brief Burst length size decoder
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

--! @brief   Plug-n-play descriptor structure
--! @details Each slave device must generates this datatype output that
--!          is connected directly to the 'pnp' slave module on system bus.
--! @todo    Add 'config structure size' field to this structure to make
--!          it more PCI capabalities style.
type nasti_slave_config_type is record
    xindex : integer;
    xaddr  : std_logic_vector(CFG_NASTI_CFG_ADDR_BITS-1 downto 0);
    xmask  : std_logic_vector(CFG_NASTI_CFG_ADDR_BITS-1 downto 0);
    vid    : std_logic_vector(15 downto 0); --! Vendor ID
    did    : std_logic_vector(15 downto 0); --! Device ID
end record;

--! @brief   Arrays of the plug-n-play descriptors.
--! @details Configuration bus vector from all slaves to plug'n'play
--!          NASTI device (pnp).
type nasti_slave_cfg_vector is array (0 to CFG_NASTI_SLAVES_TOTAL-1) 
       of nasti_slave_config_type;


--! Default config value
constant nasti_slave_config_none : nasti_slave_config_type := (
    0, (others => '0'), (others => '1'), VENDOR_GNSSSENSOR, GNSSSENSOR_DUMMY);

--! AMBA AXI4 compliant data structure.
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

--! Empty metadata value.
constant META_NONE : nasti_metadata_type := (
  (others =>'0'), X"00", "000", NASTI_BURST_INCR, '0', X"0", "000", "0000", "0000"
);

--! Master device output signals
type nasti_master_out_type is record
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

--! @brief   Master device empty value.
--! @warning If the master is not connected to the vector then vector value
--!          MUST BE initialized by this value.
constant nasti_master_out_none : nasti_master_out_type := (
      '0', META_NONE, (others=>'0'), '0',
      '0', (others=>'0'), '0', (others=>'0'), '0', 
      '0', '0', META_NONE, (others=>'0'), '0', '0');


--! Master device input signals.
type nasti_master_in_type is record
  --! How is owner of the AXI bus. It's controlled by 'axictrl' module.
  grant : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
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
  --! Read response. This signal indicates the status of the read transfer. 
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
  --! Read ID tag. This signal is the identification tag for the read data
  --! group of signals generated by the slave.
  r_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  --! User signal. Optinal User-defined signal in the read channel. Supported 
  --! only in AXI4.
  r_user : std_logic;--_vector(0 downto 0);
end record;

-- Demultiplexing slaves output to the single port:
type nasti_master_out_vector is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
       of nasti_master_out_type;


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

  --! Read valid.
  r_valid : std_logic;
  --! Read response. This signal indicates the status of the read transfer. 
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
  --! Read ID tag. This signal is the identification tag for the read data
  --! group of signals generated by the slave.
  r_id   : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  --! User signal. Optinal User-defined signal in the read channel. Supported 
  --! only in AXI4.
  r_user : std_logic;--_vector(0 downto 0);
end record;

--! If the slave is not connected to the vector then vector value
--! MUST BE initialized by this value.
constant nasti_slave_out_none : nasti_slave_out_type := (
      '0', '0', '0', NASTI_RESP_EXOKAY,
      (others=>'0'), '0', '0', '0', NASTI_RESP_EXOKAY, (others=>'1'), 
      '0', (others=>'0'), '0');

-- Demultiplexing slaves output to the single port:
type nasti_slaves_out_vector is array (0 to CFG_NASTI_SLAVES_TOTAL-1) 
       of nasti_slave_out_type;


type global_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES-1) 
       of std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);

type nasti_slave_rstatetype is (rwait, rtrans);
type nasti_slave_wstatetype is (wwait, wtrans, whandshake);


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
    rwaitready : std_logic;                 --! Reading wait state flag: 0=waiting
    
    wburst : std_logic_vector(1 downto 0);  -- 0=INCREMENT
    wsize  : integer;                       -- code in range 0=1 Bytes upto 7=128 Bytes. 
    waddr  : global_addr_array_type;        --! 4 KB bank
    wlen   : integer;                       --! AXI4 supports 256 burst operation
    wid    : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
    wresp  : std_logic_vector(1 downto 0);  --! OK=0
    wuser  : std_logic;
end record;

constant NASTI_SLAVE_BANK_RESET : nasti_slave_bank_type := (
    rwait, wwait,
    NASTI_BURST_FIXED, 0, (others=>(others=>'0')), 0, (others=>'0'), NASTI_RESP_OKAY, '0', '1',
    NASTI_BURST_FIXED, 0, (others=>(others=>'0')), 0, (others=>'0'), NASTI_RESP_OKAY, '0'
);


--! Read/write access state machines implementation.
procedure procedureAxi4(
     i      : in nasti_slave_in_type;
     cfg    : in nasti_slave_config_type;
     i_bank : in nasti_slave_bank_type;
     o_bank : out nasti_slave_bank_type
);

--! Read from the AXI4 bank latched address by index
--function functionAxi4GetRdAddr(
--     r   : nasti_slave_bank_type;
--     idx : integer) 
--return std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);

  --! Convert bank registers into output signals.
function functionAxi4Output(
     r : nasti_slave_bank_type;
     rd_val : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0))
return nasti_slave_out_type;

--! @brief   AXI bus controller. 
--! @details Simplified version with the hardcoded priorities to bus access.
--!          Lower master index has a higher priority.
component axictrl is
port (
    clk    : in std_logic;
    nrst   : in std_logic;
    slvoi  : in  nasti_slaves_out_vector;
    mstoi  : in  nasti_master_out_vector;
    slvio  : out nasti_slave_in_type;
    mstio  : out nasti_master_in_type
  );
end component; 

end; -- package declaration

--! Implementation of the declared sub-programs (functions and
--! procedures).
package body types_nasti is

  --! Read/write access state machines implementation.
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

    -- Reading state machine:
    case i_bank.rstate is
    when rwait =>
        if i.ar_valid = '1' 
          and ((i.ar_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and cfg.xmask) = cfg.xaddr) then
            o_bank.rstate := rtrans;
            traddr := (i.ar_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
                   & i.ar_bits.addr(11 downto 0);
            for n in 0 to CFG_NASTI_DATA_BYTES-1 loop
               o_bank.raddr(n) := traddr + n;
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
            o_bank.rlen := i_bank.rlen - 1;
            if i_bank.rburst = NASTI_BURST_INCR then
              for n in 0 to CFG_NASTI_DATA_BYTES-1 loop
                o_bank.raddr(n) := i_bank.raddr(n) + i_bank.rsize;
              end loop;
            end if;
            -- End of transaction:
            if i_bank.rlen = 0 then
                o_bank.rstate := rwait;
            end if;
        end if;
    end case;

    -- Writting state machine:
    case i_bank.wstate is
    when wwait =>
        if i.aw_valid = '1'
          and ((i.aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and cfg.xmask) = cfg.xaddr) then
            o_bank.wstate := wtrans;
            twaddr := (i.aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) and (not cfg.xmask))
                   & i.aw_bits.addr(11 downto 0);
            for n in 0 to CFG_NASTI_DATA_BYTES-1 loop
               o_bank.waddr(n) := twaddr + n;
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
            o_bank.wlen := i_bank.wlen - 1;
            if i_bank.wburst = NASTI_BURST_INCR then
              for n in 0 to CFG_NASTI_DATA_BYTES-1 loop
                o_bank.waddr(n) := i_bank.waddr(n) + i_bank.wsize;
              end loop;
            end if;
            -- End of transaction:
            if i_bank.wlen = 0 then
                o_bank.wstate := whandshake;
            end if;
        end if;
    when whandshake =>
        if i.b_ready = '1' then
            o_bank.wstate := wwait;
        end if;
    end case;
  end; -- procedure


--! Read from the AXI4 bank latched address by index
--! @param[in] idx Index of the address
--! @return Address with the masked older bits
--function functionAxi4GetRdAddr(
--     r   : nasti_slave_bank_type;
--     idx : integer)
--return std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0) is
--begin
--  return (r.raddr(idx));
--end;


--! Convert bank registers into output signals.
--! param[in] r Registers bank with the AXI4 state machines
--!             implementaitons.
--! param[in] rd_val Read value from the device's registers bank.
--!                  This value fully depends of device implementation.
--! @return NASTI output signals of the implemented slave device.
function functionAxi4Output(
     r : nasti_slave_bank_type;
     rd_val : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0))
return nasti_slave_out_type is
variable ret :  nasti_slave_out_type;
begin
    -- Read transfer:
    if r.rstate = rwait then
      ret.ar_ready    := '1';
    else
      ret.ar_ready    := '0';
    end if;

    ret.r_id      := r.rid;
    if r.rstate = rtrans and r.rlen = 0 then
      ret.r_last    := '1';
    else
      ret.r_last    := '0';
    end if;
    ret.r_resp    := r.rresp;    -- unaligned 32 bits access
    ret.r_user    := r.ruser;
    if r.rwaitready = '1' and r.rstate = rtrans then
      ret.r_valid   := '1';
    else
      ret.r_valid   := '0';
    end if;
    ret.r_data := rd_val;

    -- Write transfer:
    if r.wstate = wwait then
      ret.aw_ready    := '1';
    else
      ret.aw_ready    := '0';
    end if;
    if r.wstate = wtrans then
      ret.w_ready := '1';
    else
      ret.w_ready := '0';
    end if;

    -- Write Handshaking:
    ret.b_id := r.wid;
    ret.b_resp := r.wresp;
    ret.b_user := r.wuser;
    if r.wstate = whandshake then
      ret.b_valid := '1';
    else
      ret.b_valid := '0';
    end if;
    return (ret);
end;

end; -- package body
