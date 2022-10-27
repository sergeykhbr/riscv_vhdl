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
  port (
    clk   : in  std_logic;
    nrst  : in  std_logic;

    --! Tile-to-AXI direction
    tloi : in tile_out_type;
    msto : out nasti_master_out_type;
    --! AXI-to-Tile direction
    msti : in nasti_master_in_type;
    tlio : out tile_in_type
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
    rd_xact_id : std_logic_vector(2 downto 0);
    rd_g_type : std_logic_vector(3 downto 0);

    wstate : tile_wstatetype;
    wr_addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    wr_addr_incr : integer;
    wr_beat_cnt : integer;
    wr_xsize : std_logic_vector(2 downto 0); -- encoded AXI4 bytes size
    wr_xact_id : std_logic_vector(2 downto 0);
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
    variable vtlio  : tile_in_type;

    
    variable addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    variable write : std_logic;
    variable next_ena : std_logic;
    
    variable wWrite     : std_logic;
    variable wb_xsize : std_logic_vector(2 downto 0);    
    variable wbByteAddr : std_logic_vector(2 downto 0);    

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
          
    vtlio.a_ready := '0';
    vtlio.b_valid := '0';
    vtlio.b_opcode := "000";
    vtlio.b_param := "00";
    vtlio.b_size := "0000";
    vtlio.b_source := "000";
    vtlio.b_address := (others => '0');
    vtlio.b_mask := (others => '0');
    vtlio.b_data := (others => '0');
    vtlio.c_ready := '0';
    vtlio.d_valid := '0';
    vtlio.d_opcode := "001";
    vtlio.d_param := "00";
    vtlio.d_size := "0000";
    vtlio.d_source := "000";
    vtlio.d_sink := "0000";
    vtlio.d_addr_lo := "000";
    vtlio.d_data := (others => '0');
    vtlio.d_error := '0';
    vtlio.e_ready := '1';

    wWrite := not tloi.a_opcode(2);
    
    if tloi.a_size(3 downto 2) /= "00" then
       wb_xsize     := "011";
    else
       wb_xsize     := '0' & tloi.a_size(1 downto 0);
    end if;

    vmsto.aw_valid        := tloi.a_valid and wWrite;
    vmsto.ar_valid        := tloi.a_valid and not wWrite;

    case r.wstate is
    when wwait_acq =>
        if vmsto.aw_valid = '1' and r.rstate = rwait_acq then
                                    
          v.wr_xsize     := wb_xsize;
          v.wr_addr      := tloi.a_address;
          v.wr_addr_incr := XSizeToBytes(conv_integer(wb_xsize));
          v.wr_beat_cnt  := conv_integer(tloi.a_size(3 downto 2));
          v.wr_xact_id   := tloi.a_source;
          v.wmask        := tloi.a_mask;

          if msti.aw_ready = '1' then
             v.wstate := writting;
             v.wdata := tloi.a_data;
          end if;
          vmsto.aw_bits         := functionAxi4MetaData(tloi.a_address, v.wr_beat_cnt, wb_xsize);
          vmsto.aw_id(2 downto 0) := tloi.a_source;
          vmsto.aw_id(CFG_ROCKET_ID_BITS-1 downto 3) := (others => '0');

          vtlio.a_ready := tloi.a_valid and msti.aw_ready;
        end if;

    when writting =>
          if r.wr_beat_cnt = 0 and msti.w_ready = '1' then
              vmsto.w_last := '1';
              v.wstate := wwait_acq;
          elsif msti.w_ready = '1' and tloi.a_valid = '1' then
             v.wr_beat_cnt := r.wr_beat_cnt - 1;
             v.wr_addr := r.wr_addr + r.wr_addr_incr;
             v.wdata := tloi.a_data;
          end if;
          vmsto.w_valid         := '1';
          vmsto.w_data          := r.wdata;
          vmsto.w_strb          := r.wmask;
    when others => 
    end case;


    case r.rstate is
    when rwait_acq =>
        if vmsto.ar_valid = '1' and r.wstate = wwait_acq then

          v.rd_addr := tloi.a_address;
          v.rd_addr_incr := XSizeToBytes(conv_integer(wb_xsize));
          v.rd_beat_cnt := conv_integer(tloi.a_size(3 downto 2));
          v.rd_xsize := wb_xsize;
          v.rd_xact_id := tloi.a_source;

          if msti.ar_ready = '1' then
            v.rstate := reading;
          end if;
          vmsto.ar_bits         := functionAxi4MetaData(tloi.a_address, v.rd_beat_cnt, wb_xsize);
          vmsto.ar_id(2 downto 0) := tloi.a_source;
          vmsto.ar_id(CFG_ROCKET_ID_BITS-1 downto 3) := (others => '0');

          vtlio.a_ready := tloi.a_valid and msti.ar_ready;
        end if;

    when reading =>
          next_ena := tloi.d_ready and msti.r_valid;
          if next_ena = '1' and r.rd_xact_id = msti.r_id(2 downto 0) then
              v.rd_beat_cnt := r.rd_beat_cnt - 1;
              v.rd_addr := r.rd_addr + r.rd_addr_incr;
              if r.rd_beat_cnt = 0 then
                 v.rstate := rwait_acq;
              end if;
          end if;
          vmsto.r_ready         := tloi.d_ready;
    when others => 
    end case;

    if r.rstate = reading then
        if r.rd_xact_id = msti.r_id(2 downto 0) then
          vtlio.d_valid                := msti.r_valid;
        else
          vtlio.d_valid                := '0';
        end if;
        vtlio.d_size := "0110";
        vtlio.d_addr_lo       := r.rd_addr(5 downto 3);--!!  depends on AXI_DATA_WIDTH
        vtlio.d_source  := r.rd_xact_id;
        --vtlio.grant_bits_g_type          := r.rd_g_type;
        vtlio.d_data            := msti.r_data;
    elsif r.wstate = writting then
        vtlio.d_valid               := msti.w_ready;
        vtlio.d_addr_lo      := r.wr_addr(5 downto 3);--!!  depends on AXI_DATA_WIDTH
        vtlio.d_source := r.wr_xact_id;
        --vtlio.grant_bits_g_type         := r.wr_g_type;
        --vtlio.grant_bits_data           := (others => '0');
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
