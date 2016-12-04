-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Instruction Cache.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;

entity ICache is
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    -- Control path:
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    i_resp_ctrl_ready : in std_logic;
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
 
architecture arch_ICache of ICache is

  type RegistersType is record
      iline_addr : std_logic_vector(BUS_ADDR_WIDTH-4 downto 0);
      iline_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);

      iline_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      ihit : std_logic;
      ihit_data : std_logic_vector(31 downto 0);
      hold_ena : std_logic;
      hold_data : std_logic_vector(31 downto 0);
      hold_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_ctrl_valid, i_req_ctrl_addr,
                i_resp_ctrl_ready, i_req_mem_ready, 
                i_resp_mem_data_valid, i_resp_mem_data, r)
    variable v : RegistersType;
    variable wb_req_line : std_logic_vector(BUS_ADDR_WIDTH-4 downto 0);
    variable wb_cached_addr : std_logic_vector(BUS_ADDR_WIDTH-4 downto 0);
    variable wb_cached_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable w_mem_valid : std_logic;
    variable wb_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable w_valid : std_logic;
    variable wb_data : std_logic_vector(31 downto 0);
    variable w_o_valid : std_logic;
    variable wb_o_data : std_logic_vector(31 downto 0);
    variable wb_o_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  begin

    v := r;
    w_mem_valid := '0';
    wb_mem_addr := (others => '0');
    w_valid := '0';
    wb_data := (others => '0');
    w_o_valid := '0';
    wb_o_data := (others => '0');
    wb_o_addr := (others => '0');

    if i_resp_mem_data_valid = '1' then
        wb_cached_addr := r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3);
        wb_cached_data := i_resp_mem_data;
    else
        wb_cached_addr := r.iline_addr;
        wb_cached_data := r.iline_data;
    end if;

    wb_req_line := i_req_ctrl_addr(BUS_ADDR_WIDTH-1 downto 3);
    v.ihit := '0';

    if (i_req_ctrl_valid and i_req_mem_ready) = '1' then
        v.iline_addr_req := i_req_ctrl_addr;
        if wb_req_line = wb_cached_addr then
            v.ihit := '1';
            if i_req_ctrl_addr(2) = '0' then
                v.ihit_data := wb_cached_data(31 downto 0);
            else
                v.ihit_data := wb_cached_data(63 downto 32);
            end if;
        else
            w_mem_valid := '1';
            wb_mem_addr := i_req_ctrl_addr(BUS_ADDR_WIDTH-1 downto 3) & "000";
        end if;
    end if;

    if i_resp_mem_data_valid = '1' then
        w_valid := '1';
        v.iline_addr := r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3);
        v.iline_data := i_resp_mem_data;
        if r.iline_addr_req(2) = '0' then
            wb_data := i_resp_mem_data(31 downto 0);
        else
            wb_data := i_resp_mem_data(63 downto 32);
        end if;
        v.hold_ena := not i_resp_ctrl_ready;
        v.hold_addr := r.iline_addr_req;
        v.hold_data := wb_data;
    elsif r.ihit = '1' then
        w_valid := '1';
        wb_data := r.ihit_data;
        v.hold_ena := not i_resp_ctrl_ready;
        v.hold_addr := r.iline_addr_req;
        v.hold_data := r.ihit_data;
    end if;
    
    if w_valid = '1' then
      w_o_valid := i_resp_ctrl_ready;
      wb_o_data := wb_data;
      wb_o_addr := r.iline_addr_req;
    elsif r.hold_ena = '1' then
      v.hold_ena := not i_resp_ctrl_ready;
      w_o_valid := i_resp_ctrl_ready;
      wb_o_data := r.hold_data;
      wb_o_addr := r.hold_addr;
    end if;

    if i_nrst = '0' then
        v.iline_addr_req := (others => '0');
        v.iline_addr := (others => '1');
        v.iline_data := (others => '0');
        v.ihit := '0';
        v.ihit_data := (others => '0');
        v.hold_ena := '0';
        v.hold_addr := (others => '0');
        v.hold_data := (others => '0');
    end if;

    o_req_ctrl_ready <= i_req_mem_ready; -- cannot accept request when memory is busy (can be improved).
    
    o_req_mem_valid <= w_mem_valid;
    o_req_mem_addr <= wb_mem_addr;
    o_req_mem_write <= '0';
    o_req_mem_strob <= (others => '0');
    o_req_mem_data <= (others => '0');

    o_resp_ctrl_valid <= w_o_valid;
    o_resp_ctrl_data <= wb_o_data;
    o_resp_ctrl_addr <= wb_o_addr;
    
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
