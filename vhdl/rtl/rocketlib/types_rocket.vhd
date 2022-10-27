-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     System Top level modules and interconnect declarations.
-----------------------------------------------------------------------------

--! Standard library.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
--! Technology definition library.
library techmap;
use techmap.gencomp.all;
--! CPU, System Bus and common peripheries library.
library ambalib;
use ambalib.types_amba4.all;

--! @brief   Declaration of components visible on SoC top level.
package types_rocket is

--! @name Scala inherited constants.
--! @brief The following constants were define in Rocket-chip generator.
--! @{

--! @brief   Bits allocated for the memory tag value.
--! @details This value is defined \i Config.scala and depends of others
--!          configuration paramters, like number of master, clients, channels
--!          and so on. It is not used in VHDL implemenation.
constant MEM_TAG_BITS  : integer := 6;
--! @brief   SCALA generated value. Not used in VHDL.
constant MEM_ADDR_BITS : integer := 26;
--! @}

  --! @name   Rocket Chip interrupt pins 
  --!
  --! Interrupts types:
  --!    1. Local (inside tile) Software interrupts
  --!    2. Local (inside tile) interrupts from timer
  --!    3. External (global) interrupts from PLIC (Platorm-Level	Interrupt	Controller).
  --! @}
  
  constant CFG_CORE_IRQ_DEBUG : integer := 0;
  --! Local Timer's interrupt (machine mode)
  constant CFG_CORE_IRQ_MTIP  : integer := CFG_CORE_IRQ_DEBUG + 1;
  --! Local sofware interrupt (machine mode)
  constant CFG_CORE_IRQ_MSIP  : integer := CFG_CORE_IRQ_MTIP + 1;
  --! External PLIC's interrupt (machine mode)
  constant CFG_CORE_IRQ_MEIP  : integer := CFG_CORE_IRQ_MSIP + 1;
  --! External PLIC's interrupt (superuser mode)
  constant CFG_CORE_IRQ_SEIP  : integer := CFG_CORE_IRQ_MEIP + 1;
  -- Total number of implemented interrupts
  constant CFG_CORE_IRQ_TOTAL : integer := CFG_CORE_IRQ_SEIP + 1;
  --! @}


  --! @name    Memory Transaction types.
  --! @details TileLinkIO interface uses these constant to identify the payload
  --!          size of the transaction.
  --! @{
  constant MT_B  : integer := 0;  --! int8_t   Memory Transaction.
  constant MT_H  : integer := 1;  --! int16_t  Memory Transaction.
  constant MT_W  : integer := 2;  --! int32_t  Memory Transaction.
  constant MT_D  : integer := 3;  --! int64_t  Memory Transaction.
  constant MT_BU : integer := 4;  --! uint8_t  Memory Transaction.
  constant MT_HU : integer := 5;  --! uint16_t Memory Transaction.
  constant MT_WU : integer := 6;  --! uint32_t Memory Transaction.
  constant MT_Q  : integer := 7;  --! AXI data-width Memory Transaction (default 128-bits).
  --! @}

  --! @brief Memory operation types
  --! @details The union bits [5:1] contains information about current transaction
  constant M_XRD     : std_logic_vector(4 downto 0) := "00000"; --! int load
  constant M_XWR     : std_logic_vector(4 downto 0) := "00001"; --! int store
  constant M_PFR     : std_logic_vector(4 downto 0) := "00010"; --! prefetch with intent to read
  constant M_PFW     : std_logic_vector(4 downto 0) := "00011"; --! prefetch with intent to write
  constant M_XA_SWAP : std_logic_vector(4 downto 0) := "00100";
  constant M_NOP     : std_logic_vector(4 downto 0) := "00101";
  constant M_XLR     : std_logic_vector(4 downto 0) := "00110";
  constant M_XSC     : std_logic_vector(4 downto 0) := "00111";
  constant M_XA_ADD  : std_logic_vector(4 downto 0) := "01000";
  constant M_XA_XOR  : std_logic_vector(4 downto 0) := "01001";
  constant M_XA_OR   : std_logic_vector(4 downto 0) := "01010";
  constant M_XA_AND  : std_logic_vector(4 downto 0) := "01011";
  constant M_XA_MIN  : std_logic_vector(4 downto 0) := "01100";
  constant M_XA_MAX  : std_logic_vector(4 downto 0) := "01101";
  constant M_XA_MINU : std_logic_vector(4 downto 0) := "01110";
  constant M_XA_MAXU : std_logic_vector(4 downto 0) := "01111";
  constant M_FLUSH   : std_logic_vector(4 downto 0) := "10000"; --! write back dirty data and cede R/W permissions
  constant M_PRODUCE : std_logic_vector(4 downto 0) := "10001"; --! write back dirty data and cede W permissions
  constant M_CLEAN   : std_logic_vector(4 downto 0) := "10011"; --! write back dirty data and retain R/W permissions

  function isAMO(cmd : std_logic_vector(4 downto 0)) return std_logic;
  --def isPrefetch(cmd: UInt) = cmd === M_PFR || cmd === M_PFW
  --def isRead(cmd: UInt) = cmd === M_XRD || cmd === M_XLR || cmd === M_XSC || isAMO(cmd)
  function isWrite(cmd : std_logic_vector(4 downto 0)) return std_logic;
  --def isWriteIntent(cmd: UInt) = isWrite(cmd) || cmd === M_PFW || cmd === M_XLR

  --! <Definitions.scala> Object Acquire {}
  constant ACQUIRE_GET_SINGLE_DATA_BEAT : std_logic_vector(2 downto 0) := "000"; -- Get a single beat of data
  constant ACQUIRE_GET_BLOCK_DATA       : std_logic_vector(2 downto 0) := "001"; -- Get a whole block of data
  constant ACQUIRE_PUT_SINGLE_DATA_BEAT : std_logic_vector(2 downto 0) := "010"; -- Put a single beat of data.
  constant ACQUIRE_PUT_BLOCK_DATA       : std_logic_vector(2 downto 0) := "011"; -- Put  a whole block of data.
  constant ACQUIRE_PUT_ATOMIC_DATA      : std_logic_vector(2 downto 0) := "100"; -- Performe an atomic memory op
  constant ACQUIRE_GET_PREFETCH_BLOCK   : std_logic_vector(2 downto 0) := "101"; -- Prefetch a whole block of data
  constant ACQUIRE_PUT_PREFETCH_BLOCK   : std_logic_vector(2 downto 0) := "110"; -- Prefetch a whole block of data, with intent to write
  
  --! <tilelink.scala> Object Grant {}
  constant GRANT_ACK_RELEASE          : std_logic_vector(3 downto 0) := "0000"; -- For acking Releases
  constant GRANT_ACK_PREFETCH         : std_logic_vector(3 downto 0) := "0001"; -- For acking any kind of Prefetch
  constant GRANT_ACK_NON_PREFETCH_PUT : std_logic_vector(3 downto 0) := "0011"; -- For acking any kind of non-prfetch Put
  constant GRANT_SINGLE_BEAT_GET      : std_logic_vector(3 downto 0) := "0100"; -- Supplying a single beat of Get
  constant GRANT_BLOCK_GET            : std_logic_vector(3 downto 0) := "0101"; -- Supplying all beats of a GetBlock

  --! MESI coherence
  constant CACHED_ACQUIRE_SHARED      : std_logic_vector(2 downto 0) := "000"; -- get 
  constant CACHED_ACQUIRE_EXCLUSIVE   : std_logic_vector(2 downto 0) := "001"; -- put

  constant CACHED_GRANT_SHARED        : std_logic_vector(3 downto 0) := "0000";
  constant CACHED_GRANT_EXCLUSIVE     : std_logic_vector(3 downto 0) := "0001";  
  constant CACHED_GRANT_EXCLUSIVE_ACK : std_logic_vector(3 downto 0) := "0010";  

  --! @brief Memory Operation size decoder
  --! @details TileLink bus has encoded Memory Operation size
  --!          in the union[n+1:n] bits of the acquire request.
  --! @warning Sign bit isn't transmitted in union since 20160930.
  constant MEMOP_XSIZE_TOTAL : integer := 8;
  type memop_xsize_type is array (0 to MEMOP_XSIZE_TOTAL-1) of std_logic_vector(2 downto 0);
  constant opSizeToXSize : memop_xsize_type := (
    MT_B => "000",
    MT_H => "001",
    MT_W => "010",
    MT_D => "011",
    MT_BU => "100",
    MT_HU => "101",
    MT_WU => "110",
    MT_Q => conv_std_logic_vector(log2(CFG_SYSBUS_DATA_BYTES),3)
  );


