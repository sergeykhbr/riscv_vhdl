-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      RS-232 UART with the AXI4 interface.
------------------------------------------------------------------------------

--! @defgroup uart_page UART
--! @ingroup peripheries_group
--! 
--! @section uart_overview Overview
--!
--! This UART implementation can be additionally configured using the following
--! generic parameters.
--!
--! | Name | Default   | Description
--! |:-----|:---------:|:------------
--! |irqx  | 0         | <b>Interrupt pin index</b> This value is used only as argument in output Plug'n'Play configuration.
--! |fifosz| 16        | <b>FIFO size.</b> Size of the Tx and Rx FIFOs in bytes.
--! 
--! @section uart_regs UART registers mapping
--! UART acts like a slave AMBA AXI4 device that is directly mapped
--! into physical memory. Default address location for our implementation 
--! is defined by 0x80001000. Memory size is 4 KB.
--!
--! @par Control Status register (0x000).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 16   | RW | 16h'0 | Reserved      | 31:16 | Reserved.
--! | 1    | RW | 1b'0  | parity_bit    | 15 | <b>Enable parity checking</b>. Serial port setting setup by SW.
--! | 1    | RW | 1b'0  | tx_irq_ena    | 14 | <b>Enable Tx Interrupt</b>. Generate interrupt when number of symbol in output FIFO less than defined in Tx Threshold register.
--! | 1    | RW | 1b'0  | rx_irq_ena    | 13 | <b>Enable Rx Interrupt</b>. Generate interrupt when number of available for reading symbol greater or equalt Rx Threshold register.
--! | 3    | RW | 3h'0  | Reserved      | 12:10 | Reserved.
--! | 1    | RO | 1b'0  | err_stopbit   | 9 | <b>Stop Bit Error</b>. This bit is set when the Stoping Bit has the wrnog value.
--! | 1    | RO | 1b'0  | err_parity    | 8 | <b>Parity Error</b>. This bit is set when the Parity error occurs. Will be automatically cleared by next received symbol if the parity OK.
--! | 2    | RW | 2h'0  | Reserved      | 7:6 | Reserved.
--! | 1    | RO | 1b'1  | rx_fifo_empty | 5 | <b>Receive FIFO is Empty</b>.
--! | 1    | RO | 1b'0  | rx_fifo_fifo  | 4 | <b>Receive FIFO is Full</b>.
--! | 2    | RW | 2h'0  | Reserved      | 3:2 | Reserved.
--! | 1    | RO | 1b'1  | tx_fifo_empty | 1 | <b>Transmit FIFO is Empty</b>.
--! | 1    | RO | 1'b0  | tx_fifo_full  | 0 | <b>Transmit FIFO is Full</b>. 
--!
--! @par Scaler register (0x004).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 32   | RW | 32h'0 | scaler | 31:16 | <b>Scale threshold</b>. This register value is used to transform System Bus clock into port baudrate.
--!
--! @par Data register (0x010).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 24   | RW | 28h'0 | Reserved | 31:8 | Reserved.
--! | 8    | RW | 8h'0  | data     | 7:0  | <b>Data</b>. Access to Tx/Rx FIFO data. Writing into this register put data into Tx FIFO. Reading is accomplished from Rx FIFO.

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

entity nasti_uart is
  generic (
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq    : integer := 0;
    fifosz  : integer := 16
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out  nasti_slave_config_type;
    i_uart : in  uart_in_type;
    o_uart : out uart_out_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_irq  : out std_logic
  );
end; 
 
