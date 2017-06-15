-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      TAP via UART.
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

entity uart_tap is
  port (
    nrst     : in std_logic;
    clk      : in std_logic;
    i_uart   : in  uart_in_type;
    o_uart   : out uart_out_type;
    i_msti   : in nasti_master_in_type;
    o_msto   : out nasti_master_out_type;
    o_mstcfg : out nasti_master_config_type
  );
end; 
 
architecture arch_uart_tap of uart_tap is

  constant xmstconfig : nasti_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_UART_TAP
  );

  type uart_state_type is (idle, startbit, data, stopbit);
  type dma_req_state_type is (
      DMAREQ_IDLE,
      DMAREQ_ADDR,
      DMAREQ_READ,
      DMAREQ_WAIT_READ_RESP,
      DMAREQ_WDATA,
      DMAREQ_WRITE
   );

  type registers is record
    dma : dma_bank_type;
    rd_z : std_logic;
    baud_speed_detected : std_logic;
    baud_speed_confirmed : std_logic;
    baud_speed_cnt : std_logic_vector(32 downto 0);
    baud_speed_cnt_z : std_logic_vector(31 downto 0);
    scaler : std_logic_vector(31 downto 0);
    scale_cnt : std_logic_vector(31 downto 0);
    level : std_logic;

    tx_word   : std_logic_vector(31 downto 0);
    tx_shift  : std_logic_vector(10 downto 0); --! stopbit=1,parity=xor,data[7:0],startbit=0
    tx_data_cnt : integer range 0 to 11;
    tx_byte_cnt : std_logic_vector(3 downto 0); -- always 32-bits word output
    tx_state : uart_state_type;

    rx_shift  : std_logic_vector(7 downto 0);
    rx_data_cnt : integer range 0 to 7;
    rx_state : uart_state_type;
    rx_byte_ready : std_logic;
    rx_byte  : std_logic_vector(7 downto 0);

    dma_req_state : dma_req_state_type;
    dma_req_write : std_logic;
    dma_byte_cnt : integer range 0 to 7;
    dma_req_len : integer range 0 to 63;
    dma_req_addr : std_logic_vector(63 downto 0);
    dma_req_wdata : std_logic_vector(31 downto 0);
    rword_valid : std_logic;
    rword : std_logic_vector(31 downto 0);
  end record;

  signal r, rin : registers;
  signal dma_response : dma_response_type;


