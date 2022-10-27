--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
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

entity uart_tap is
  port (
    nrst     : in std_logic;
    clk      : in std_logic;
    i_uart   : in  uart_in_type;
    o_uart   : out uart_out_type;
    i_msti   : in axi4_master_in_type;
    o_msto   : out axi4_master_out_type;
    o_mstcfg : out axi4_master_config_type
  );
end; 
 
architecture arch_uart_tap of uart_tap is

  constant MAGIC_ID : std_logic_vector(7 downto 0) := X"31";
  constant SCALER_DEFAULT : std_logic_vector(17 downto 0) := "111111111111111011";
  constant BAUD_DEFAULT : std_logic_vector(17 downto 0) := (others => '1');
  constant HANDSHAKE_ACK : std_logic_vector(31 downto 0) := X"0a4b4341";

  constant xmstconfig : axi4_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_UART_TAP
  );

  type uart_state_type is (idle, startbit, data, stopbit);
  type dma_req_state_type is (
      DMAREQ_IDLE,
      DMAREQ_OPERATION,
      DMAREQ_ADDR,
      DMAREQ_READ,
      DMAREQ_WAIT_READ_RESP,
      DMAREQ_UART_TX,
      DMAREQ_WDATA,
      DMAREQ_WRITE
   );

  type registers is record
    dma : dma_bank_type;

    tx_data   : std_logic_vector(31 downto 0);
    tx_byte_cnt : integer range 0 to 4;

    dma_req_state : dma_req_state_type;
    dma_state_next : dma_req_state_type;
    dma_req_write : std_logic;
    dma_byte_cnt : integer range 0 to 7;
    dma_req_len : integer range 0 to 63;
    dma_req_addr : std_logic_vector(63 downto 0);
    dma_req_wdata : std_logic_vector(31 downto 0);
    rword_valid : std_logic;
    rword : std_logic_vector(31 downto 0);
    watchdog : integer;
  end record;

  signal r, rin : registers;
  signal dma_response : dma_response_type;

  signal w_com_dready : std_logic;   -- new byte is avaiable for read
  signal w_com_accepted : std_logic; -- new byte can be accepted;
  signal wb_com_data : std_logic_vector(7 downto 0);
  signal w_com_thempty : std_logic; -- transmitter's hold register is empty
  signal w_com_write : std_logic;

  component dcom_uart is
  port (
    rst    : in  std_ulogic;
    clk    : in  std_ulogic;
    i_cfg_frame : in std_logic;
    i_cfg_ovf   : in std_logic;
    i_cfg_break : in std_logic;
    i_cfg_tcnt  : in std_logic_vector(1 downto 0);
    i_cfg_rxen  : in std_logic;
    i_cfg_brate : in std_logic_vector(17 downto 0);
    i_cfg_scaler : in std_logic_vector(17 downto 0);
    o_cfg_scaler : out std_logic_vector(31 downto 0);
    o_cfg_rxen : out std_logic;
    o_cfg_txen : out std_logic;
    o_cfg_flow : out std_logic;

    i_com_read    	: in std_ulogic;
    i_com_write   	: in std_ulogic;
    i_com_data		: in std_logic_vector(7 downto 0);
    o_com_dready 	: out std_ulogic;
    o_com_tsempty	: out std_ulogic;
    o_com_thempty	: out std_ulogic;
    o_com_lock    	: out std_ulogic;
    o_com_enable 	: out std_ulogic;
    o_com_data		: out std_logic_vector(7 downto 0);
    ui     : in  uart_in_type;
    uo     : out uart_out_type
  );
  end component; 