architecture arch_nasti_uart of nasti_uart is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => xirq,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_UART
  );

  type fifo_type is array (0 to fifosz-1) of std_logic_vector(7 downto 0);
  type state_type is (idle, startbit, data, parity, stopbit);

  type bank_type is record
        tx_state  : state_type;
        tx_fifo   : fifo_type;
        tx_wr_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        tx_rd_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        tx_byte_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        tx_shift  : std_logic_vector(10 downto 0); --! stopbit=1,parity=xor,data[7:0],startbit=0
        tx_data_cnt : integer range 0 to 11;
        tx_scaler_cnt : integer;
        tx_level : std_logic;
        tx_irq_thresh : std_logic_vector(log2(fifosz)-1 downto 0);
        tx_more_thresh : std_logic_vector(1 downto 0);

        rx_state  : state_type;
        rx_fifo   : fifo_type;
        rx_wr_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        rx_rd_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        rx_byte_cnt : std_logic_vector(log2(fifosz)-1 downto 0);
        rx_shift  : std_logic_vector(7 downto 0);
        rx_data_cnt : integer range 0 to 7;
        rx_scaler_cnt : integer;
        rx_level : std_logic;
        rx_irq_thresh : std_logic_vector(log2(fifosz)-1 downto 0);
        rx_more_thresh : std_logic_vector(1 downto 0);

        scaler : integer;
        err_parity : std_logic;
        err_stopbit : std_logic;
        parity_bit : std_logic;
        tx_irq_ena : std_logic;
        rx_irq_ena : std_logic;
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

signal r, rin : registers;