begin

  comblogic : process(nrst, i_msti, i_uart, r, dma_response)
    variable v : registers;
    variable wb_dma_request : dma_request_type;
    variable wb_dma_response : dma_response_type;
    variable wb_msto : nasti_master_out_type;

    variable posedge_flag : std_logic;
    variable negedge_flag : std_logic;
  begin

    v := r;
    v.rx_byte_ready := '0';
    wb_dma_request.valid := '0';
    wb_dma_request.ready := '0';
    wb_dma_request.write := '0';
    wb_dma_request.addr := (others => '0');
    wb_dma_request.size := "010"; -- 4 bytes
    wb_dma_request.bytes := (others => '0');
    wb_dma_request.wdata := (others => '0');

    --! Baud rate detector and scaler
    posedge_flag := '0';
    negedge_flag := '0';
    v.rd_z := i_uart.rd;
    if r.baud_speed_detected = '0' then
        v.baud_speed_cnt := r.baud_speed_cnt + 1;
        if r.rd_z /= i_uart.rd then
            negedge_flag := '1';
            v.baud_speed_cnt := (others => '0');
            v.baud_speed_cnt_z := r.baud_speed_cnt(32 downto 1);
        end if;

        if r.rx_byte_ready = '1' and r.rx_byte = X"55" then
            v.baud_speed_detected := '1';
            v.scaler := r.baud_speed_cnt_z;
            v.scale_cnt := (others => '0');
            v.level := '1';
        end if;
    else
        v.scale_cnt := r.scale_cnt + 1;
        if r.rd_z /= i_uart.rd then
            negedge_flag := '1';
            v.level := '1';
            v.scale_cnt := (others => '0');
        elsif r.scale_cnt = r.scaler then
            posedge_flag := r.level;
            negedge_flag := not r.level;
            v.level := not r.level;
            v.scale_cnt := (others => '0');
        end if;
        -- Check baud speed detector correctness
        if r.baud_speed_confirmed ='0' and r.rx_byte_ready = '1' then
            if r.rx_byte = X"55"  then
                v.baud_speed_confirmed := '1';
            else
                v.baud_speed_detected := '0';
                v.scaler := (others => '0');
                v.scale_cnt := (others => '0');
                v.baud_speed_cnt := (others => '0');
                v.level := '0';
            end if;
        end if;
    end if;


    if r.tx_state = idle and r.tx_byte_cnt = "0000" then
        if r.rword_valid = '1' then
            v.rword_valid := '0';
            v.tx_word := r.rword;
            v.tx_byte_cnt := "0001";
        end if;
    end if;

    -- Tx state machine:
    if i_uart.cts = '1' and posedge_flag = '1' then
        case r.tx_state is
        when idle =>
            if r.tx_byte_cnt /= "0000" then
                -- stopbit=1,parity=1,data[7:0],startbit=0
                v.tx_shift := "11" & r.tx_word(7 downto 0) & '0';
                v.tx_word := X"00" & r.tx_word(31 downto 8);
                v.tx_state := startbit;
                v.tx_byte_cnt := r.tx_byte_cnt(2 downto 0) & '0';
                v.tx_data_cnt := 0;
            end if;
        when startbit =>
            v.tx_state := data;
        when data =>
            if r.tx_data_cnt = 8 then
                v.tx_state := stopbit;
            end if;
        when stopbit =>
            v.tx_state := idle;
        when others =>
        end case;
        
        if r.tx_state /= idle then
            v.tx_data_cnt := r.tx_data_cnt + 1;
            v.tx_shift := '1' & r.tx_shift(10 downto 1);
        end if;
    end if;

    --! Rx state machine:
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
                v.rx_state := stopbit;
            else
                v.rx_data_cnt := r.rx_data_cnt + 1;
            end if;
        when stopbit =>
            --if i_uart.rd = '0' then
            --    v.bank0.err_stopbit := '1';
            --else
            --    v.bank0.err_stopbit := '0';
            --end if;
            v.rx_byte_ready := '1';
            v.rx_byte := r.rx_shift(7 downto 0);
            v.rx_state := idle;
        when others =>
        end case;
    end if;

    --! DMA control
    if r.baud_speed_confirmed = '1' then
        case r.dma_req_state is
        when DMAREQ_IDLE =>
            if r.rx_byte_ready = '1' and r.rx_byte(7) = '1' then
                v.dma_req_write := r.rx_byte(6);
                v.dma_req_len := conv_integer(r.rx_byte(5 downto 0));
                v.dma_req_state := DMAREQ_ADDR;
                v.dma_byte_cnt := 0;
            end if;
        when DMAREQ_ADDR =>
            if r.rx_byte_ready = '1' then
                v.dma_req_addr := r.rx_byte & r.dma_req_addr(63 downto 8);
                if r.dma_byte_cnt = 7 then
                    if r.dma_req_write = '1' then
                        v.dma_req_state := DMAREQ_WDATA;
                        v.dma_byte_cnt := 0;
                    else
                        v.dma_req_state := DMAREQ_READ;
                    end if;
                else
                    v.dma_byte_cnt := r.dma_byte_cnt + 1;
                end if;
            end if;
        when DMAREQ_READ =>
            if r.rword_valid = '0' and r.tx_state = idle and r.tx_byte_cnt = "0000" then
                wb_dma_request.valid := '1';
            end if;
            wb_dma_request.write := '0';
            wb_dma_request.addr := r.dma_req_addr(CFG_NASTI_ADDR_BITS-1 downto 0);
            wb_dma_request.bytes := conv_std_logic_vector(4, 11);
            wb_dma_request.wdata := (others => '0');
            if dma_response.ready = '1' then
                v.dma_req_state := DMAREQ_WAIT_READ_RESP;
            end if;
        when DMAREQ_WAIT_READ_RESP =>
            wb_dma_request.ready := '1';
            if dma_response.valid = '1' then
                v.rword := dma_response.rdata(31 downto 0);
                v.rword_valid := '1';
                if r.dma_req_len = 0 then
                    v.dma_req_state := DMAREQ_IDLE;
                else
                    v.dma_req_len := r.dma_req_len - 1;
                    v.dma_req_addr := r.dma_req_addr + 4;
                    v.dma_req_state := DMAREQ_READ;
                end if;
            end if;
        when DMAREQ_WDATA =>
            if r.rx_byte_ready = '1' then
                v.dma_req_wdata := r.rx_byte & r.dma_req_wdata(31 downto 8);
                v.dma_byte_cnt := r.dma_byte_cnt + 1;
                if r.dma_byte_cnt = 3 then
                   v.dma_req_state := DMAREQ_WRITE;
                end if;
            end if;
        when DMAREQ_WRITE =>
            wb_dma_request.valid := '1';
            wb_dma_request.write := '1';
            wb_dma_request.addr := r.dma_req_addr(CFG_NASTI_ADDR_BITS-1 downto 0);
            wb_dma_request.bytes := conv_std_logic_vector(4, 11);
            wb_dma_request.wdata := r.dma_req_wdata & r.dma_req_wdata;
            if dma_response.ready = '1' then
                if r.dma_req_len = 0 then
                    v.dma_req_state := DMAREQ_IDLE;
                else
                    v.dma_byte_cnt := 0;
                    v.dma_req_len := r.dma_req_len - 1;
                    v.dma_req_addr := r.dma_req_addr + 4;
                    v.dma_req_state := DMAREQ_WDATA;
                end if;
            end if;
        when others =>
        end case;
    end if;

    procedureAxi4DMA(
        i_request => wb_dma_request,
        o_response => wb_dma_response,
        i_bank => r.dma,
        o_bank => v.dma,
        i_msti => i_msti,
        o_msto => wb_msto
    );
    dma_response <= wb_dma_response;

    o_uart.rts <= '1';
    if r.tx_state = idle then
        o_uart.td <= '1';
    else
        o_uart.td <= r.tx_shift(0);
    end if;


    if nrst = '0' then
        v.rd_z := i_uart.rd;
        v.baud_speed_detected := '0';
        v.baud_speed_confirmed := '0';
        v.baud_speed_cnt := (others => '0');
        v.baud_speed_cnt_z := (others => '0');
        v.scaler := (others => '0');
        v.scale_cnt := (others => '0');
        v.level := '0';

        v.tx_state := idle;
        v.rx_state := idle;
        v.tx_byte_cnt := (others => '0');
        v.tx_shift := (others => '0');
        v.tx_word := (others => '0');
        v.rx_shift := (others => '0');
        v.rx_byte := (others => '0');

        v.dma := DMA_BANK_RESET;
        v.dma_req_state := DMAREQ_IDLE;
        v.dma_req_write := '0';
        v.dma_byte_cnt := 0;
        v.dma_req_len := 0;
        v.dma_req_addr := (others => '0');
        v.dma_req_wdata := (others => '0');
        v.rword_valid := '0';
        v.rword := (others => '0');
    end if;

    rin <= v;
    o_msto <= wb_msto;
  end process;

  o_mstcfg <= xmstconfig;

  -- registers:
  regs : process(clk)
  begin 
     if rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;