begin

  comblogic : process(nrst, i_msti, i_uart, r, dma_response, w_com_dready,
                      wb_com_data, w_com_thempty)
    variable v : registers;
    variable wb_dma_request : dma_request_type;
    variable wb_dma_response : dma_response_type;
    variable wb_msto : axi4_master_out_type;

    variable v_com_write : std_logic;
    variable v_com_accepted : std_logic;
  begin

    v := r;
    wb_dma_request.valid := '0';
    wb_dma_request.ready := '0';
    wb_dma_request.write := '0';
    wb_dma_request.addr := (others => '0');
    wb_dma_request.size := "010"; -- 4 bytes
    wb_dma_request.bytes := (others => '0');
    wb_dma_request.wdata := (others => '0');

    v_com_accepted := '0';
    v_com_write := '0';

    --! DMA control
    case r.dma_req_state is
    when DMAREQ_IDLE =>
        v_com_accepted := '1';
        if w_com_dready = '1' and wb_com_data = MAGIC_ID then
            v.dma_req_state := DMAREQ_OPERATION;
        end if;
    when DMAREQ_OPERATION =>
        v_com_accepted := '1';
        if w_com_dready = '1' then
            v.dma_req_write := wb_com_data(6);
            v.dma_req_len := conv_integer(wb_com_data(5 downto 0));
            v.dma_req_state := DMAREQ_ADDR;
            v.dma_byte_cnt := 0;
        end if;
    when DMAREQ_ADDR =>
        v_com_accepted := '1';
        if w_com_dready = '1' then
            v.dma_req_addr := wb_com_data & r.dma_req_addr(63 downto 8);
            if r.dma_byte_cnt = 7 then
                if (wb_com_data & r.dma_req_addr(63 downto 40)) /= X"00000000" then
                    v.dma_req_state := DMAREQ_IDLE;
                elsif r.dma_req_write = '1' then
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
        wb_dma_request.valid := '1';
        wb_dma_request.write := '0';
        wb_dma_request.addr := r.dma_req_addr(CFG_SYSBUS_ADDR_BITS-1 downto 0);
        wb_dma_request.bytes := conv_std_logic_vector(4, 11);
        wb_dma_request.wdata := (others => '0');
        if dma_response.ready = '1' then
            v.dma_req_state := DMAREQ_WAIT_READ_RESP;
        end if;
    when DMAREQ_WAIT_READ_RESP =>
        wb_dma_request.ready := '1';
        if dma_response.valid = '1' then
            v.dma_req_state := DMAREQ_UART_TX;
            v.tx_data := dma_response.rdata(31 downto 0);
            v.tx_byte_cnt := 4;
            if r.dma_req_len = 0 then
                v.dma_state_next := DMAREQ_IDLE;
            else
                v.dma_req_len := r.dma_req_len - 1;
                v.dma_req_addr := r.dma_req_addr + 4;
                v.dma_state_next := DMAREQ_READ;
            end if;
        end if;

    when DMAREQ_WDATA =>
        v_com_accepted := '1';
        if w_com_dready = '1' then
            v.dma_req_wdata := wb_com_data & r.dma_req_wdata(31 downto 8);
            v.dma_byte_cnt := r.dma_byte_cnt + 1;
            if r.dma_byte_cnt = 3 then
               v.dma_req_state := DMAREQ_WRITE;
            end if;
        end if;
    when DMAREQ_WRITE =>
        wb_dma_request.valid := '1';
        wb_dma_request.write := '1';
        wb_dma_request.addr := r.dma_req_addr(CFG_SYSBUS_ADDR_BITS-1 downto 0);
        wb_dma_request.bytes := conv_std_logic_vector(4, 11);
        wb_dma_request.wdata := r.dma_req_wdata & r.dma_req_wdata;
        if dma_response.ready = '1' then
            if r.dma_req_len = 0 then
                v.dma_req_state := DMAREQ_UART_TX; -- Handshake ACK
                v.tx_data := HANDSHAKE_ACK;
                v.tx_byte_cnt := 4;
                v.dma_state_next := DMAREQ_IDLE;
            else
                v.dma_byte_cnt := 0;
                v.dma_req_len := r.dma_req_len - 1;
                v.dma_req_addr := r.dma_req_addr + 4;
                v.dma_req_state := DMAREQ_WDATA;
            end if;
        end if;

    when DMAREQ_UART_TX =>
        v_com_write := '1';
        if r.tx_byte_cnt = 0 then
            v.dma_req_state := r.dma_state_next;
        elsif w_com_thempty = '1' then
            v.tx_byte_cnt := r.tx_byte_cnt - 1;
            v.tx_data := X"00" & r.tx_data(31 downto 8);
        end if;
    when others =>
    end case;


    procedureAxi4DMA(
        i_request => wb_dma_request,
        o_response => wb_dma_response,
        i_bank => r.dma,
        o_bank => v.dma,
        i_msti => i_msti,
        o_msto => wb_msto
    );
    dma_response <= wb_dma_response;
    w_com_accepted <= v_com_accepted;
    w_com_write <= v_com_write;


    if nrst = '0' then
        v.tx_byte_cnt := 0;
        v.tx_data := (others => '0');

        v.dma := DMA_BANK_RESET;
        v.dma_req_state := DMAREQ_IDLE;
        v.dma_state_next := DMAREQ_IDLE;
        v.dma_req_write := '0';
        v.dma_byte_cnt := 0;
        v.dma_req_len := 0;
        v.dma_req_addr := (others => '0');
        v.dma_req_wdata := (others => '0');
    end if;

    rin <= v;
    o_msto <= wb_msto;
  end process;

  o_mstcfg <= xmstconfig;

  dcom0 : dcom_uart  port map (
    rst    => nrst,
    clk    => clk,
    i_cfg_frame => '0',
    i_cfg_ovf   => '0',
    i_cfg_break => '0',
    i_cfg_tcnt  => "00",
    i_cfg_rxen  => '0',
    i_cfg_brate => BAUD_DEFAULT,
    i_cfg_scaler => SCALER_DEFAULT,
    o_cfg_scaler => open,
    o_cfg_rxen => open,
    o_cfg_txen => open,
    o_cfg_flow => open,

    i_com_read => w_com_accepted,
    i_com_write => w_com_write,
    i_com_data	=> r.tx_data(7 downto 0),
    o_com_dready => w_com_dready,
    o_com_tsempty => open,
    o_com_thempty => w_com_thempty,
    o_com_lock => open,
    o_com_enable => open,
    o_com_data => wb_com_data,
    ui => i_uart,
    uo => o_uart
  );


  -- registers:
  regs : process(clk)
  begin 
     if rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;