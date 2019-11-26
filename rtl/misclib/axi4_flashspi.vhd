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

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library misclib;
use misclib.types_misc.all;

entity axi4_flashspi is
  generic (
    async_reset : boolean := false;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    wait_while_write : boolean := true  -- hold AXI bus response until end of write cycle
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out axi4_slave_config_type;
    i_spi  : in  spi_in_type;
    o_spi  : out spi_out_type;
    i_axi  : in  axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type  );
end; 
 
architecture arch_axi4_flashspi of axi4_flashspi is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_SPI_FLASH
  );

  constant zero32 : std_logic_vector(31 downto 0) := (others => '0');

  type state_type is (idle, wsetup, rsetup, txcmd, rbyte, wbyte, rd_complete, wr_complete, wr_accept);
  type page_buf_type is array (0 to 31) of std_logic_vector(31 downto 0); --128 bytes

  type registers is record
     scaler : std_logic_vector(31 downto 0);
     scaler_cnt : std_logic_vector(31 downto 0);
     state : state_type;
     rready : std_logic;
     wready : std_logic;

     -- Access to control registers always 4 bytes only
     raddr : std_logic_vector(17 downto 2);
     rdata : std_logic_vector(63 downto 0);
     waddr : std_logic_vector(7 downto 2);
     wdata : std_logic_vector(31 downto 0);

     csn : std_logic;
     sck : std_logic;
     op64 : std_logic;
     so_shifter : std_logic_vector(31 downto 0);
     si_shifter : std_logic_vector(63 downto 0);
     cmdbit_cnt : integer range 0 to 31;
     databyte_cnt : integer range 0 to 255;
     databyte_mask : std_logic_vector(6 downto 0);
     wraccess : std_logic;
     bytes_received : integer range 0 to 8;
     buf_addr : std_logic_vector(6 downto 0);
  end record;

  constant R_RESET : registers := (
    (others => '0'), (others => '0'), idle, -- scaler, scaler_cnt, state
    '0', '0',                          -- rready, wready
    (others => '0'), (others => '0'),  -- raddr, rdata
    (others => '0'), (others => '0'),  -- waddr, wdata
    '1', '0', '0',                     -- csn, sck, op64
    (others => '0'), (others => '0'),  -- so_shifter, si_shifter
    0, 0,                              -- cmdbit_cnt, databyte_cnt
    (others => '0'), '0',              -- databyte_mask, wraccess
    0, (others => '0')                 -- bytes_received, buf_addr
  );

  signal wb_page_addr : std_logic_vector(4 downto 0);
  signal wb_page_rdata0 : std_logic_vector(31 downto 0);
  signal wb_page_wdata0 : std_logic_vector(31 downto 0);
  signal w_page_we0 : std_logic;
  signal pagebuf0 : page_buf_type;

  signal wb_page_rdata1 : std_logic_vector(31 downto 0);
  signal wb_page_wdata1 : std_logic_vector(31 downto 0);
  signal w_page_we1 : std_logic;
  signal pagebuf1 : page_buf_type;

  signal wb_bus_raddr : global_addr_array_type;
  signal w_bus_re    : std_logic;
  signal wb_bus_waddr : global_addr_array_type;
  signal w_bus_we    : std_logic;
  signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  signal w_data_ready : std_logic;

  signal r, rin : registers;