type tile_in_type is record
    a_ready : std_logic;
    b_valid : std_logic;
    b_opcode : std_logic_vector(2 downto 0);
    b_param : std_logic_vector(1 downto 0);
    b_size : std_logic_vector(3 downto 0);
    b_source : std_logic_vector(2 downto 0);
    b_address : std_logic_vector(31 downto 0);
    b_mask : std_logic_vector(7 downto 0);
    b_data : std_logic_vector(63 downto 0);
    c_ready : std_logic;
    d_valid : std_logic;
    d_opcode : std_logic_vector(2 downto 0);
    d_param : std_logic_vector(1 downto 0);
    d_size : std_logic_vector(3 downto 0);
    d_source : std_logic_vector(2 downto 0);
    d_sink : std_logic_vector(3 downto 0);
    d_addr_lo : std_logic_vector(2 downto 0);
    d_data : std_logic_vector(63 downto 0);
    d_error : std_logic;
    e_ready : std_logic;
end record;

type tile_out_type is record
    a_valid : std_logic;
    a_opcode : std_logic_vector(2 downto 0);
    a_param : std_logic_vector(2 downto 0);
    a_size : std_logic_vector(3 downto 0);
    a_source : std_logic_vector(2 downto 0);
    a_address : std_logic_vector(31 downto 0);
    a_mask : std_logic_vector(7 downto 0);
    a_data : std_logic_vector(63 downto 0);
    b_ready : std_logic;
    c_valid : std_logic;
    c_opcode : std_logic_vector(2 downto 0);
    c_param : std_logic_vector(2 downto 0);
    c_size : std_logic_vector(3 downto 0);
    c_source : std_logic_vector(2 downto 0);
    c_address : std_logic_vector(31 downto 0);
    c_data : std_logic_vector(63 downto 0);
    c_error : std_logic;
    d_ready : std_logic;
    e_valid : std_logic;
    e_sink : std_logic_vector(3 downto 0);
