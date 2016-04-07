-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     This file implements RF-controller entity axi_rfctrl.
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

library commonlib;
use commonlib.types_common.all;

--! AMBA system bus specific library
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;

--! @brief     RF-front controller based on MAX2769 ICs.
--! @details   This unit implements SPI interface with MAX2769 ICs
--!            and interacts with the antenna control signals.
entity axi_rfctrl is
  generic (
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port (
    nrst    : in  std_logic;
    clk    : in  std_logic;
    o_cfg  : out nasti_slave_config_type;
    i_axi   : in  nasti_slave_in_type;
    o_axi   : out nasti_slave_out_type;
    i_gps_ld : in std_logic;
    i_glo_ld : in std_logic;
    --! @name  Synthezator's SPI interface signals:
    --! @brief Connects to MAX2769 IC.
    --! @{
    outSCLK  : out std_logic;
    outSDATA : out std_logic;
    outCSn   : out std_logic_vector(1 downto 0);
    --! @}
    
    --! @name  Antenna control signals:
    --! @brief RF front-end IO analog signals.
    --! @{
    inExtAntStat   : in std_logic;
    inExtAntDetect : in std_logic;
    outExtAntEna   : out std_logic;
    outIntAntContr   : out std_logic
    --! @}
  );
end; 
 
architecture rtl of axi_rfctrl is

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_RF_CONTROL,
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES
  );

  type local_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of integer;

  type registers is record
    bank_axi : nasti_slave_bank_type;
  
    conf1       : std_logic_vector(27 downto 0);
    conf2       : std_logic_vector(27 downto 0);
    conf3       : std_logic_vector(27 downto 0);
    pllconf     : std_logic_vector(27 downto 0);
    div         : std_logic_vector(27 downto 0);
    fdiv        : std_logic_vector(27 downto 0);
    strm        : std_logic_vector(27 downto 0);
    clkdiv      : std_logic_vector(27 downto 0);
    test1       : std_logic_vector(27 downto 0);
    test2       : std_logic_vector(27 downto 0);
    scale       : std_logic_vector(31 downto 0);
    load_run    : std_ulogic;
    select_spi  : std_logic_vector(1 downto 0);
    loading     : std_ulogic;
    ScaleCnt    : std_logic_vector(31 downto 0);
    SClkPosedge  : std_ulogic;
    SClkNegedge  : std_ulogic;
    SCLK         : std_ulogic;
    BitCnt       : integer range 0 to 33;
    CS           : std_ulogic;  --!! not inversed!!
    WordSelector : std_logic_vector(8 downto 0);
    SendWord     : std_logic_vector(31 downto 0);
    ExtAntEna    : std_ulogic;
    IntAntContr  : std_ulogic;
  end record;

signal r, rin : registers;

