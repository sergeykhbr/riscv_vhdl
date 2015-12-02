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
library rocketlib;
use rocketlib.types_nasti.all;
use rocketlib.types_tile.all;
use rocketlib.types_rocket.all;

entity AxiBridge is
  port (
    clk   : in  std_logic;
    nrst  : in  std_logic;
    i_busy    : in std_logic;
    o_acquired : out std_logic;
    i     : in bridge_in_type;
    o     : out bridge_out_type
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

  comblogic : process(i, i_busy, r)
    variable v : registers;
    variable vo     : bridge_out_type;
    
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
    
    vo.nasti.aw_valid        := '0';
    vo.nasti.aw_bits         := META_NONE;
    vo.nasti.aw_id           := (others => '0');
    vo.nasti.w_valid         := '0';
    vo.nasti.w_data          := (others => '0');
    vo.nasti.w_last          := '0';
    vo.nasti.w_strb          := (others => '0');
    vo.nasti.ar_valid        := '0';
    vo.nasti.ar_bits         := META_NONE;
    vo.nasti.ar_id           := (others => '0');
    vo.nasti.r_ready         := '0';
    vo.nasti.ar_user         := '0';
    vo.nasti.aw_user         := '0';
    vo.nasti.w_user          := '0';
    vo.nasti.b_ready         := '1';
          
    vo.tile.acquire_ready := '0';
    vo.tile.probe_valid   := '0'; -- unused
    vo.tile.release_ready := '0'; -- unused
    vo.tile.grant_valid   := '0';
    vo.tile.grant_bits_addr_beat       := "00";
    vo.tile.grant_bits_client_xact_id  := "00";
    vo.tile.grant_bits_manager_xact_id := "0000"; -- const
    vo.tile.grant_bits_is_builtin_type := '1';    -- const
    vo.tile.grant_bits_g_type          := GRANT_ACK_RELEASE;
    vo.tile.grant_bits_data            := (others => '0');
    
    procedureDecodeTileAcquire(i.tile.acquire_bits_a_type,--in
                               i.tile.acquire_bits_is_builtin_type, --in
                               i.tile.acquire_bits_union, --in
                               wWrite,
                               wbWMask,
                               wbAxiSize,
                               wbByteAddr,
                               wbBeatCnt);

    wbAddr := i.tile.acquire_bits_addr_block 
            & i.tile.acquire_bits_addr_beat 
            & "0000";--wbByteAddr;
    
    case r.wstate is
    when wwait_acq =>
        if i.tile.acquire_valid = '1' and wWrite = '1' and i_busy = '0' and r.rstate = rwait_acq then
                                    
          v.wr_addr      := wbAddr;
          v.wr_addr_incr := XSizeToBytes(conv_integer(wbAxiSize));
          v.wr_beat_cnt  := wbBeatCnt;
          v.wr_xsize     := opSizeToXSize(conv_integer(i.tile.acquire_bits_union(8 downto 6)));
          v.wr_xact_id   := i.tile.acquire_bits_client_xact_id;
          if i.tile.acquire_bits_is_builtin_type = '1' then
              v.wr_g_type    := GRANT_ACK_NON_PREFETCH_PUT;
          else
              v.wr_g_type    := CACHED_GRANT_EXCLUSIVE_ACK;
          end if;
          v.wmask        := wbWMask;

          if i.nasti.aw_ready = '1' then
             v.wstate := writting;
             v.wdata := i.tile.acquire_bits_data;
          end if;
          vo.nasti.aw_valid        := '1';
          vo.nasti.aw_bits         := functionAxi4MetaData(wbAddr, wbBeatCnt, wbAxiSize);
          vo.nasti.aw_id(1 downto 0) := i.tile.acquire_bits_client_xact_id;
          vo.nasti.aw_id(CFG_ROCKET_ID_BITS-1 downto 2) := (others => '0');

          vo.tile.acquire_ready := i.tile.acquire_valid and i.nasti.aw_ready;
        end if;

    when writting =>
          if r.wr_beat_cnt = 0 and i.nasti.w_ready = '1' then
              vo.nasti.w_last := '1';
              v.wstate := wwait_acq;
          elsif i.nasti.w_ready = '1' and i.tile.acquire_valid = '1' then
             v.wr_beat_cnt := r.wr_beat_cnt - 1;
             v.wr_addr := r.wr_addr + r.wr_addr_incr;
             v.wdata := i.tile.acquire_bits_data;
          end if;
          vo.nasti.w_valid         := '1';
          vo.nasti.w_data          := r.wdata;
          vo.nasti.w_strb          := r.wmask;
    when others => 
    end case;


    case r.rstate is
    when rwait_acq =>
        if i.tile.acquire_valid = '1' and wWrite = '0' and i_busy = '0' and r.wstate = wwait_acq then

          v.rd_addr := wbAddr;
          v.rd_addr_incr := XSizeToBytes(conv_integer(wbAxiSize));
          v.rd_beat_cnt := wbBeatCnt;
          v.rd_xsize := opSizeToXSize(conv_integer(i.tile.acquire_bits_union(8 downto 6)));
          v.rd_xact_id := i.tile.acquire_bits_client_xact_id;
          if i.tile.acquire_bits_is_builtin_type = '1' then
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


          if i.nasti.ar_ready = '1' then
            v.rstate := reading;
          end if;
          vo.nasti.ar_valid        := '1';
          vo.nasti.ar_bits         := functionAxi4MetaData(wbAddr, wbBeatCnt, wbAxiSize);
          vo.nasti.ar_id(1 downto 0) := i.tile.acquire_bits_client_xact_id;
          vo.nasti.ar_id(CFG_ROCKET_ID_BITS-1 downto 2) := (others => '0');

          vo.tile.acquire_ready := i.tile.acquire_valid and i.nasti.ar_ready;
        end if;

    when reading =>
          next_ena := i.tile.grant_ready and i.nasti.r_valid;
          if next_ena = '1' and r.rd_xact_id = i.nasti.r_id(1 downto 0) then
              v.rd_beat_cnt := r.rd_beat_cnt - 1;
              v.rd_addr := r.rd_addr + r.rd_addr_incr;
              if r.rd_beat_cnt = 0 then
                 v.rstate := rwait_acq;
              end if;
          end if;
          vo.nasti.r_ready         := i.tile.grant_ready;
    when others => 
    end case;

    if r.rstate = reading then
        if r.rd_xact_id = i.nasti.r_id(1 downto 0) then
          vo.tile.grant_valid                := i.nasti.r_valid;
        else
          vo.tile.grant_valid                := '0';
        end if;
        vo.tile.grant_bits_addr_beat       := r.rd_addr(5 downto 4);
        vo.tile.grant_bits_client_xact_id  := r.rd_xact_id;
        vo.tile.grant_bits_g_type          := r.rd_g_type;
        vo.tile.grant_bits_data            := i.nasti.r_data;
        o_acquired <= '1';
    elsif r.wstate = writting then
        vo.tile.grant_valid               := i.nasti.w_ready;
        vo.tile.grant_bits_addr_beat      := r.wr_addr(5 downto 4);
        vo.tile.grant_bits_client_xact_id := r.wr_xact_id;
        vo.tile.grant_bits_g_type         := r.wr_g_type;
        vo.tile.grant_bits_data           := (others => '0');
        o_acquired <= '1';
    else
        o_acquired <= '0';
    end if;
    
    rin <= v;
    o <= vo;

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
