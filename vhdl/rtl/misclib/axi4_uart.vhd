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

entity axi4_uart is
  generic (
    async_reset : boolean := false;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq    : integer := 0;
    fifosz  : integer := 16
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out axi4_slave_config_type;
    i_uart : in  uart_in_type;
    o_uart : out uart_out_type;
    i_axi  : in  axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type;
    o_irq  : out std_logic
  );
end; 
 
architecture arch_axi4_uart of axi4_uart is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(xirq, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_UART
  );

  constant zero32 : std_logic_vector(31 downto 0) := (others => '0');

  type fifo_mem is array (0 to fifosz-1) of std_logic_vector(7 downto 0);
  type state_type is (idle, startbit, data, parity, stopbit);

  type fifo_in_type is record
    raddr : integer range 0 to fifosz-1;
    waddr : integer range 0 to fifosz-1;
    we : std_logic;
    wdata : std_logic_vector(7 downto 0);
  end record;

  constant fifo_in_none : fifo_in_type := (0, 0, '0', X"00");

  signal rfifoi : fifo_in_type;
  signal rx_fifo_rdata : std_logic_vector(7 downto 0);
  signal rx_fifo : fifo_mem;

  signal tfifoi : fifo_in_type;
  signal tx_fifo_rdata : std_logic_vector(7 downto 0);
  signal tx_fifo   : fifo_mem;

  type registers is record
      tx_state  : state_type;
      tx_wr_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
      tx_rd_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
      tx_byte_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
      tx_shift  : std_logic_vector(10 downto 0); --! stopbit=1,parity=xor,data[7:0],startbit=0
      tx_data_cnt : integer range 0 to 11;
      tx_scaler_cnt : std_logic_vector(31 downto 0);
      tx_level : std_logic;
      tx_irq_thresh : std_logic_vector(log2(fifosz)-1 downto 0);
      tx_more_thresh : std_logic_vector(1 downto 0);

      rx_state  : state_type;
      rx_wr_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
      rx_rd_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
      rx_byte_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
      rx_shift  : std_logic_vector(7 downto 0);
      rx_data_cnt : integer range 0 to 7;
      rx_scaler_cnt : std_logic_vector(31 downto 0);
      rx_level : std_logic;
      rx_irq_thresh : std_logic_vector(log2(fifosz)-1 downto 0);
      rx_more_thresh : std_logic_vector(1 downto 0);

      rdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
      scaler : std_logic_vector(31 downto 0);
      err_parity : std_logic;
      err_stopbit : std_logic;
      parity_bit : std_logic;
      tx_irq_ena : std_logic;
      rx_irq_ena : std_logic;
      fwcpuid : std_logic_vector(31 downto 0);
  end record;

  constant R_RESET : registers := (
        idle,  -- tx_state
        (others => '0'), (others => '0'),  -- tx_wr_cnt, tx_rd_cnt
        (others => '0'),  -- tx_byte_cnt
        (others => '0'),  -- tx_shift
        0, -- tx_data_cnt
        (others => '0'), -- tx_scaler_cnt
        '0', -- tx_level
        (others => '0'), -- tx_irq_thresh
        (others => '0'), -- tx_more_thresh
        idle, -- rx_state
        (others => '0'), (others => '0'), -- rx_wr_cnt , rx_rd_cnt
        (others => '0'), -- rx_byte_cnt
        (others => '0'), -- rx_shift
        0, -- rx_data_cnt
        (others => '0'), -- rx_scaler_cnt
        '1', -- rx_level
        (others => '0'), -- rx_irq_thresh
        (others => '0'), -- rx_more_thresh
        (others => '0'), -- rdata
        (others => '0'), -- scaler
        '0', -- err_parity
        '0', -- err_stopbit
        '0', -- parity_bit
        '0', -- tx_irq_ena
        '0', -- rx_irq_ena
        (others => '0')); -- fwcpuid

signal r, rin : registers;

signal wb_bus_raddr : global_addr_array_type;
signal w_bus_re    : std_logic;
signal wb_bus_waddr : global_addr_array_type;
signal w_bus_we    : std_logic;
signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);


