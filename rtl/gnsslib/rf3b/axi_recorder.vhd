-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2017 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     ADC samples recorder.
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

library commonlib;
use commonlib.types_common.all;

--! AMBA system bus specific library
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library techmap;
use techmap.types_mem.all;

entity axi_recorder is
  generic (
    tech     : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#ffff0#
  );
  port (
    nrst    : in  std_logic;
    clk_bus : in  std_logic;
    clk_adc : in  std_logic;
    o_cfg   : out nasti_slave_config_type;
    i_axi   : in  nasti_slave_in_type;
    o_axi   : out nasti_slave_out_type;
    i_gps_I : in std_logic_vector(1 downto 0);
    i_gps_Q : in std_logic_vector(1 downto 0)
  );
end; 
 
architecture rtl of axi_recorder is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => 0,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_ADC_RECORDER
  );

  type registers is record
    bank_axi : nasti_slave_bank_type;
    state : std_logic;
  end record;

  type adc_registers is record
      shifter : std_logic_vector(63 downto 0);
      sample_cnt : std_logic_vector(16 downto 0);
      waddr  : std_logic_vector(12 downto 0);
      we : std_logic;
      state : std_logic;
  end record;

  signal r, rin : registers;
  signal r2 : adc_registers;
  signal wb_ram_raddr : std_logic_vector(12 downto 0);
  signal wb_ram_rdata : std_logic_vector(63 downto 0);

begin

   ram2p : syncram_2p_tech generic map (
        tech => tech,
        abits => 13,
        dbits => 64,
        sepclk => 1
   ) port map (
        rclk     => clk_bus,
        renable  => '1',
        raddress => wb_ram_raddr,
        dataout  => wb_ram_rdata,
        wclk     => clk_adc,
        write    => r2.we,
        waddress => r2.waddr,
        datain   => r2.shifter
   );

  adcproc0 : process(clk_adc) begin
    if rising_edge(clk_adc) then 
        if nrst = '0' then
            r2.shifter <= (others => '0');
            r2.sample_cnt <= (others => '0');
            r2.waddr <= (others => '0');
            r2.we <= '0';
            r2.state <= '0';
        elsif r2.state = '1' then
            r2.shifter <= r2.shifter(59 downto 0) & i_gps_I & i_gps_Q;
            r2.sample_cnt <= r2.sample_cnt + 1;

            r2.waddr <= r2.sample_cnt(16 downto 4);
            r2.we <= '0';
            if r2.sample_cnt(3 downto 0) = "1111" then
               r2.we <= '1';
            end if;
            if r2.sample_cnt = (X"FFFF"&'1') then
               r2.state <= '0';
            end if;
        else 
            r2.we <= '0';
            r2.shifter <= (others => '0');
            r2.sample_cnt <= (others => '0');
            r2.state <= r.state;
        end if;
    end if;
  end process;



  comblogic : process(nrst, r, r2.state, i_axi, wb_ram_rdata)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  begin

    v := r;
    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);

    wb_ram_raddr <= r.bank_axi.raddr(0)(15 downto 3);
    
    if r.bank_axi.raddr(0)(2) = '0' then
      rdata := wb_ram_rdata;
    else
      rdata := wb_ram_rdata(63 downto 32) & wb_ram_rdata(63 downto 32);
    end if;

    if r2.state = '1' then
        v.state := '0';
    end if;

    -- write registers
    if i_axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

            v.state := '1';
    end if;
    -- reset operation

    if nrst = '0' then 
      v.bank_axi := NASTI_SLAVE_BANK_RESET;
      v.state := '0';
    end if;

    o_axi <= functionAxi4Output(r.bank_axi, rdata);
    rin <= v;
  end process;


  o_cfg <= xconfig;

  -- registers:
  regs : process(clk_bus) begin 
    if rising_edge(clk_bus) then 
       r <= rin; 
    end if;
  end process;

end;
