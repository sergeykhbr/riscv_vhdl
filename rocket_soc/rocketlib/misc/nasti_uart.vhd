-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      RS-232 UART with the AXI4 interface.
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

entity nasti_uart is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    fifosz  : integer := 16;
    parity_bit : integer := 1
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out  nasti_slave_config_type;
    i_uart : in  uart_in_type;
    o_uart : out uart_out_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type
  );
end; 
 
architecture arch_nasti_uart of nasti_uart is
  --! 4-bytes alignment so that all registers implemented as 32-bits
  --! width.
  constant ALIGNMENT_BYTES : integer := 4;

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_UART
  );

  type local_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1) 
       of integer;
  type fifo_type is array (0 to fifosz-1) of std_logic_vector(7 downto 0);
  type state_type is (idle, startbit, data, parity, stopbit);

  type bank_type is record
        tx_state  : state_type;
        tx_fifo   : fifo_type;
        tx_wr_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        tx_rd_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        tx_shift  : std_logic_vector(10 downto 0); --! stopbit=1,parity=xor,data[7:0],startbit=0
        tx_data_cnt : integer range 0 to 11;
        tx_scaler_cnt : integer;
        tx_level : std_logic;

        rx_state  : state_type;
        rx_fifo   : fifo_type;
        rx_wr_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        rx_rd_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        rx_shift  : std_logic_vector(7 downto 0);
        rx_data_cnt : integer range 0 to 7;
        rx_scaler_cnt : integer;
        rx_level : std_logic;

        scaler : integer;
        err_parity : std_logic;
        err_stopbit : std_logic;
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

signal r, rin : registers;

