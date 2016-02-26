-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     TileLink-to-AXI4 bridge implementation.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library rocketlib;
use rocketlib.types_rocket.all;

entity AxiBridge is
  generic (
     xindex : integer := 0
  );
  port (
    clk   : in  std_logic;
    nrst  : in  std_logic;

    --! Tile-to-AXI direction
    tloi : in tile_cached_out_type;
    msto : out nasti_master_out_type;
    --! AXI-to-Tile direction
    msti : in nasti_master_in_type;
    tlio : out tile_cached_in_type
  );
end; 
 
architecture arch_AxiBridge of AxiBridge is

  type tile_rstatetype is (rwait_acq, reading);
  type tile_wstatetype is (wwait_acq, writting);
    
  type registers is record
    rstate : tile_rstatetype;
    rd_addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    rd_addr_incr : integer;
    rd_beat_cnt : integer;
    rd_xsize : std_logic_vector(2 downto 0); -- encoded AXI4 bytes size
    rd_xact_id : std_logic_vector(1 downto 0);
    rd_g_type : std_logic_vector(3 downto 0);

    wstate : tile_wstatetype;
    wr_addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    wr_addr_incr : integer;
    wr_beat_cnt : integer;
    wr_xsize : std_logic_vector(2 downto 0); -- encoded AXI4 bytes size
    wr_xact_id : std_logic_vector(1 downto 0);
    wr_g_type : std_logic_vector(3 downto 0);
    wmask : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  end record;


signal r, rin : registers;


  function functionAxi4MetaData(
    a   : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    len : integer;
    sz  : std_logic_vector(2 downto 0)
  ) return nasti_metadata_type is
    variable        ret : nasti_metadata_type;
  begin
        ret.addr    := a;
        ret.len     := conv_std_logic_vector(len,8);
        ret.size    := sz;
        ret.burst   := NASTI_BURST_INCR;
        ret.lock    := '0';
        ret.cache   := (others => '0');
        ret.prot    := (others => '0');
        ret.qos     := (others => '0');
        ret.region  := (others => '0');
        return (ret);
  end function;