end record;


  --! @brief Decode Acquire request from the Cached/Uncached TileLink
  --! @param[in] a_type   Request type depends of the built_in flag
  --! @param[in] built_in This flag defines cached or uncached request. For
  --!                     the uncached this value is set to 1.
  --! @param[in] u        Union bits. This value is decoding depending of
  --!                     types operation (rd/wr) and cached/uncached.
  procedure procedureDecodeTileAcquire (
    a_type    : in std_logic_vector(2 downto 0);
    built_in  : in std_logic;
    u         : in std_logic_vector(10 downto 0);--was 16
    write     : out std_logic;
    wmask     : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    axi_sz    : out std_logic_vector(2 downto 0);
    byte_addr : out std_logic_vector(2 downto 0);
    beat_cnt  : out integer
  );


--! @brief   RocketTile component declaration.
--! @details This module implements Risc-V Core with L1-cache, 
--!          branch predictor and other stuffs of the RocketTile.
--! @param[in] xindex1 Cached Tile AXI master index
--! @param[in] xindex2 Uncached Tile AXI master index
--! @param[in] hartid  Tile ID. At least 0 must be implemented.
--! @param[in] reset_vector  Reset instruction pointer value.
--! @param[in] rst     Reset signal with active HIGH level.
--! @param[in] soft_rst Software Reset via DSU
--! @param[in] clk_sys System clock (BUS/CPU clock).
--! @param[in] slvo    Bus-to-Slave device signals.
--! @param[in] msti    Bus-to-Master device signals.
--! @param[out] msto1  CachedTile-to-Bus request signals.
--! @param[out] msto2  UncachedTile-to-Bus request signals.
--! @param[in] interrupts  Interrupts line supported by Rocket chip.
component rocket_l1only is 
generic (
    hartid : integer := 0;
    reset_vector : integer := 16#1000#
);
port ( 
    nrst     : in std_logic;
    clk_sys  : in std_logic;
    msti1    : in axi4_master_in_type;
    msto1    : out axi4_master_out_type;
    mstcfg1  : out axi4_master_config_type;
    msti2    : in axi4_master_in_type;
    msto2    : out axi4_master_out_type;
    mstcfg2  : out axi4_master_config_type;
    interrupts : in std_logic_vector(CFG_CORE_IRQ_TOTAL-1 downto 0)
);
end component;

end; -- package declaration

