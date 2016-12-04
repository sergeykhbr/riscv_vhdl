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


entity DCache is
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
    i_resp_data_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0)
  );
end; 
 
architecture arch_DCache of DCache is

  type RegistersType is record
      req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_size : std_logic_vector(1 downto 0);
      rena : std_logic;
      hold_ena : std_logic;
      hold_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      hold_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_data_valid, i_req_data_write, i_req_data_sz, 
                i_req_data_addr, i_req_data_data, i_resp_mem_data_valid, 
                i_resp_mem_data, i_req_mem_ready, i_resp_data_ready, r)
    variable v : RegistersType;
    variable wb_req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_req_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    variable wb_rdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable wb_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable wb_rtmp : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable w_o_valid : std_logic;
    variable wb_o_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable wb_o_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  begin

    v := r;
    wb_req_addr := (others => '0');
    wb_req_strob := (others => '0');
    wb_rdata := (others => '0');
    wb_wdata := (others => '0');
    wb_rtmp := (others => '0');
    w_o_valid := '0';
    wb_o_data := (others => '0');
    wb_o_addr := (others => '0');

    wb_req_addr(BUS_ADDR_WIDTH-1 downto 3) 
        := i_req_data_addr(BUS_ADDR_WIDTH-1 downto 3);

    v.rena := not i_req_data_write and i_req_data_valid;
    if i_req_data_write = '1' then
        case i_req_data_sz is
        when "00" =>
            wb_wdata := i_req_data_data(7 downto 0) &
                i_req_data_data(7 downto 0) & i_req_data_data(7 downto 0) &
                i_req_data_data(7 downto 0) & i_req_data_data(7 downto 0) &
                i_req_data_data(7 downto 0) & i_req_data_data(7 downto 0) &
                i_req_data_data(7 downto 0);
            if i_req_data_addr(2 downto 0) = "000" then
                wb_req_strob := X"01";
            elsif i_req_data_addr(2 downto 0) = "001" then
                wb_req_strob := X"02";
            elsif i_req_data_addr(2 downto 0) = "010" then
                wb_req_strob := X"04";
            elsif i_req_data_addr(2 downto 0) = "011" then
                wb_req_strob := X"08";
            elsif i_req_data_addr(2 downto 0) = "100" then
                wb_req_strob := X"10";
            elsif i_req_data_addr(2 downto 0) = "101" then
                wb_req_strob := X"20";
            elsif i_req_data_addr(2 downto 0) = "110" then
                wb_req_strob := X"40";
            elsif i_req_data_addr(2 downto 0) = "111" then
                wb_req_strob := X"80";
            end if;
        when "01" =>
            wb_wdata := i_req_data_data(15 downto 0) &
                i_req_data_data(15 downto 0) & i_req_data_data(15 downto 0) &
                i_req_data_data(15 downto 0);
            if i_req_data_addr(2 downto 1) = "00" then
                wb_req_strob := X"03";
            elsif i_req_data_addr(2 downto 1) = "01" then
                wb_req_strob := X"0C";
            elsif i_req_data_addr(2 downto 1) = "10" then
                wb_req_strob := X"30";
            else
                wb_req_strob := X"C0";
            end if;
        when "10" =>
            wb_wdata := i_req_data_data(31 downto 0) &
                        i_req_data_data(31 downto 0);
            if i_req_data_addr(2) = '1' then
                wb_req_strob := X"F0";
            else
                wb_req_strob := X"0F";
            end if;
        when "11" =>
            wb_wdata := i_req_data_data;
            wb_req_strob := X"FF";
        when others =>
        end case;
    end if;

    case r.req_addr(2 downto 0) is
    when "001" =>
        wb_rtmp := X"00" & i_resp_mem_data(63 downto 8);
    when "010" =>
        wb_rtmp := X"0000" & i_resp_mem_data(63 downto 16);
    when "011" =>
        wb_rtmp := X"000000" & i_resp_mem_data(63 downto 24);
    when "100" =>
        wb_rtmp := X"00000000" & i_resp_mem_data(63 downto 32);
    when "101" =>
        wb_rtmp := X"0000000000" & i_resp_mem_data(63 downto 40);
    when "110" =>
        wb_rtmp := X"000000000000" & i_resp_mem_data(63 downto 48);
    when "111" =>
        wb_rtmp := X"00000000000000" & i_resp_mem_data(63 downto 56);
    when others =>
        wb_rtmp := i_resp_mem_data;
    end case;

    case r.req_size is
    when "00" =>
        wb_rdata(7 downto 0) := wb_rtmp(7 downto 0);
    when "01" =>
        wb_rdata(15 downto 0) := wb_rtmp(15 downto 0);
    when "10" =>
        wb_rdata(31 downto 0) := wb_rtmp(31 downto 0);
    when others =>
        wb_rdata := wb_rtmp;
    end case;
    
    if (i_req_data_valid and i_req_mem_ready) = '1' then
        v.req_addr := i_req_data_addr;
        v.req_size := i_req_data_sz;
    end if;
    
    if i_resp_mem_data_valid = '1' then
      w_o_valid := i_resp_data_ready;
      wb_o_data := wb_rdata;
      wb_o_addr := r.req_addr;
      v.hold_ena := not i_resp_data_ready;
      v.hold_addr := r.req_addr;
      v.hold_data := wb_rdata;
    elsif r.hold_ena = '1' then
      v.hold_ena := not i_resp_data_ready;
      w_o_valid := i_resp_data_ready;
      wb_o_data := r.hold_data;
      wb_o_addr := r.hold_addr;
    end if;


    if i_nrst = '0' then
        v.req_addr := (others => '0');
        v.req_size := (others => '0');
        v.rena := '0';
        v.hold_ena := '0';
        v.hold_addr := (others => '0');
        v.hold_data := (others => '0');
    end if;

    o_req_data_ready <= i_req_mem_ready;

    o_req_mem_valid <= i_req_data_valid;
    o_req_mem_write <= i_req_data_write;
    o_req_mem_addr <= wb_req_addr;
    o_req_mem_strob <= wb_req_strob;
    o_req_mem_data <= wb_wdata;

    o_resp_data_valid <= w_o_valid;
    o_resp_data_data <= wb_o_data;
    o_resp_data_addr <= wb_o_addr;
    
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk)
  begin 
     if rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
