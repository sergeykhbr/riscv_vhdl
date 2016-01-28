-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--!
--! @brief     Fast Search Engine system bus bridge.
--! @details   Bridge to access from AXI4 to FSE registers,
--!            using reclocking pipes for read and write operations
--! @warnign   'BUS clock' should be equals or greater that 'FSE clock',
--!            otherwise there maybe skips of the reading data from FSE
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.gencomp.all;
library gnsslib;
use gnsslib.types_sync.all;
use gnsslib.types_fse_v2.all;
use gnsslib.types_gnss.all;
library rocketlib;
use rocketlib.types_nasti.all;

entity AxiFseBridge is
generic
(
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    did      : std_logic_vector(15 downto 0) := GNSSSENSOR_FSE_V2
);
port (
    i : in bridge_in_type;
    o : out bridge_out_type
);
end;

architecture rtl of AxiFseBridge is

  --! 4-bytes alignment so that all registers implemented as 32-bits
  --! width.
  constant ALIGNMENT_BYTES : integer := 4;
  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => did
  );

  type local_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1) 
       of integer;
  
  type regtype is record
    bank_axi : nasti_slave_bank_type;
  end record;

  type wr_fifo_type is record
    rd_ena : std_logic;
    wr_ena : std_logic;
    wdata : std_logic_vector(42 downto 0);
    rdata : std_logic_vector(42 downto 0);
    empty : std_logic;
    full  : std_logic;
  end record;

  type rd_fifo_type is record
    wr_ena  : std_logic;
    rd_ena  : std_logic;
    wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    empty : std_logic;
    full  : std_logic;
  end record;

signal io_fifo_wr : wr_fifo_type;
signal io_fifo_rd : rd_fifo_type;
signal r, rin     : regtype;
signal r_wr2fse : std_logic;

begin

  comb : process (r, i, io_fifo_rd.rdata, io_fifo_rd.empty, io_fifo_wr.empty)
    variable v : regtype;
    variable raddr_reg : local_addr_array_type;
    variable wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    variable val : std_logic_vector(8*ALIGNMENT_BYTES-1 downto 0);

    variable rd_ena : std_logic;
    variable wr_ena : std_logic;
    variable wr_adr32 : std_logic_vector(8 downto 0);
    variable wr_idx32 : integer;
  begin

    v := r;

    procedureAxi4(i.axi, xconfig, r.bank_axi, v.bank_axi);

    rd_ena := '0';
    if v.bank_axi.rstate = rtrans then
      rd_ena := '1';
    end if;

    wr_idx32 := 0;
    wr_adr32 := (others => '0');
    wr_ena := '0';
    if i.axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      wdata := i.axi.w_data;
      wstrb := i.axi.w_strb;
      for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
         if wr_ena = '0' and conv_integer(wstrb(ALIGNMENT_BYTES*(n+1)-1 downto ALIGNMENT_BYTES*n)) /= 0 then
           wr_ena := '1';
           wr_adr32 := r.bank_axi.waddr(ALIGNMENT_BYTES*n)(10 downto 2);
           wr_idx32 := n;
         end if;
      end loop;
    end if;


    io_fifo_wr.wr_ena  <= wr_ena or (rd_ena and r.bank_axi.rwaitready);
    if wr_ena = '1' then
      case wr_idx32 is
      when 0 =>  io_fifo_wr.wdata(31 downto 0) <= i.axi.w_data(31 downto 0);
      when 1 =>  io_fifo_wr.wdata(31 downto 0) <= i.axi.w_data(63 downto 32);
      when 2 =>  io_fifo_wr.wdata(31 downto 0) <= i.axi.w_data(95 downto 64);
      when 3 =>  io_fifo_wr.wdata(31 downto 0) <= i.axi.w_data(127 downto 96);
      when others =>  
      end case;
      io_fifo_wr.wdata(42 downto 32) <= wr_ena & '0' & wr_adr32;
    else
      io_fifo_wr.wdata(42 downto 0) <= '0' & rd_ena & i.axi.ar_bits.addr(10 downto 2) & zero32;
    end if;
    io_fifo_wr.rd_ena <= not io_fifo_wr.empty;

    io_fifo_rd.wr_ena  <= i.rdata_rdy;
    io_fifo_rd.wdata <= i.rdata;
    
    --! redefine value 'always ready' inserting waiting states.
    if io_fifo_rd.empty = '0' then
      v.bank_axi.rwaitready := '1';
    elsif rd_ena = '1' and r.bank_axi.rwaitready = '1' then
      v.bank_axi.rwaitready := '0';
    end if;
    io_fifo_rd.rd_ena <= not io_fifo_rd.empty;
    

    rin <= v;
    o.axi <= functionAxi4Output(r.bank_axi, io_fifo_rd.rdata);
    
  end process;

  FifoWr : afifo generic map
  (
    abits => 3,
    dbits => 43
  ) port map
  (
    i_nrst      => i.nrst,
    i_rclk      => i.clk_fse, -- reading clock
    i_rd_ena    => io_fifo_wr.rd_ena,
    o_data      => io_fifo_wr.rdata,
    o_empty     => io_fifo_wr.empty,
    i_wclk      => i.clk_bus, -- writing clock
    i_wr_ena    => io_fifo_wr.wr_ena,
    i_data      => io_fifo_wr.wdata,
    o_full      => io_fifo_wr.full
  );

  o.data <= io_fifo_wr.rdata(31 downto 0);
  o.addr <= io_fifo_wr.rdata(40 downto 32);
  o.rd_ena <= io_fifo_wr.rdata(41) and r_wr2fse;
  o.wr_ena <= io_fifo_wr.rdata(42) and r_wr2fse;

  FifoRd : afifo generic map
  (
    abits => 3,
    dbits => CFG_NASTI_DATA_BITS
  ) port map
  (
    i_nrst      => i.nrst,
    i_rclk      => i.clk_bus, -- reading clock
    i_rd_ena    => io_fifo_rd.rd_ena,
    o_data      => io_fifo_rd.rdata,
    o_empty     => io_fifo_rd.empty,
    i_wclk      => i.clk_fse, -- writing clock
    i_wr_ena    => io_fifo_rd.wr_ena,
    i_data      => io_fifo_rd.wdata,
    o_full      => io_fifo_rd.full
  );

  o.cfg  <= xconfig;

  reg : process(i.clk_bus, i.nrst)
  begin 
    if i.nrst = '0' then 
       r.bank_axi <= NASTI_SLAVE_BANK_RESET;
    elsif rising_edge(i.clk_bus) then 
      r <= rin;
    end if;
  end process;

  regfse : process(i.clk_fse, i.nrst)
  begin 
    if i.nrst = '0' then 
       r_wr2fse <= '0';
    elsif rising_edge(i.clk_fse) then 
      r_wr2fse <= not io_fifo_wr.empty;
    end if;
  end process;

end; 