begin

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i_axi,
    o_xslvo => o_axi,
    i_ready => '1',
    i_rdata => r.rdata,
    o_re => w_bus_re,
    o_r32 => open,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );

  comblogic : process(nrst, i_uart, r, tx_fifo_rdata, rx_fifo_rdata,
                      w_bus_re, wb_bus_raddr, wb_bus_waddr, w_bus_we,
                      wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable tmp : std_logic_vector(31 downto 0);

    variable v_rfifoi : fifo_in_type;
    variable v_tfifoi : fifo_in_type;
    variable posedge_flag : std_logic;
    variable negedge_flag : std_logic;
    variable tx_fifo_empty : std_logic;
    variable tx_fifo_full : std_logic;
    variable rx_fifo_empty : std_logic;
    variable rx_fifo_full : std_logic;
    variable t_tx, t_rx : std_logic_vector(7 downto 0);
    variable par : std_logic;
    variable irq_ena : std_logic;
  begin

    v := r;

    v_rfifoi := fifo_in_none;
    v_rfifoi.raddr := conv_integer(r.rx_rd_cnt);
    v_rfifoi.waddr := conv_integer(r.rx_wr_cnt);
    v_rfifoi.wdata := r.rx_shift;

    v_tfifoi := fifo_in_none;
    v_tfifoi.raddr := conv_integer(r.tx_rd_cnt);
    v_tfifoi.waddr := conv_integer(r.tx_wr_cnt);

    -- Check FIFOs counters with thresholds:
    v.tx_more_thresh := r.tx_more_thresh(0) & '0';
    if r.tx_byte_cnt > r.tx_irq_thresh then
      v.tx_more_thresh(0) := '1';
    end if;

    v.rx_more_thresh := r.rx_more_thresh(0) & '0';
    if r.rx_byte_cnt > r.rx_irq_thresh then
      v.rx_more_thresh(0) := '1';
    end if;
    
    irq_ena := '0';
    if (r.tx_more_thresh(1) and not r.tx_more_thresh(0)) = '1' then
       irq_ena := r.tx_irq_ena;
    end if;
    if (not r.rx_more_thresh(1) and r.rx_more_thresh(0)) = '1' then
       irq_ena := irq_ena or r.rx_irq_ena;
    end if;
    
    -- system bus clock scaler to baudrate:
    posedge_flag := '0';
    negedge_flag := '0';
    if r.scaler /= zero32 then
        if r.tx_scaler_cnt = (r.scaler-1) then
            v.tx_scaler_cnt := zero32;
            v.tx_level := not r.tx_level;
            posedge_flag := not r.tx_level;
        else
            v.tx_scaler_cnt := r.tx_scaler_cnt + 1;
        end if;

        if r.rx_state = idle and i_uart.rd = '1' then
            v.rx_scaler_cnt := zero32;
            v.rx_level := '1';
        elsif r.rx_scaler_cnt = (r.scaler-1) then
            v.rx_scaler_cnt := zero32;
            v.rx_level := not r.rx_level;
            negedge_flag := r.rx_level;
        else
            v.rx_scaler_cnt := r.rx_scaler_cnt + 1;
        end if;
    end if;

    -- Transmitter's FIFO:
    tx_fifo_full := '0';
    if (r.tx_wr_cnt + 1) = r.tx_rd_cnt then
        tx_fifo_full := '1';
    end if;
    tx_fifo_empty := '0';
    if r.tx_rd_cnt = r.tx_wr_cnt then
        tx_fifo_empty := '1';
        v.tx_byte_cnt := (others => '0');
    end if;

    -- Receiver's FIFO:
    rx_fifo_full := '0';
    if (r.rx_wr_cnt + 1) = r.rx_rd_cnt then
        rx_fifo_full := '1';
    end if;
    rx_fifo_empty := '0';
    if r.rx_rd_cnt = r.rx_wr_cnt then
        rx_fifo_empty := '1';
        v.rx_byte_cnt := (others => '0');
    end if;

    -- Transmitter's state machine:
    if i_uart.cts = '1' and posedge_flag = '1' then
        case r.tx_state is
        when idle =>
            if tx_fifo_empty = '0' then
                -- stopbit=1,parity=xor,data[7:0],startbit=0
                t_tx := tx_fifo_rdata; --r.tx_fifo(conv_integer(r.tx_rd_cnt));
                if r.parity_bit = '1' then
                    par := t_tx(7) xor t_tx(6) xor t_tx(5) xor t_tx(4)
                         xor t_tx(3) xor t_tx(2) xor t_tx(1) xor t_tx(0);
                    v.tx_shift := '1' & par & t_tx & '0';
                else
                    v.tx_shift := "11" & t_tx & '0';
                end if;
                
                v.tx_state := startbit;
                v.tx_rd_cnt := r.tx_rd_cnt + 1;
                v.tx_byte_cnt := r.tx_byte_cnt - 1;
                v.tx_data_cnt := 0;
            end if;
        when startbit =>
            v.tx_state := data;
        when data =>
            if r.tx_data_cnt = 8 then
                if r.parity_bit = '1' then
                    v.tx_state := parity;
                else
                    v.tx_state := stopbit;
                end if;
            end if;
        when parity =>
            v.tx_state := stopbit;
        when stopbit =>
            v.tx_state := idle;
        when others =>
        end case;
        
        if r.tx_state /= idle then
            v.tx_data_cnt := r.tx_data_cnt + 1;
            v.tx_shift := '1' & r.tx_shift(10 downto 1);
        end if;
    end if;

    --! Receiver's state machine:
    if negedge_flag = '1' then
        case r.rx_state is
        when idle =>
            if i_uart.rd = '0' then
                v.rx_state := data;
                v.rx_shift := (others => '0');
                v.rx_data_cnt := 0;
            end if;
        when data =>
            v.rx_shift := i_uart.rd & r.rx_shift(7 downto 1);
            if r.rx_data_cnt = 7 then
                if r.parity_bit = '1' then
                    v.rx_state := parity;
                else
                    v.rx_state := stopbit;
                end if;
            else
                v.rx_data_cnt := r.rx_data_cnt + 1;
            end if;
        when parity =>
            t_rx := r.rx_shift;
            par := t_rx(7) xor t_rx(6) xor t_rx(5) xor t_rx(4)
               xor t_rx(3) xor t_rx(2) xor t_rx(1) xor t_rx(0);
            if par = i_uart.rd then
                v.err_parity := '0';
            else 
                v.err_parity := '1';
            end if;
            v.rx_state := stopbit;
        when stopbit =>
            if i_uart.rd = '0' then
                v.err_stopbit := '1';
            else
                v.err_stopbit := '0';
            end if;
            if rx_fifo_full = '0' then
                v_rfifoi.we := '1';
                --v.rx_fifo(conv_integer(r.rx_wr_cnt)) := r.rx_shift;
                v.rx_wr_cnt := r.rx_wr_cnt + 1;
                v.rx_byte_cnt := r.rx_byte_cnt + 1;
            end if;
            v.rx_state := idle;
        when others =>
        end case;
    end if;


    o_uart.rts <= '1';
    if r.tx_state = idle then
        o_uart.td <= '1';
    else
        o_uart.td <= r.tx_shift(0);
    end if;


    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       tmp := (others => '0');
       case conv_integer(wb_bus_raddr(n)(11 downto 2)) is
          when 0 => 
                tmp(1 downto 0) := tx_fifo_empty & tx_fifo_full;
                tmp(5 downto 4) := rx_fifo_empty & rx_fifo_full;
                tmp(9 downto 8) := r.err_stopbit & r.err_parity;
                tmp(13) := r.rx_irq_ena;
                tmp(14) := r.tx_irq_ena;
                tmp(15) := r.parity_bit;
          when 1 => 
                tmp := r.scaler;
          when 2 => 
                tmp := r.fwcpuid;
          when 4 => 
                if rx_fifo_empty = '0' and w_bus_re = '1' then
                    tmp(7 downto 0) := rx_fifo_rdata; 
                    v.rx_rd_cnt := r.rx_rd_cnt + 1;
                    v.rx_byte_cnt := r.rx_byte_cnt - 1;
                end if;
          when others => 
       end case;
       v.rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if w_bus_we = '1' then
      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         if conv_integer(wb_bus_wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           tmp := wb_bus_wdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n);
           case conv_integer(wb_bus_waddr(n)(11 downto 2)) is
             when 0 =>
                    v.parity_bit := tmp(15);
                    v.tx_irq_ena := tmp(14);
                    v.rx_irq_ena := tmp(13);
             when 1 => 
                    v.scaler     := tmp;
                    v.rx_scaler_cnt := zero32;
                    v.tx_scaler_cnt := zero32;
             when 2 => 
                    if r.fwcpuid = X"00000000" or tmp = X"00000000" then
                        v.fwcpuid := tmp;
                    end if;
             when 4 => 
                    if tx_fifo_full = '0' then
                        v_tfifoi.we := '1';
                        v_tfifoi.wdata := tmp(7 downto 0);
                        v.tx_wr_cnt := r.tx_wr_cnt + 1;
                        v.tx_byte_cnt := r.tx_byte_cnt + 1;
                    end if;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    if not async_reset and nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;

    rfifoi <= v_rfifoi;
    tfifoi <= v_tfifoi;

    o_irq <= irq_ena;
  end process;

  cfg <= xconfig;

  -- fifo pseudo memory:
  tfifo0 : process(clk, tfifoi, tx_fifo)
  begin 
     if rising_edge(clk) then 
        if tfifoi.we = '1' then
            tx_fifo(tfifoi.waddr) <= tfifoi.wdata;
        end if;
     end if; 
     tx_fifo_rdata <= tx_fifo(tfifoi.raddr);
  end process;

  rfifo0 : process(clk, rfifoi, rx_fifo)
  begin 
     if rising_edge(clk) then 
        if rfifoi.we = '1' then
            rx_fifo(rfifoi.waddr) <= rfifoi.wdata;
        end if;
     end if; 
     rx_fifo_rdata <= rx_fifo(rfifoi.raddr);
  end process;

  -- registers:
  regs : process(clk, nrst)
  begin 
     if async_reset and nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;