begin

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i_axi,
    o_xslvo => o_axi,
    i_ready => w_data_ready,
    i_rdata => r.rdata,
    o_re => w_bus_re,
    o_r32 => open,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );

  comblogic : process(nrst, i_spi, r, wb_page_rdata0, wb_page_rdata1,
                      w_bus_re, wb_bus_raddr, wb_bus_waddr, w_bus_we,
                      wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);

    variable posedge_flag : std_logic;
    variable negedge_flag : std_logic;

    variable vb_page_addr_mux : std_logic_vector(4 downto 0);
    variable vb_page_addr : std_logic_vector(4 downto 0);
    variable vb_page_wdata0 : std_logic_vector(31 downto 0);
    variable v_page_we0 : std_logic;
    variable vb_page_wdata1 : std_logic_vector(31 downto 0);
    variable v_page_we1 : std_logic;
  begin

    v := r;

    vb_page_addr := (others => '0');
    vb_page_wdata0 := (others => '0');
    v_page_we0 := '0';
    vb_page_wdata1 := (others => '0');
    v_page_we1 := '0';

    v.rready := '0';
    v.wready := '0';
    w_data_ready <= r.rready or r.wready;

    -- system bus clock scaler to baudrate:
    posedge_flag := '0';
    negedge_flag := '0';
    if r.scaler /= zero32 then
        if r.csn = '1' then
            v.scaler_cnt := zero32;
            v.sck := '0';
        elsif r.scaler_cnt = (r.scaler-1) then
            v.scaler_cnt := zero32;
            v.sck := not r.sck;
            posedge_flag := not r.sck;
            negedge_flag := r.sck;
        else
            v.scaler_cnt := r.scaler_cnt + 1;
        end if;
    end if;

    case r.state is
    when idle =>
        v.so_shifter := (others => '0');
        v.csn := '1';
        v.sck := '0';
    when rsetup =>
       v.wraccess := '0';
       v.bytes_received := 0;
       if r.raddr(17) = '1' then
          -- Control registers:
          case conv_integer(r.raddr(16 downto 2)) is
              when 0 =>
                  v.state := rd_complete;
                  v.si_shifter(31 downto 0) := r.scaler;
                  v.si_shifter(63 downto 32) := (others => '0');
                  v.bytes_received := 1;   -- to avoid bytes swapping
              when 4 => -- Read Flash STATUS
                  v.state := txcmd;
                  v.csn := '0';
                  v.cmdbit_cnt := 7;
                  v.databyte_cnt := 0;
                  v.databyte_mask := (others => '0'); -- Clear mask to enable 'rbyte' state
                  v.so_shifter := X"05000000";
                  v.si_shifter := (others => '0');
              when 6 => -- Read Flash ID and Release from Deep Power-down
                  v.state := txcmd;
                  v.csn := '0';
                  v.cmdbit_cnt := 31;
                  v.databyte_cnt := 0;                 -- Read one Byte Manufacturer ID = 0x29
                  v.databyte_mask := (others => '0');  -- Clear mask to enable 'rbyte' state
                  v.so_shifter := X"AB000000";
                  v.si_shifter := (others => '0');
              when others =>
                  v.state := rd_complete;
                  v.si_shifter := (others => '0');
          end case;
       else
          -- Access to SPI
          v.state := txcmd;
          v.csn := '0';
          v.cmdbit_cnt := 31;
          if r.op64 = '1' then
              v.databyte_cnt := 7;
          else
              v.databyte_cnt := 3;
          end if;
          v.databyte_mask := (others => '0');

          -- [31:24] - command 0x3 = READ
          -- [23:17] - ignored by flash
          -- [16:0] - address
          v.so_shifter := X"03" & "0000000" & r.raddr(16 downto 2) & "00";
       end if;
    when wsetup =>
        -- Only control request. Write to page buffer doesn't get here:
        v.wraccess := '1';
        case conv_integer(r.waddr) is
        when 0 =>
            v.state := wr_complete;
            v.scaler := r.wdata;
        when 4 => -- Write Flash STATUS
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 15;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"01" & r.wdata(7 downto 0) & X"0000";
        when 8 => -- Write Enable
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 7;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"06000000";
        when 10 => -- Page Write
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 31;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '0'); -- Clear mask to enter 'wbyte' state
            v.so_shifter := X"02" & r.wdata(23 downto 8) & X"00";
            v.buf_addr := (others => '0');
        when 12 => -- Write Disable
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 7;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"04000000";
        when 14 => -- Page Erase
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 31;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"42" & r.wdata(23 downto 0);
        when 16 => -- Sector Erase
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 31;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"D8" & r.wdata(23 downto 0);
        when 18 => -- Chip Erase
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 7;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"C7000000";
        when 20 => -- Deep Power-Down mode enable
            v.state := txcmd;
            v.csn := '0';
            v.cmdbit_cnt := 7;
            v.databyte_cnt := 0;
            v.databyte_mask := (others => '1'); -- Set mask to skip 'wbyte' state
            v.so_shifter := X"B9000000";
        when others =>
            v.state := wr_complete;
        end case;

    when txcmd =>
       if negedge_flag = '1' then
           v.so_shifter := r.so_shifter(30 downto 0) & "0";
           if r.cmdbit_cnt = 0 then
               if r.databyte_mask = "0000000" then
                   if r.wraccess = '1' then
                       v.state := wbyte;
                       -- Transmit 256 bytes of page buffer starting from 0 offset
                       v.so_shifter := wb_page_rdata0(7 downto 0) & wb_page_rdata0(15 downto 8)
                                    & wb_page_rdata0(23 downto 16) & wb_page_rdata0(31 downto 24);
                       v.buf_addr := r.buf_addr + 1;
                       v.databyte_cnt := 3;
                   else
                       v.state := rbyte;
                   end if;
               else
                   if r.wraccess = '1' then
                       v.state := wr_complete;
                   else
                       v.state := rd_complete;
                   end if;
               end if;
           else
               v.cmdbit_cnt := r.cmdbit_cnt - 1;
           end if;
       end if;

    when rbyte =>
       if posedge_flag = '1' then
           v.si_shifter := r.si_shifter(62 downto 0) & i_spi.SDI;
           v.databyte_mask := r.databyte_mask(5 downto 0) & '1';
           if r.databyte_mask = "1111111" then
               v.bytes_received := r.bytes_received + 1;
               if r.databyte_cnt = 0 then
                    v.state := rd_complete;
               else
                    v.databyte_cnt := r.databyte_cnt - 1;
                    v.databyte_mask := (others => '0');
               end if;
           end if;
       end if;
      
    when rd_complete =>
       v.rready := '1';   -- End of access wait-states
       if r.bytes_received = 8 then
           v.rdata := r.si_shifter(7 downto 0) & r.si_shifter(15 downto 8)
                    & r.si_shifter(23 downto 16) & r.si_shifter(31 downto 24)
                    & r.si_shifter(39 downto 32) & r.si_shifter(47 downto 40)
                    & r.si_shifter(55 downto 48) & r.si_shifter(63 downto 56);
       elsif r.bytes_received = 4 then
           v.rdata := r.si_shifter(7 downto 0) & r.si_shifter(15 downto 8)
                    & r.si_shifter(23 downto 16) & r.si_shifter(31 downto 24)
                    & r.si_shifter(7 downto 0) & r.si_shifter(15 downto 8)
                    & r.si_shifter(23 downto 16) & r.si_shifter(31 downto 24);
       else
           v.rdata := r.si_shifter;
       end if;
       v.state := idle;

    when wbyte =>
       if negedge_flag = '1' then
           v.so_shifter := r.so_shifter(30 downto 0) & "0";
           v.databyte_mask := r.databyte_mask(5 downto 0) & '1';
           if r.databyte_mask = "1111111" then
               v.databyte_mask := (others => '0');
               if r.databyte_cnt = 0 then
                    v.buf_addr := r.buf_addr + 1;
                    if conv_integer(r.buf_addr) = 64 then
                        v.state := wr_complete;
                    elsif r.buf_addr(0) = '1' then
                        v.databyte_cnt := 3;
                        v.so_shifter := wb_page_rdata1(7 downto 0) & wb_page_rdata1(15 downto 8)
                                    & wb_page_rdata1(23 downto 16) & wb_page_rdata1(31 downto 24);
                    else 
                        v.databyte_cnt := 3;
                        v.so_shifter := wb_page_rdata0(7 downto 0) & wb_page_rdata0(15 downto 8)
                                    & wb_page_rdata0(23 downto 16) & wb_page_rdata0(31 downto 24);
                    end if;
               else
                    v.databyte_cnt := r.databyte_cnt - 1;
               end if;
           end if;
       end if;

    when wr_complete =>
       if wait_while_write then
           v.wready := '1';   -- End of access wait-states
       end if;
       v.state := wr_accept;
       v.csn := '1';
    when wr_accept =>
       -- To avoid re-accept the same write request
       v.state := idle;
    when others =>
    end case;


    if w_bus_re = '1' then
         v.state := rsetup;
         v.raddr := wb_bus_raddr(0)(17 downto 2);
         if wb_bus_raddr(0)(2) = '1' then
             v.op64 := '0';
         else
             v.op64 := '1';
         end if;
    end if;


    -- Wait states: Read and Write transaction takes at least 1 wait state, except
    --     0 clocks (no wait states). Writing into page buffer
    --     1 clock. Read/Write control register without access to SPI Flash (scaler, example)
    --     N clocks. When access to Flash, depending length of SPI sequence and scaler.
    if w_bus_we = '1' then

        wstrb := wb_bus_wstrb;

        if wb_bus_waddr(0)(17) = '0' then
            -- Write to page buffer
            v.wready := '1';  -- No wait states needed
            vb_page_addr := wb_bus_waddr(0)(7 downto 3);
            if wb_bus_waddr(0)(2) = '0' then
                -- 4 or 8 bytes
                v_page_we0 := wstrb(3) or wstrb(2) or wstrb(1) or wstrb(0);
                vb_page_wdata0 := wb_bus_wdata(31 downto 0);
                v_page_we1 := wstrb(7) or wstrb(6) or wstrb(5) or wstrb(4);
                vb_page_wdata1 := wb_bus_wdata(63 downto 32);
            else
                -- 4-bytes only
                v_page_we0 := '0';
                vb_page_wdata0 := (others => '0');
                v_page_we1 := wstrb(3) or wstrb(2) or wstrb(1) or wstrb(0);
                vb_page_wdata1 := wb_bus_wdata(31 downto 0);
            end if;
        elsif r.state = idle then
            if not wait_while_write then
                v.wready := '1';
            end if;
            v.state := wsetup;
            -- Only 4-bytes access to control registers:
            if wb_bus_waddr(0)(2) = '0' and wstrb = X"F0" then
                v.waddr := wb_bus_waddr(1)(7 downto 2);
                v.wdata := wb_bus_wdata(63 downto 32);
            else
                v.waddr := wb_bus_waddr(0)(7 downto 2);
                v.wdata := wb_bus_wdata(31 downto 0);
            end if;
        end if;
    end if;

    if r.state = txcmd or r.state = wbyte then
        vb_page_addr_mux := r.buf_addr(5 downto 1);
    else
        vb_page_addr_mux := vb_page_addr;
    end if;

    if not async_reset and nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;

    wb_page_addr <= vb_page_addr_mux;
    wb_page_wdata0 <= vb_page_wdata0;
    w_page_we0 <= v_page_we0;
    wb_page_wdata1 <= vb_page_wdata1;
    w_page_we1 <= v_page_we1;

  end process;

  cfg <= xconfig;

  o_spi.SDO <= r.so_shifter(31);
  o_spi.SCK <= r.sck;
  o_spi.nCS <= r.csn;
  o_spi.nWP <= '1';
  o_spi.nHOLD <= '1';
  o_spi.RESET <= '0';

  reg : process (nrst, clk, wb_page_addr, w_page_we0, wb_page_wdata0,
                                          w_page_we1, wb_page_wdata1) 
  begin
    if nrst = '0' then
        pagebuf0 <= (others => (others => '1'));
        pagebuf1 <= (others => (others => '1'));
    elsif rising_edge(clk) then 
      if w_page_we0 = '1' then
        pagebuf0(conv_integer(wb_page_addr)) <= wb_page_wdata0;
      end if;

      if w_page_we1 = '1' then
        pagebuf1(conv_integer(wb_page_addr)) <= wb_page_wdata1;
      end if;
    end if;
  end process;

  wb_page_rdata0 <= pagebuf0(conv_integer(wb_page_addr));
  wb_page_rdata1 <= pagebuf1(conv_integer(wb_page_addr));


  -- registers:
  regs : process(nrst, clk)
  begin 
    if async_reset and nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;