begin

  comblogic : process(nrst, i_uart, i_axi, r)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);

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

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);

    -- Check FIFOs counters with thresholds:
    v.bank0.tx_more_thresh := r.bank0.tx_more_thresh(0) & '0';
    if r.bank0.tx_byte_cnt > r.bank0.tx_irq_thresh then
      v.bank0.tx_more_thresh(0) := '1';
    end if;

    v.bank0.rx_more_thresh := r.bank0.rx_more_thresh(0) & '0';
    if r.bank0.rx_byte_cnt > r.bank0.rx_irq_thresh then
      v.bank0.rx_more_thresh(0) := '1';
    end if;
    
    irq_ena := '0';
    if (r.bank0.tx_more_thresh(1) and not r.bank0.tx_more_thresh(0)) = '1' then
       irq_ena := r.bank0.tx_irq_ena;
    end if;
    if (not r.bank0.rx_more_thresh(1) and r.bank0.rx_more_thresh(0)) = '1' then
       irq_ena := irq_ena or r.bank0.rx_irq_ena;
    end if;
    
    -- system bus clock scaler to baudrate:
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
        v.bank0.tx_byte_cnt := (others => '0');
    end if;

    -- Receiver's FIFO:
    rx_fifo_full := '0';
    if (r.bank0.rx_wr_cnt + 1) = r.bank0.rx_rd_cnt then
        rx_fifo_full := '1';
    end if;
    rx_fifo_empty := '0';
    if r.bank0.rx_rd_cnt = r.bank0.rx_wr_cnt then
        rx_fifo_empty := '1';
        v.bank0.rx_byte_cnt := (others => '0');
    end if;

    -- Transmitter's state machine:
    if i_uart.cts = '1' and posedge_flag = '1' then
        case r.bank0.tx_state is
        when idle =>
            if tx_fifo_empty = '0' then
                -- stopbit=1,parity=xor,data[7:0],startbit=0
                t_tx := r.bank0.tx_fifo(conv_integer(r.bank0.tx_rd_cnt));
                if r.bank0.parity_bit = '1' then
                    par := t_tx(7) xor t_tx(6) xor t_tx(5) xor t_tx(4)
                         xor t_tx(3) xor t_tx(2) xor t_tx(1) xor t_tx(0);
                    v.bank0.tx_shift := '1' & par & t_tx & '0';
                else
                    v.bank0.tx_shift := "11" & t_tx & '0';
                end if;
                
                v.bank0.tx_state := startbit;
                v.bank0.tx_rd_cnt := r.bank0.tx_rd_cnt + 1;
                v.bank0.tx_byte_cnt := r.bank0.tx_byte_cnt - 1;
                v.bank0.tx_data_cnt := 0;
            end if;
        when startbit =>
            v.bank0.tx_state := data;
        when data =>
            if r.bank0.tx_data_cnt = 8 then
                if r.bank0.parity_bit = '1' then
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
                if r.bank0.parity_bit = '1' then
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
                v.bank0.rx_byte_cnt := r.bank0.rx_byte_cnt + 1;
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


    for n in 0 to CFG_WORDS_ON_BUS-1 loop

       tmp := (others => '0');
       case conv_integer(r.bank_axi.raddr(n)(11 downto 2)) is
          when 0 => 
                tmp(1 downto 0) := tx_fifo_empty & tx_fifo_full;
                tmp(5 downto 4) := rx_fifo_empty & rx_fifo_full;
                tmp(9 downto 8) := r.bank0.err_stopbit & r.bank0.err_parity;
                tmp(13) := r.bank0.rx_irq_ena;
                tmp(14) := r.bank0.tx_irq_ena;
                tmp(15) := r.bank0.parity_bit;
          when 1 => 
                tmp := conv_std_logic_vector(r.bank0.scaler,32);
          when 4 => 
                if rx_fifo_empty = '0' and r.bank_axi.rstate = rtrans then
                    tmp(7 downto 0) := r.bank0.rx_fifo(conv_integer(r.bank0.rx_rd_cnt)); 
                    v.bank0.rx_rd_cnt := r.bank0.rx_rd_cnt + 1;
                    v.bank0.rx_byte_cnt := r.bank0.rx_byte_cnt - 1;
                end if;
          when others => 
       end case;
       rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if i_axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      wstrb := i_axi.w_strb;
      for n in 0 to CFG_WORDS_ON_BUS-1 loop

         if conv_integer(wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           tmp := i_axi.w_data(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n);
           case conv_integer(r.bank_axi.waddr(n)(11 downto 2)) is
             when 0 =>
                    v.bank0.parity_bit := tmp(15);
                    v.bank0.tx_irq_ena := tmp(14);
                    v.bank0.rx_irq_ena := tmp(13);
             when 1 => 
                    v.bank0.scaler     := conv_integer(tmp);
                    v.bank0.rx_scaler_cnt := 0;
                    v.bank0.tx_scaler_cnt := 0;
             when 4 => 
                    if tx_fifo_full = '0' then
                        v.bank0.tx_fifo(conv_integer(r.bank0.tx_wr_cnt)) := tmp(7 downto 0);
                        v.bank0.tx_wr_cnt := r.bank0.tx_wr_cnt + 1;
                        v.bank0.tx_byte_cnt := r.bank0.tx_byte_cnt + 1;
                    end if;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    if nrst = '0' then
        v.bank_axi := NASTI_SLAVE_BANK_RESET;
        v.bank0.tx_state := idle;
        v.bank0.tx_level := '0';
        v.bank0.tx_scaler_cnt := 0;
        v.bank0.tx_rd_cnt := (others => '0');
        v.bank0.tx_wr_cnt := (others => '0');
        v.bank0.tx_byte_cnt := (others => '0');
        v.bank0.tx_irq_thresh := (others => '0');
        v.bank0.tx_more_thresh := (others => '0');

        v.bank0.rx_state := idle;
        v.bank0.rx_level := '1';
        v.bank0.rx_scaler_cnt := 0;
        v.bank0.rx_rd_cnt := (others => '0');
        v.bank0.rx_wr_cnt := (others => '0');
        v.bank0.rx_byte_cnt := (others => '0');
        v.bank0.rx_irq_thresh := (others => '0');
        v.bank0.rx_more_thresh := (others => '0');

        v.bank0.scaler := 0;
        v.bank0.err_parity := '0';
        v.bank0.err_stopbit := '0';
        v.bank0.parity_bit := '0';
        v.bank0.tx_irq_ena := '1';
        v.bank0.rx_irq_ena := '1';
    end if;

    o_axi <= functionAxi4Output(r.bank_axi, rdata);
    o_irq <= irq_ena;
    rin <= v;
  end process;

  cfg <= xconfig;

  -- registers:
  regs : process(clk)
  begin 
     if rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;