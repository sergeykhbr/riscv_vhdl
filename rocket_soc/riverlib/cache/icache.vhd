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
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    -- Debug Signals:
    o_istate : out std_logic_vector(1 downto 0);
    o_istate_z : out std_logic_vector(1 downto 0);
    o_ierr_state : out std_logic
  );
end; 
 
architecture arch_ICache of ICache is

  constant State_Idle : std_logic_vector(1 downto 0) := "00";
  constant State_WaitGrant : std_logic_vector(1 downto 0) := "01";
  constant State_WaitResp : std_logic_vector(1 downto 0) := "10";
  constant State_WaitAccept : std_logic_vector(1 downto 0) := "11";

  type RegistersType is record
      iline_addr : std_logic_vector(BUS_ADDR_WIDTH-4 downto 0);
      iline_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      iline_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      iline_addr_hit : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      iline_data_hit : std_logic_vector(31 downto 0);
      state : std_logic_vector(1 downto 0);
      state_z : std_logic_vector(1 downto 0);
      hit_line : std_logic;
      err_state : std_logic;
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_ctrl_valid, i_req_ctrl_addr,
                i_resp_ctrl_ready, i_req_mem_ready, 
                i_resp_mem_data_valid, i_resp_mem_data, r)
    variable v : RegistersType;
    variable w_o_req_ctrl_ready : std_logic;
    variable w_o_req_mem_valid : std_logic;
    variable wb_o_req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable w_req_fire : std_logic;
    variable w_o_resp_valid : std_logic;
    variable wb_o_resp_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_resp_data : std_logic_vector(31 downto 0);
    variable w_hit_req : std_logic;
    variable w_hit_line : std_logic;
    variable w_hit : std_logic;
    variable wb_req_line : std_logic_vector(BUS_ADDR_WIDTH-4 downto 0);
  begin

    v := r;
    w_hit_req := '0';
    w_hit_line := '0';
    wb_req_line := i_req_ctrl_addr(BUS_ADDR_WIDTH-1 downto 3);
    if (i_resp_mem_data_valid = '1'
      and (wb_req_line = r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3))) then
        w_hit_req := '1';
    end if;
    if (wb_req_line = r.iline_addr) then
        w_hit_line := '1';
    end if;
    w_hit := w_hit_req or w_hit_line;

    w_o_req_mem_valid := not w_hit and i_req_ctrl_valid;
    wb_o_req_mem_addr := i_req_ctrl_addr(BUS_ADDR_WIDTH-1 downto 3) & "000";
    w_o_req_ctrl_ready := w_hit or i_req_mem_ready;
    w_req_fire := i_req_ctrl_valid and w_o_req_ctrl_ready;
    case r.state is
    when State_Idle =>
        if i_req_ctrl_valid = '1' then
            if w_hit = '1' then
                v.state := State_WaitAccept;
            elsif i_req_mem_ready = '1' then
                v.state := State_WaitResp;
            else
                v.state := State_WaitGrant;
            end if;
        end if;
    when State_WaitGrant =>
        if i_req_mem_ready = '1' then
            v.state := State_WaitResp;
        elsif w_hit = '1' then
            v.err_state := '1';
        end if;
    when State_WaitResp =>
        if i_resp_mem_data_valid = '1' then
            if i_resp_ctrl_ready = '0' then
                v.state := State_WaitAccept;
            elsif i_req_ctrl_valid = '0' then
                v.state := State_Idle;
            else
                -- New request
                if w_hit = '1' then
                    v.state := State_WaitAccept;
                elsif i_req_mem_ready = '1' then
                    v.state := State_WaitResp;
                else
                    v.state := State_WaitGrant;
                end if;
            end if;
        end if;
    when State_WaitAccept =>
        if i_resp_ctrl_ready = '1' then
            if i_req_ctrl_valid = '0' then
                v.state := State_Idle;
            else
                if w_hit = '1' then
                    v.state := State_WaitAccept;
                elsif i_req_mem_ready = '1' then
                    v.state := State_WaitResp;
                else
                    v.state := State_WaitGrant;
                end if;
            end if;
        end if;
    when others =>
    end case;

    if v.state /= r.state then
        v.state_z := r.state;
    end if;


    if w_req_fire = '1' then
        v.iline_addr_req := i_req_ctrl_addr;
        v.hit_line := '0';
        if w_hit_line = '1' then
            v.hit_line := '1';
            v.iline_addr_hit := i_req_ctrl_addr;
            if i_req_ctrl_addr(2) = '0' then
                v.iline_data_hit := r.iline_data(31 downto 0);
            else
                v.iline_data_hit := r.iline_data(63 downto 32);
            end if;
        end if;
    end if;
    if i_resp_mem_data_valid = '1' then
        v.iline_addr := r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3);
        v.iline_data :=  i_resp_mem_data;
    end if;

    wb_o_resp_addr := r.iline_addr_req;
    if r.state = State_WaitAccept then
        w_o_resp_valid := '1';
        if r.hit_line = '1' then
            wb_o_resp_addr := r.iline_addr_hit;
            wb_o_resp_data := r.iline_data_hit;
        else 
            if r.iline_addr_req(2) = '0' then
                wb_o_resp_data := r.iline_data(31 downto 0);
            else
                wb_o_resp_data := r.iline_data(63 downto 32);
            end if;
        end if;
    else
        w_o_resp_valid := i_resp_mem_data_valid;
        if r.iline_addr_req(2) = '0' then
            wb_o_resp_data := i_resp_mem_data(31 downto 0);
        else
            wb_o_resp_data := i_resp_mem_data(63 downto 32);
        end if;
    end if;

    if i_nrst = '0' then
        v.iline_addr_req := (others => '0');
        v.iline_addr := (others => '1');
        v.iline_data := (others => '0');
        v.state := State_Idle;
        v.state_z := State_Idle;
        v.iline_addr_hit := (others => '1');
        v.iline_data_hit := (others => '0');
        v.hit_line := '0';
        v.err_state := '0';
    end if;

    o_req_ctrl_ready <= w_o_req_ctrl_ready;
    
    o_req_mem_valid <= w_o_req_mem_valid;
    o_req_mem_addr <= wb_o_req_mem_addr;
    o_req_mem_write <= '0';
    o_req_mem_strob <= (others => '0');
    o_req_mem_data <= (others => '0');

    o_resp_ctrl_valid <= w_o_resp_valid;
    o_resp_ctrl_data <= wb_o_resp_data;
    o_resp_ctrl_addr <= wb_o_resp_addr;
    o_istate <= r.state;
    o_istate_z <= r.state_z;
    o_ierr_state <= r.err_state;
    
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