begin

  comblogic : process(i_uart, i_axi, r)
    variable v : registers;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    variable val : std_logic_vector(8*ALIGNMENT_BYTES-1 downto 0);

    variable posedge_flag : std_logic;
    variable negedge_flag : std_logic;
    variable tx_fifo_empty : std_logic;
    variable tx_fifo_full : std_logic;
    variable rx_fifo_empty : std_logic;
    variable rx_fifo_full : std_logic;
    variable t_tx, t_rx : std_logic_vector(7 downto 0);
    variable par : std_logic;
  begin

    v := r;

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);


    posedge_flag := '0';
    negedge_flag := '0';
    if r.bank0.scaler /= 0 then
        if r.bank0.tx_scaler_cnt = (r.bank0.scaler-1) then
            v.bank0.tx_scaler_cnt := 0;
            v.bank0.tx_level := not r.bank0.tx_level;
            posedge_flag := not r.bank0.tx_level;
        else
            v.bank0.tx_scaler_cnt := r.bank0.tx_scaler_cnt + 1;
        end if;

        if r.bank0.rx_state = idle and i_uart.rd = '1' then
            v.bank0.rx_scaler_cnt := 0;
            v.bank0.rx_level := '1';
        elsif r.bank0.rx_scaler_cnt = (r.bank0.scaler-1) then
            v.bank0.rx_scaler_cnt := 0;
            v.bank0.rx_level := not r.bank0.rx_level;
            negedge_flag := r.bank0.rx_level;
        else
            v.bank0.rx_scaler_cnt := r.bank0.rx_scaler_cnt + 1;
        end if;
    end if;

    -- Transmitter's FIFO:
    tx_fifo_full := '0';
    if (r.bank0.tx_wr_cnt + 1) = r.bank0.tx_rd_cnt then
        tx_fifo_full := '1';
    end if;
    tx_fifo_empty := '0';
    if r.bank0.tx_rd_cnt = r.bank0.tx_wr_cnt then
        tx_fifo_empty := '1';
    end if;

    -- Receiver's FIFO:
    rx_fifo_full := '0';
    if (r.bank0.rx_wr_cnt + 1) = r.bank0.rx_rd_cnt then
        rx_fifo_full := '1';
    end if;
    rx_fifo_empty := '0';
    if r.bank0.rx_rd_cnt = r.bank0.rx_wr_cnt then
        rx_fifo_empty := '1';
    end if;

    -- Transmitter's state machine:
    if i_uart.cts = '1' and posedge_flag = '1' then
        case r.bank0.tx_state is
        when idle =>
            if tx_fifo_empty = '0' then
                -- stopbit=1,parity=xor,data[7:0],startbit=0
                t_tx := r.bank0.tx_fifo(conv_integer(r.bank0.tx_rd_cnt));
                if parity_bit = 1 then
                    par := t_tx(7) xor t_tx(6) xor t_tx(5) xor t_tx(4)
                         xor t_tx(3) xor t_tx(2) xor t_tx(1) xor t_tx(0);
                    v.bank0.tx_shift := '1' & par & t_tx & '0';
                else
                    v.bank0.tx_shift := "11" & t_tx & '0';
                end if;
                
                v.bank0.tx_state := startbit;
                v.bank0.tx_rd_cnt := r.bank0.tx_rd_cnt + 1;
                v.bank0.tx_data_cnt := 0;
            end if;
        when startbit =>
            v.bank0.tx_state := data;
        when data =>
            if r.bank0.tx_data_cnt = 8 then
                if parity_bit = 1 then
                    v.bank0.tx_state := parity;
                else
                    v.bank0.tx_state := stopbit;
                end if;
            end if;
        when parity =>
            v.bank0.tx_state := stopbit;
        when stopbit =>
            v.bank0.tx_state := idle;
        when others =>
        end case;
        
        if r.bank0.tx_state /= idle then
            v.bank0.tx_data_cnt := r.bank0.tx_data_cnt + 1;
            v.bank0.tx_shift := '1' & r.bank0.tx_shift(10 downto 1);
        end if;
    end if;

    --! Receiver's state machine:
    if negedge_flag = '1' then
        case r.bank0.rx_state is
        when idle =>
            if i_uart.rd = '0' then
                v.bank0.rx_state := data;
                v.bank0.rx_shift := (others => '0');
                v.bank0.rx_data_cnt := 0;
            end if;
        when data =>
            v.bank0.rx_shift := i_uart.rd & r.bank0.rx_shift(7 downto 1);
            if r.bank0.rx_data_cnt = 7 then
                if parity_bit = 1 then
                    v.bank0.rx_state := parity;
                else
                    v.bank0.rx_state := stopbit;
                end if;
            else
                v.bank0.rx_data_cnt := r.bank0.rx_data_cnt + 1;
            end if;
        when parity =>
            t_rx := r.bank0.rx_shift;
            par := t_rx(7) xor t_rx(6) xor t_rx(5) xor t_rx(4)
               xor t_rx(3) xor t_rx(2) xor t_rx(1) xor t_rx(0);
            if par = i_uart.rd then
                v.bank0.err_parity := '0';
            else 
                v.bank0.err_parity := '1';
            end if;
            v.bank0.rx_state := stopbit;
        when stopbit =>
            if i_uart.rd = '0' then
                v.bank0.err_stopbit := '1';
            else
                v.bank0.err_stopbit := '0';
            end if;
            if rx_fifo_full = '0' then
                v.bank0.rx_fifo(conv_integer(r.bank0.rx_wr_cnt)) := r.bank0.rx_shift;
                v.bank0.rx_wr_cnt := r.bank0.rx_wr_cnt + 1;
            end if;
            v.bank0.rx_state := idle;
        when others =>
        end case;
    end if;


    o_uart.rts <= '1';
    if r.bank0.tx_state = idle then
        o_uart.td <= '1';
    else
        o_uart.td <= r.bank0.tx_shift(0);
    end if;


    for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
       raddr_reg(n) := conv_integer(r.bank_axi.raddr(ALIGNMENT_BYTES*n)(11 downto 2));

       val := (others => '0');
       case raddr_reg(n) is
          when 0 => 
                if rx_fifo_empty = '0' then
                    val(7 downto 0) := r.bank0.rx_fifo(conv_integer(r.bank0.rx_rd_cnt)); 
                    v.bank0.rx_rd_cnt := r.bank0.rx_rd_cnt + 1;
                end if;
          when 1 => 
                val(1 downto 0) := tx_fifo_empty & tx_fifo_full;
                val(5 downto 4) := rx_fifo_empty & rx_fifo_full;
                val(9 downto 8) := r.bank0.err_stopbit & r.bank0.err_parity;
          when 2 => 
                val := conv_std_logic_vector(r.bank0.scaler,32);
          when others => 
                val := X"badef00d";
       end case;
       rdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n) := val;
    end loop;


    if i_axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      wdata := i_axi.w_data;
      wstrb := i_axi.w_strb;
      for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(ALIGNMENT_BYTES*n)(11 downto 2));

         if conv_integer(wstrb(ALIGNMENT_BYTES*(n+1)-1 downto ALIGNMENT_BYTES*n)) /= 0 then
           val := wdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n);
           case waddr_reg(n) is
             when 0 => 
                    if tx_fifo_full = '0' then
                        v.bank0.tx_fifo(conv_integer(r.bank0.tx_wr_cnt)) := val(7 downto 0);
                        v.bank0.tx_wr_cnt := r.bank0.tx_wr_cnt + 1;
                    end if;
             when 2 => 
                    v.bank0.scaler     := conv_integer(val);
                    v.bank0.rx_scaler_cnt := 0;
                    v.bank0.tx_scaler_cnt := 0;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    o_axi <= functionAxi4Output(r.bank_axi, rdata);
    rin <= v;
  end process;

  cfg <= xconfig;

  -- registers:
  regs : process(clk, nrst)
  begin 
     if nrst = '0' then
        r.bank_axi <= NASTI_SLAVE_BANK_RESET;
        r.bank0.tx_state <= idle;
        r.bank0.tx_level <= '0';
        r.bank0.tx_scaler_cnt <= 0;
        r.bank0.tx_rd_cnt <= (others => '0');
        r.bank0.tx_wr_cnt <= (others => '0');

        r.bank0.rx_state <= idle;
        r.bank0.rx_level <= '1';
        r.bank0.rx_scaler_cnt <= 0;
        r.bank0.rx_rd_cnt <= (others => '0');
        r.bank0.rx_wr_cnt <= (others => '0');

        r.bank0.scaler <= 0;
        r.bank0.err_parity <= '0';
        r.bank0.err_stopbit <= '0';
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;