begin

  comblogic : process(nrst, r, i_axi, i_glo_ld, i_gps_ld, inExtAntStat, inExtAntDetect)
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
    variable wstrb : std_logic_vector(CFG_ALIGN_BYTES-1 downto 0);

  variable v : registers;
  variable readdata : std_logic_vector(31 downto 0);
  variable wNewWord : std_ulogic;
  begin

    v := r;
    v.load_run := '0';

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);

     -- read registers:
     for n in 0 to CFG_WORDS_ON_BUS-1 loop
        raddr_reg(n) := conv_integer(r.bank_axi.raddr(n)(11 downto 2));
        tmp := (others => '0');
        case raddr_reg(n) is
          when 0 => tmp  := "0000" & r.conf1;
          when 1 => tmp  := "0000" & r.conf2;
          when 2 => tmp  := "0000" & r.conf3;
          when 3 => tmp  := "0000" & r.pllconf;
          when 4 => tmp  := "0000" & r.div;
          when 5 => tmp  := "0000" & r.fdiv;
          when 6 => tmp  := "0000" & r.strm;
          when 7 => tmp  := "0000" & r.clkdiv;
          when 8 => tmp  := "0000" & r.test1;
          when 9 => tmp  := "0000" & r.test2;
          when 10 => tmp  := r.scale;
          when 11 => 
            tmp(9 downto 0):= conv_std_logic_vector(r.BitCnt,6) & '0' & r.loading & i_glo_ld & i_gps_ld;
          when 15 => 
            tmp(5 downto 0)  := inExtAntStat & inExtAntDetect & "00"& r.IntAntContr & r.ExtAntEna;
          when others =>
        end case;
        rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
     end loop;


    -- write registers
    if i_axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(n)(11 downto 2));
         tmp := i_axi.w_data(32*(n+1)-1 downto 32*n);
         wstrb := i_axi.w_strb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n);

         if conv_integer(wstrb) /= 0 then
            case waddr_reg(n) is
              when 0 => v.conf1 := tmp(27 downto 0);
              when 1 => v.conf2 := tmp(27 downto 0);
              when 2 => v.conf3 := tmp(27 downto 0);
              when 3 => v.pllconf := tmp(27 downto 0);
              when 4 => v.div := tmp(27 downto 0);
              when 5 => v.fdiv := tmp(27 downto 0);
              when 6 => v.strm := tmp(27 downto 0);
              when 7 => v.clkdiv := tmp(27 downto 0);
              when 8 => v.test1 := tmp(27 downto 0);
              when 9 => v.test2 := tmp(27 downto 0);
              when 10 => 
                if tmp(31 downto 1) = zero32(31 downto 1) then 
                   v.scale := conv_std_logic_vector(2,32);
                else 
                   v.scale := tmp;
                end if;
              when 11 => 
                v.load_run := '1';
                v.ScaleCnt := (others => '0');
                v.BitCnt := 0;
                if tmp = zero32 then 
                  v.select_spi := "01";
                elsif tmp = conv_std_logic_vector(1,32) then 
                  v.select_spi := "10";
                else
                  v.select_spi := "00";
                end if;
              when 15 => 
                v.ExtAntEna   := tmp(0);
                v.IntAntContr := tmp(1);
              when others => 
           end case;
         end if;
      end loop;
    end if;

  
    -- loading procedure:
    if((r.SClkNegedge='1') and (r.BitCnt=33)) then wNewWord := '1';
    else wNewWord := '0'; end if;
    
    if(r.load_run='1')                                   then v.loading := '1';
    elsif((wNewWord='1')and(r.WordSelector="000000000")) then v.loading := '0'; end if;
  
    if((r.loading and r.SClkNegedge)='1') then v.ScaleCnt := (others => '0');
    elsif(r.loading='1')                  then v.ScaleCnt := r.ScaleCnt+1; end if;

    -- scaler pulse:
    if((r.scale/=zero32)and(r.ScaleCnt=r.scale)) then v.SClkNegedge := '1';
    else                                              v.SClkNegedge := '0'; end if;

    if((r.scale/=zero32)and(r.ScaleCnt=('0'& r.scale(31 downto 1)))) then v.SClkPosedge := '1';
    else                                                                  v.SClkPosedge := '0'; end if;

    -- SCLK former:
    if(r.SClkPosedge='1') then v.SCLK := '1';
    elsif(r.SClkNegedge='1') then v.SCLK := '0'; end if;

    -- Not inversed CS signal:
    if((r.SClkNegedge='1')and(r.BitCnt=33)) then v.BitCnt := 0;
    elsif(r.SClkNegedge='1')                then v.BitCnt := r.BitCnt + 1; end if;
  
    if((r.BitCnt=0)or((r.BitCnt=33))) then v.CS := '0';
    else                                   v.CS := '1'; end if;

    -- Word multiplexer:
    if(r.load_run='1')  then v.WordSelector := "000000001";
    elsif(wNewWord='1') then v.WordSelector := r.WordSelector(7 downto 0) & '0'; end if;
  
    if(r.load_run='1') then                      v.SendWord := r.conf1 & "0000";
    elsif((wNewWord='1')and(r.WordSelector(0)='1')) then v.SendWord := r.conf2 & "0001";
    elsif((wNewWord='1')and(r.WordSelector(1)='1')) then v.SendWord := r.conf3 & "0010";
    elsif((wNewWord='1')and(r.WordSelector(2)='1')) then v.SendWord := r.pllconf & "0011";
    elsif((wNewWord='1')and(r.WordSelector(3)='1')) then v.SendWord := r.div & "0100";
    elsif((wNewWord='1')and(r.WordSelector(4)='1')) then v.SendWord := r.fdiv & "0101";
    elsif((wNewWord='1')and(r.WordSelector(5)='1')) then v.SendWord := r.strm & "0110";
    elsif((wNewWord='1')and(r.WordSelector(6)='1')) then v.SendWord := r.clkdiv & "0111";
    elsif((wNewWord='1')and(r.WordSelector(7)='1')) then v.SendWord := r.test1 & "1000";
    elsif((wNewWord='1')and(r.WordSelector(8)='1')) then v.SendWord := r.test2 & "1001";
    elsif((r.SClkNegedge='1')and(r.BitCnt/=0)and(r.BitCnt/=33)) then  v.SendWord := r.SendWord(30 downto 0)&'0'; end if;

    -- reset operation

    if nrst = '0' then 
      v.bank_axi := NASTI_SLAVE_BANK_RESET;
      v.load_run := '0';
      v.conf1 := (others => '0');
      v.conf2 := (others => '0');
      v.conf3 := (others => '0');
      v.pllconf := (others => '0');
      v.div := (others => '0');
      v.fdiv := (others => '0');
      v.strm := (others => '0');
      v.clkdiv := (others => '0');
      v.test1 := (others => '0');
      v.test2 := (others => '0');
      v.scale := (others => '0');
      v.SCLK := '0';
      v.BitCnt := 0;
      v.CS := '0';
      v.select_spi := (others => '0');
      v.ExtAntEna := '0';
      v.SendWord := (others=>'0');
      v.loading := '0';
      v.ScaleCnt := (others => '0');
      v.WordSelector := (others => '0');
      v.IntAntContr := '0';
    end if;

    rin <= v;
    o_axi <= functionAxi4Output(r.bank_axi, rdata);

  end process;


  o_cfg  <= xconfig;

  outSCLK   <= r.SCLK;
  outCSn(0) <= not(r.CS and r.select_spi(0));
  outCSn(1) <= not(r.CS and r.select_spi(1));
  outSDATA  <= r.SendWord(31);

  outExtAntEna <= r.ExtAntEna;
  outIntAntContr <= r.IntAntContr;


  -- registers:
  regs : process(clk, nrst) begin 
    if rising_edge(clk) then 
       r <= rin; 
    end if;
  end process;

end;
