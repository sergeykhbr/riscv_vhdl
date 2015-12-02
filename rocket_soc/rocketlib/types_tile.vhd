-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     TileLink bus interface declaration.
-----------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
--! Technology definition library
library techmap;
use techmap.gencomp.all;
--! Common constants and data conversion functions library
library commonlib;
use commonlib.types_common.all;
--! CPU, System Bus and common peripheries library.
library rocketlib;
use rocketlib.types_nasti.all;

--! @brief   TileLinkIO specific types definition.
--! @details TileLinkIO interface could be implemented in cached and
--!          uncached variantes. These bus are fully defined in SCALA
--!          System generator and are very poor documented.
--!          Cached link implements all signals of the uncached plus
--!          additional support for the "probe" and "release" transactions
--!          that are used to signal state of the cache memory access.
--!          Cached link could be used (and used) for uncached access.
package types_tile is
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

  --! <tilelink.scala> Object Acquire {}
  constant ACQUIRE_GET_SINGLE_DATA_BEAT : std_logic_vector(2 downto 0) := "000";
  constant ACQUIRE_GET_BLOCK_DATA       : std_logic_vector(2 downto 0) := "001"; -- 
  constant ACQUIRE_PUT_SINGLE_DATA_BEAT : std_logic_vector(2 downto 0) := "010"; -- Single beat data.
  constant ACQUIRE_PUT_BLOCK_DATA       : std_logic_vector(2 downto 0) := "011"; -- For acMultibeat data.
  constant ACQUIRE_PUT_ATOMIC_DATA      : std_logic_vector(2 downto 0) := "100"; -- Single beat data. 64 bits width
  constant ACQUIRE_PREFETCH_BLOCK       : std_logic_vector(2 downto 0) := "101";
  
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
  --!          in the union[8:6] bits of the acquire request.
  constant MEMOP_XSIZE_TOTAL : integer := 8;
  type memop_xsize_type is array (0 to MEMOP_XSIZE_TOTAL-1) of std_logic_vector(2 downto 0);
  constant opSizeToXSize : memop_xsize_type := (
    MT_B => "000",
    MT_BU => "000",
    MT_H => "001",
    MT_HU => "001",
    MT_W => "010",
    MT_WU => "010", --! unimplemented in scala
    MT_D => "011",
    MT_Q => conv_std_logic_vector(log2(CFG_NASTI_DATA_BYTES),3)
  );


type tile_cached_in_type is record
    acquire_ready : std_logic;
    grant_valid : std_logic;
    grant_bits_addr_beat : std_logic_vector(1 downto 0);
    --! client's transaction id
    grant_bits_client_xact_id : std_logic_vector(1 downto 0);
    grant_bits_manager_xact_id : std_logic_vector(3 downto 0);
    grant_bits_is_builtin_type : std_logic;
    grant_bits_g_type : std_logic_vector(3 downto 0);
    grant_bits_data : std_logic_vector(127 downto 0);
    probe_valid : std_logic;
    probe_bits_addr_block : std_logic_vector(25 downto 0);
    probe_bits_p_type : std_logic_vector(1 downto 0);
    release_ready : std_logic;
end record;

type tile_cached_out_type is record
    acquire_valid : std_logic;
    acquire_bits_addr_block : std_logic_vector(25 downto 0);
    acquire_bits_client_xact_id : std_logic_vector(1 downto 0);
    acquire_bits_addr_beat : std_logic_vector(1 downto 0);
    acquire_bits_is_builtin_type : std_logic;
    acquire_bits_a_type : std_logic_vector(2 downto 0);
    acquire_bits_union : std_logic_vector(16 downto 0);
    acquire_bits_data : std_logic_vector(127 downto 0);
    grant_ready : std_logic;
    probe_ready : std_logic;
    release_valid : std_logic;
    release_bits_addr_beat : std_logic_vector(1 downto 0);
    release_bits_addr_block : std_logic_vector(25 downto 0);
    release_bits_client_xact_id : std_logic_vector(1 downto 0);
    release_bits_r_type : std_logic_vector(2 downto 0);
    release_bits_voluntary : std_logic;
    release_bits_data : std_logic_vector(127 downto 0);
end record;

type tile_uncached_in_type is record
    acquire_ready : std_logic;
    grant_valid : std_logic;
    grant_bits_addr_beat : std_logic_vector(1 downto 0);
    grant_bits_client_xact_id : std_logic_vector(1 downto 0);
    grant_bits_manager_xact_id : std_logic_vector(3 downto 0);
    grant_bits_is_builtin_type : std_logic;
    grant_bits_g_type : std_logic_vector(3 downto 0);
    grant_bits_data : std_logic_vector(127 downto 0);