begin

  comblogic : process(tloi, msti, r)
    variable v : registers;
    variable vmsto : nasti_master_out_type;
    variable vtlio  : tile_cached_in_type;

    
    variable addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    variable write : std_logic;
    variable next_ena : std_logic;
    
    variable wbAddr     : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    variable wWrite     : std_logic;
    variable wbWMask    : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);    
    variable wbAxiSize  : std_logic_vector(2 downto 0);    
    variable wbByteAddr : std_logic_vector(3 downto 0);    
    variable wbBeatCnt  : integer;

  begin

    v := r;
    addr := (others => '0');
    write := '0';
    
    vmsto.aw_valid        := '0';
    vmsto.aw_bits         := META_NONE;
    vmsto.aw_id           := (others => '0');
    vmsto.w_valid         := '0';
    vmsto.w_data          := (others => '0');
    vmsto.w_last          := '0';
    vmsto.w_strb          := (others => '0');
    vmsto.ar_valid        := '0';
    vmsto.ar_bits         := META_NONE;
    vmsto.ar_id           := (others => '0');
    vmsto.r_ready         := '0';
    vmsto.ar_user         := '0';
    vmsto.aw_user         := '0';
    vmsto.w_user          := '0';
    vmsto.b_ready         := '1';
          
    vtlio.acquire_ready := '0';
    vtlio.probe_valid   := '0'; -- unused
    vtlio.release_ready := '0'; -- unused
    vtlio.grant_valid   := '0';
    vtlio.grant_bits_addr_beat       := "00";
    vtlio.grant_bits_client_xact_id  := "00";
    vtlio.grant_bits_manager_xact_id := "0000"; -- const
    vtlio.grant_bits_is_builtin_type := '1';    -- const
    vtlio.grant_bits_g_type          := GRANT_ACK_RELEASE;
    vtlio.grant_bits_data            := (others => '0');
    
    procedureDecodeTileAcquire(tloi.acquire_bits_a_type,--in
                               tloi.acquire_bits_is_builtin_type, --in
                               tloi.acquire_bits_union, --in
                               wWrite,
                               wbWMask,
                               wbAxiSize,
                               wbByteAddr,
                               wbBeatCnt);

    wbAddr := tloi.acquire_bits_addr_block 
            & tloi.acquire_bits_addr_beat 
            & "0000";--wbByteAddr;

    vmsto.aw_valid        := tloi.acquire_valid and wWrite;
    vmsto.ar_valid        := tloi.acquire_valid and not wWrite;

    case r.wstate is
    when wwait_acq =>
        if vmsto.aw_valid = '1' and msti.grant(xindex) = '1' and r.rstate = rwait_acq then
                                    
          v.wr_addr      := wbAddr;
          v.wr_addr_incr := XSizeToBytes(conv_integer(wbAxiSize));
          v.wr_beat_cnt  := wbBeatCnt;
          v.wr_xsize     := opSizeToXSize(conv_integer(tloi.acquire_bits_union(8 downto 6)));
          v.wr_xact_id   := tloi.acquire_bits_client_xact_id;
          if tloi.acquire_bits_is_builtin_type = '1' then
              v.wr_g_type    := GRANT_ACK_NON_PREFETCH_PUT;
          else
              v.wr_g_type    := CACHED_GRANT_EXCLUSIVE_ACK;
          end if;
          v.wmask        := wbWMask;

          if msti.aw_ready = '1' then
             v.wstate := writting;
             v.wdata := tloi.acquire_bits_data;
          end if;
          vmsto.aw_bits         := functionAxi4MetaData(wbAddr, wbBeatCnt, wbAxiSize);
          vmsto.aw_id(1 downto 0) := tloi.acquire_bits_client_xact_id;
          vmsto.aw_id(CFG_ROCKET_ID_BITS-1 downto 2) := (others => '0');

          vtlio.acquire_ready := tloi.acquire_valid and msti.aw_ready;
        end if;

    when writting =>
          if r.wr_beat_cnt = 0 and msti.w_ready = '1' then
              vmsto.w_last := '1';
              v.wstate := wwait_acq;
          elsif msti.w_ready = '1' and tloi.acquire_valid = '1' then
             v.wr_beat_cnt := r.wr_beat_cnt - 1;
             v.wr_addr := r.wr_addr + r.wr_addr_incr;
             v.wdata := tloi.acquire_bits_data;
          end if;
          vmsto.w_valid         := '1';
          vmsto.w_data          := r.wdata;
          vmsto.w_strb          := r.wmask;
    when others => 
    end case;


    case r.rstate is
    when rwait_acq =>
        if vmsto.ar_valid = '1' and msti.grant(xindex) = '1' and r.wstate = wwait_acq then

          v.rd_addr := wbAddr;
          v.rd_addr_incr := XSizeToBytes(conv_integer(wbAxiSize));
          v.rd_beat_cnt := wbBeatCnt;
          v.rd_xsize := opSizeToXSize(conv_integer(tloi.acquire_bits_union(8 downto 6)));
          v.rd_xact_id := tloi.acquire_bits_client_xact_id;
          if tloi.acquire_bits_is_builtin_type = '1' then
            if wbBeatCnt = 0 then
              v.rd_g_type := GRANT_SINGLE_BEAT_GET;
            else
              v.rd_g_type := GRANT_BLOCK_GET;
            end if;
          else
              v.wr_g_type    := CACHED_GRANT_SHARED;
              
              --wbBeatCnt := 3;
              --wbAxiSize := "100";
              --v.rd_addr_incr := XSizeToBytes(conv_integer(wbAxiSize));
              --v.rd_beat_cnt  := wbBeatCnt;
              --v.rd_xsize     := wbAxiSize;
          end if;


          if msti.ar_ready = '1' then
            v.rstate := reading;
          end if;
          vmsto.ar_bits         := functionAxi4MetaData(wbAddr, wbBeatCnt, wbAxiSize);
          vmsto.ar_id(1 downto 0) := tloi.acquire_bits_client_xact_id;
          vmsto.ar_id(CFG_ROCKET_ID_BITS-1 downto 2) := (others => '0');

          vtlio.acquire_ready := tloi.acquire_valid and msti.ar_ready;
        end if;

    when reading =>
          next_ena := tloi.grant_ready and msti.r_valid;
          if next_ena = '1' and r.rd_xact_id = msti.r_id(1 downto 0) then
              v.rd_beat_cnt := r.rd_beat_cnt - 1;
              v.rd_addr := r.rd_addr + r.rd_addr_incr;
              if r.rd_beat_cnt = 0 then
                 v.rstate := rwait_acq;
              end if;
          end if;
          vmsto.r_ready         := tloi.grant_ready;
    when others => 
    end case;

    if r.rstate = reading then
        if r.rd_xact_id = msti.r_id(1 downto 0) then
          vtlio.grant_valid                := msti.r_valid;
        else
          vtlio.grant_valid                := '0';
        end if;
        vtlio.grant_bits_addr_beat       := r.rd_addr(5 downto 4);
        vtlio.grant_bits_client_xact_id  := r.rd_xact_id;
        vtlio.grant_bits_g_type          := r.rd_g_type;
        vtlio.grant_bits_data            := msti.r_data;
    elsif r.wstate = writting then
        vtlio.grant_valid               := msti.w_ready;
        vtlio.grant_bits_addr_beat      := r.wr_addr(5 downto 4);
        vtlio.grant_bits_client_xact_id := r.wr_xact_id;
        vtlio.grant_bits_g_type         := r.wr_g_type;
        vtlio.grant_bits_data           := (others => '0');
    end if;
    
    rin <= v;
    tlio <= vtlio;
    msto  <= vmsto;

  end process;


  -- registers:
  regs : process(clk, nrst)
  begin 
     if nrst = '0' then
        r.rstate <= rwait_acq;
        r.wstate <= wwait_acq;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;
