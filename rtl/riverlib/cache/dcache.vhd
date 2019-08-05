-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Data Cache.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity DCache is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    -- Data path:
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_sz : in std_logic_vector(1 downto 0);
    i_req_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_req_data_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_req_data_ready : out std_logic;
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_resp_data_load_fault : out std_logic;
    o_resp_data_store_fault : out std_logic;
    o_resp_data_store_fault_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_data_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_req_mem_len : out std_logic_vector(7 downto 0);
    o_req_mem_burst : out std_logic_vector(1 downto 0);
    o_req_mem_last : out std_logic;
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_load_fault : in std_logic;
    i_resp_mem_store_fault : in std_logic;
    i_resp_mem_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -- Debug Signals:
    o_dstate : out std_logic_vector(1 downto 0)
  );
end; 
 
architecture arch_DCache of DCache is

  constant State_Idle : std_logic_vector(1 downto 0) := "00";
  constant State_WaitGrant : std_logic_vector(1 downto 0) := "01";
  constant State_WaitResp : std_logic_vector(1 downto 0) := "10";
  constant State_WaitAccept : std_logic_vector(1 downto 0) := "11";

  type RegistersType is record
      req_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
      req_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      dline_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      dline_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      dline_size_req : std_logic_vector(1 downto 0);
      dline_load_fault : std_logic;
      state : std_logic_vector(1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
      (others => '0'), (others => '0'),
      (others => '0'), (others => '0'), (others => '0'),
      '0', State_Idle);

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_data_valid, i_req_data_write, i_req_data_sz, 
                i_req_data_addr, i_req_data_data, i_resp_mem_data_valid, 
                i_resp_mem_data, i_resp_mem_load_fault, i_resp_mem_store_fault,
                i_resp_mem_store_fault_addr, i_req_mem_ready, i_resp_data_ready, r)
    variable v : RegistersType;
    variable w_wait_response : std_logic;
    variable w_o_req_data_ready : std_logic;
    variable w_o_req_mem_valid : std_logic;
    variable wb_o_req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_req_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    variable wb_o_req_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable w_req_fire : std_logic;
    variable w_o_resp_valid : std_logic;
    variable wb_o_resp_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_resp_data_mux : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable wb_o_resp_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable w_o_resp_load_fault : std_logic;
    variable wb_rtmp : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  begin

    v := r;
    wb_o_req_strob := (others => '0');
    wb_o_req_wdata := (others => '0');
    wb_o_resp_data := (others => '0');
    wb_rtmp := (others => '0');

    w_wait_response := '0';
    if r.state = State_WaitResp and i_resp_mem_data_valid = '0' then
        w_wait_response := '1';
    end if;

    case i_req_data_sz is
    when "00" =>
        wb_o_req_wdata := i_req_data_data(7 downto 0) &
            i_req_data_data(7 downto 0) & i_req_data_data(7 downto 0) &
            i_req_data_data(7 downto 0) & i_req_data_data(7 downto 0) &
            i_req_data_data(7 downto 0) & i_req_data_data(7 downto 0) &
            i_req_data_data(7 downto 0);
        if i_req_data_addr(2 downto 0) = "000" then
            wb_o_req_strob := X"01";
        elsif i_req_data_addr(2 downto 0) = "001" then
            wb_o_req_strob := X"02";
        elsif i_req_data_addr(2 downto 0) = "010" then
            wb_o_req_strob := X"04";
        elsif i_req_data_addr(2 downto 0) = "011" then
            wb_o_req_strob := X"08";
        elsif i_req_data_addr(2 downto 0) = "100" then
            wb_o_req_strob := X"10";
        elsif i_req_data_addr(2 downto 0) = "101" then
            wb_o_req_strob := X"20";
        elsif i_req_data_addr(2 downto 0) = "110" then
            wb_o_req_strob := X"40";
        elsif i_req_data_addr(2 downto 0) = "111" then
            wb_o_req_strob := X"80";
        end if;
    when "01" =>
        wb_o_req_wdata := i_req_data_data(15 downto 0) &
            i_req_data_data(15 downto 0) & i_req_data_data(15 downto 0) &
            i_req_data_data(15 downto 0);
        if i_req_data_addr(2 downto 1) = "00" then
            wb_o_req_strob := X"03";
        elsif i_req_data_addr(2 downto 1) = "01" then
            wb_o_req_strob := X"0C";
        elsif i_req_data_addr(2 downto 1) = "10" then
            wb_o_req_strob := X"30";
        else
            wb_o_req_strob := X"C0";
        end if;
    when "10" =>
        wb_o_req_wdata := i_req_data_data(31 downto 0) &
                          i_req_data_data(31 downto 0);
        if i_req_data_addr(2) = '1' then
            wb_o_req_strob := X"F0";
        else
            wb_o_req_strob := X"0F";
        end if;
    when "11" =>
        wb_o_req_wdata := i_req_data_data;
        wb_o_req_strob := X"FF";
    when others =>
    end case;


    w_o_req_mem_valid := i_req_data_valid and not w_wait_response;
    wb_o_req_mem_addr := i_req_data_addr(BUS_ADDR_WIDTH-1 downto 3) & "000";
    w_o_req_data_ready := i_req_mem_ready;
    w_req_fire := w_o_req_mem_valid and w_o_req_data_ready;
    case r.state is
    when State_Idle =>
        if i_req_data_valid = '1' then
            if i_req_mem_ready = '1' then
                v.state := State_WaitResp;
            else
                v.state := State_WaitGrant;
            end if;
        end if;
    when State_WaitGrant =>
        if i_req_mem_ready = '1' then
            v.state := State_WaitResp;
        end if;
    when State_WaitResp =>
        if i_resp_mem_data_valid = '1' then
            if i_resp_data_ready = '0' then
                v.state := State_WaitAccept;
            elsif i_req_data_valid = '0' then
                v.state := State_Idle;
            else
                -- New request
                if i_req_mem_ready = '1' then
                    v.state := State_WaitResp;
                else
                    v.state := State_WaitGrant;
                end if;
            end if;
        end if;
    when State_WaitAccept =>
        if i_resp_data_ready = '1' then
            if i_req_data_valid = '0' then
                v.state := State_Idle;
            else
                if i_req_mem_ready = '1' then
                    v.state := State_WaitResp;
                else
                    v.state := State_WaitGrant;
                end if;
            end if;
        end if;
    when others =>
    end case;

    if w_req_fire = '1' then
        v.dline_addr_req := i_req_data_addr;
        v.dline_size_req := i_req_data_sz;
        v.req_strob := wb_o_req_strob;
        v.req_wdata := wb_o_req_wdata;
    end if;
    if i_resp_mem_data_valid = '1' then
        v.dline_data := i_resp_mem_data;
        v.dline_load_fault := i_resp_mem_load_fault;
    end if;

    wb_o_resp_addr := r.dline_addr_req;
    if r.state = State_WaitAccept then
        w_o_resp_valid := '1';
        wb_resp_data_mux := r.dline_data;
        w_o_resp_load_fault := r.dline_load_fault;
    else
        w_o_resp_valid := i_resp_mem_data_valid;
        wb_resp_data_mux := i_resp_mem_data;
        w_o_resp_load_fault := i_resp_mem_load_fault;
    end if;

    case r.dline_addr_req(2 downto 0) is
    when "001" =>
        wb_rtmp := X"00" & wb_resp_data_mux(63 downto 8);
    when "010" =>
        wb_rtmp := X"0000" & wb_resp_data_mux(63 downto 16);
    when "011" =>
        wb_rtmp := X"000000" & wb_resp_data_mux(63 downto 24);
    when "100" =>
        wb_rtmp := X"00000000" & wb_resp_data_mux(63 downto 32);
    when "101" =>
        wb_rtmp := X"0000000000" & wb_resp_data_mux(63 downto 40);
    when "110" =>
        wb_rtmp := X"000000000000" & wb_resp_data_mux(63 downto 48);
    when "111" =>
        wb_rtmp := X"00000000000000" & wb_resp_data_mux(63 downto 56);
    when others =>
        wb_rtmp := wb_resp_data_mux;
    end case;

    case r.dline_size_req is
    when "00" =>
        wb_o_resp_data(7 downto 0) := wb_rtmp(7 downto 0);
    when "01" =>
        wb_o_resp_data(15 downto 0) := wb_rtmp(15 downto 0);
    when "10" =>
        wb_o_resp_data(31 downto 0) := wb_rtmp(31 downto 0);
    when others =>
        wb_o_resp_data := wb_rtmp;
    end case;
    
    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_req_data_ready <= w_o_req_data_ready;

    o_req_mem_valid <= w_o_req_mem_valid;
    o_req_mem_addr <= wb_o_req_mem_addr;
    o_req_mem_write <= i_req_data_write;
    o_req_mem_strob <= r.req_strob;
    o_req_mem_data <= r.req_wdata;
    o_req_mem_len <= X"00";
    o_req_mem_burst <= "00";
    o_req_mem_last <= '1';

    o_resp_data_valid <= w_o_resp_valid;
    o_resp_data_data <= wb_o_resp_data;
    o_resp_data_addr <= wb_o_resp_addr;
    o_resp_data_load_fault <= w_o_resp_load_fault;
    -- AXI b channel
    o_resp_data_store_fault <= i_resp_mem_store_fault;
    o_resp_data_store_fault_addr <= i_resp_mem_store_fault_addr;
    o_dstate <= r.state;
    
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