end record;

type tile_uncached_out_type is record
    acquire_valid : std_logic;
    acquire_bits_addr_block : std_logic_vector(25 downto 0);
    acquire_bits_client_xact_id : std_logic_vector(1 downto 0);
    acquire_bits_addr_beat : std_logic_vector(1 downto 0);
    acquire_bits_is_builtin_type : std_logic;
    acquire_bits_a_type : std_logic_vector(2 downto 0);
    acquire_bits_union : std_logic_vector(16 downto 0);
    acquire_bits_data : std_logic_vector(127 downto 0);
    grant_ready : std_logic;
end record;

type bridge_in_type is record
  tile : tile_cached_out_type;
  nasti : nasti_slave_out_type;
end record;

type bridge_out_type is record
  tile : tile_cached_in_type;
  nasti : nasti_slave_in_type;
end record;

  component AxiBridge is
  port (
    clk   : in  std_logic;
    nrst  : in  std_logic;
    i_busy    : in std_logic;
    o_acquired : out std_logic;
    i     : in bridge_in_type;
    o     : out bridge_out_type
  );
  end component; 


  --! @brief Decode Acquire request from the Cached/Uncached TileLink
  --! @param[in] a_type   Request type depends of the built_in flag
  --! @param[in] built_in This flag defines cached or uncached request. For
  --!                     the uncached this value is set to 1.
  --! @param[in] u        Union bits. This value is decoding depending of
  --!                     types operation (rd/wr) and cached/uncached.
  procedure procedureDecodeTileAcquire (
    a_type    : in std_logic_vector(2 downto 0);
    built_in  : in std_logic;
    u         : in std_logic_vector(16 downto 0);
    write     : out std_logic;
    wmask     : out std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    axi_sz    : out std_logic_vector(2 downto 0);
    byte_addr : out std_logic_vector(3 downto 0);
    beat_cnt  : out integer
  );


end; -- package declaration

--! -----------------
package body types_tile is
  
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
    u         : in std_logic_vector(16 downto 0);
    write     : out std_logic;
    wmask     : out std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    axi_sz    : out std_logic_vector(2 downto 0);
    byte_addr : out std_logic_vector(3 downto 0);
    beat_cnt  : out integer
  ) is
  begin

    if built_in = '1' then
      -- Cached request
      case a_type is
      when ACQUIRE_GET_SINGLE_DATA_BEAT =>
          write := '0';
          wmask := (others => '0');
          byte_addr := u(12 downto 9);--tst.block.byte_addr;
          axi_sz := opSizeToXSize(conv_integer(u(8 downto 6)));
          beat_cnt := 0;
      when ACQUIRE_PREFETCH_BLOCK |
           ACQUIRE_GET_BLOCK_DATA =>
          -- cache line size / data bits width
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 3;--tlDataBeats-1; 
      when ACQUIRE_PUT_SINGLE_DATA_BEAT =>
          -- Single beat data.
          write := '1';
          wmask := u(16 downto 1);
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 0;
      when ACQUIRE_PUT_BLOCK_DATA =>
          -- Multibeat data.
          write := '1';
          wmask := (others => '1');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 3;--tlDataBeats-1; 
      when ACQUIRE_PUT_ATOMIC_DATA =>
          -- Single beat data. 64 bits width
          write := '1';
          if u(12) = '0' then
              wmask(7 downto 0) := (others => '1');
              wmask(15 downto 8) := (others => '0');
          else 
              wmask(7 downto 0) := (others => '0');
              wmask(15 downto 8) := (others => '1');
          end if;
          byte_addr := (others => '0');
          axi_sz := opSizeToXSize(conv_integer(u(8 downto 6)));
          beat_cnt := 0; 
      when others =>
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := (others => '0');
          beat_cnt := 0;
      end case;
    else --! built_in = '0'
      -- Cached request
      case a_type is
      when CACHED_ACQUIRE_SHARED =>
          write := '0';
          wmask := (others => '0');
          byte_addr := u(12 downto 9);--tst.block.byte_addr;
          axi_sz := opSizeToXSize(conv_integer(u(8 downto 6)));
          beat_cnt := 0;
      when CACHED_ACQUIRE_EXCLUSIVE =>
          -- Single beat data.
          write := '1';
          wmask := u(16 downto 1);
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
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