--! -----------------
package body types_rocket is
  
  function isAMO(cmd : std_logic_vector(4 downto 0))
    return std_logic is
    variable t1 : std_logic;
  begin
    t1 := '0';
    if cmd = M_XA_SWAP then
      t1 := '1';
    end if;
    return (cmd(3) or t1);
  end;

  function isWrite(cmd : std_logic_vector(4 downto 0))  
    return std_logic is
    variable ret : std_logic;
  begin
    ret := isAMO(cmd);
    if cmd = M_XWR then ret := '1'; end if;
    if cmd = M_XSC then ret := '1'; end if;
    return (ret);
  end;

  --! @brief Decode Acquire request from the Cached/Uncached TileLink
  --! @param[in] a_type   Request type depends of the built_in flag
  --! @param[in] built_in This flag defines cached or uncached request. For
  --!                     the uncached this value is set to 1.
  --! @param[in] u        Union bits. This value is decoding depending of
  --!                     types operation (rd/wr) and cached/uncached.
  procedure procedureDecodeTileAcquire(
    a_type    : in std_logic_vector(2 downto 0);
    built_in  : in std_logic;
    u         : in std_logic_vector(10 downto 0);--was 16
    write     : out std_logic;
    wmask     : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    axi_sz    : out std_logic_vector(2 downto 0);
    byte_addr : out std_logic_vector(2 downto 0);
    beat_cnt  : out integer
  ) is
  begin

    if built_in = '1' then
      -- Cached request
      case a_type is
      when ACQUIRE_GET_SINGLE_DATA_BEAT =>
          write := '0';
          wmask := (others => '0');
          --! union used as: 
          --!   addr[2:0] & op_sz[1:0] & mem_op_code[M_SZ-1:0] & alloc[0]
          --!   [10:8][7:6][5:1][0]
          byte_addr := u(10 downto 8);--tst.block.byte_addr;
          axi_sz := opSizeToXSize(conv_integer(u(7 downto 6)));
          beat_cnt := 0;
      when ACQUIRE_GET_PREFETCH_BLOCK |
           ACQUIRE_PUT_PREFETCH_BLOCK |
           ACQUIRE_GET_BLOCK_DATA =>
          -- cache line size / data bits width
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_SYSBUS_ADDR_OFFSET,3);
          beat_cnt := 7;--3;--tlDataBeats-1; 
      when ACQUIRE_PUT_SINGLE_DATA_BEAT =>
          -- Single beat data.
          write := '1';
          --! union used as: 
          --!   wmask[log2(64)-1:0] & alloc[0]
          wmask := u(CFG_SYSBUS_DATA_BYTES downto 1);
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_SYSBUS_ADDR_OFFSET,3);
          beat_cnt := 0;
      when ACQUIRE_PUT_BLOCK_DATA =>
          -- Multibeat data.
          write := '1';
          wmask := (others => '1');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_SYSBUS_ADDR_OFFSET,3);
          beat_cnt := 7;--3;--tlDataBeats-1; 
      when ACQUIRE_PUT_ATOMIC_DATA =>
          -- Single beat data. 64 bits width
          write := '1';
          --if CFG_NASTI_DATA_BITS = 128 then
          --    if u(12) = '0' then
          --        wmask(7 downto 0) := (others => '1');
          --        wmask(15 downto 8) := (others => '0');
          --    else 
          --        wmask(7 downto 0) := (others => '0');
          --        wmask(15 downto 8) := (others => '1');
          --    end if;
          --else
              wmask := (others => '1');
          --end if;
          byte_addr := (others => '0');
          axi_sz := opSizeToXSize(conv_integer(u(7 downto 6)));
          beat_cnt := 0; 
      when others =>
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := (others => '0');
          beat_cnt := 0;
      end case;
    else --! built_in = '0'
      --! Cached request
      case a_type is
      when CACHED_ACQUIRE_SHARED =>
          --! Uncore/coherence/Metadata.scala
          --!      union = op_code[4:0] & '1';
          write := '0';
          wmask := (others => '0');
          byte_addr := u(10 downto 8);--tst.block.byte_addr;
          axi_sz := opSizeToXSize(conv_integer(u(7 downto 6)));
          beat_cnt := 0;
      when CACHED_ACQUIRE_EXCLUSIVE =>
          -- Single beat data.
          write := '1';
          --! Uncore/coherence/Metadata.scala
          --!      union = op_code[4:0] & '1';
          --! unclear how to manage it.
          --wmask := u(CFG_NASTI_DATA_BYTES downto 1);
          wmask := (others => '1');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_SYSBUS_ADDR_OFFSET,3);
          beat_cnt := 0;
      when others =>
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := (others => '0');
          beat_cnt := 0;
      end case;
    end if;
  end procedure;

end; -- package body
