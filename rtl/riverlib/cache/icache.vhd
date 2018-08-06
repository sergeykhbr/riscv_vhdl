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
    o_istate : out std_logic_vector(1 downto 0)
  );
end; 
 
architecture arch_ICache of ICache is

  constant State_Idle : std_logic_vector(1 downto 0) := "00";
  constant State_WaitGrant : std_logic_vector(1 downto 0) := "01";
  constant State_WaitResp : std_logic_vector(1 downto 0) := "10";
  constant State_WaitAccept : std_logic_vector(1 downto 0) := "11";

  constant Hit_Line1 : integer := 0;
  constant Hit_Line2 : integer := 1;
  constant Hit_Response : integer := 2;
  constant Hit_Total : integer := 3;

  constant ILINE_TOTAL : integer := 2;

  type line_type is record
      addr : std_logic_vector(BUS_ADDR_WIDTH-4 downto 0);
      data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  end record;

  type iline_vector is array (0 to ILINE_TOTAL-1) of line_type;

  type line_signal_type is record
       hit : std_logic_vector(ILINE_TOTAL downto 0);     -- Hit_Total = ILINE_TOTAL + 1
       hit_hold : std_logic_vector(ILINE_TOTAL-1 downto 0);
       hit_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
       hold_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  end record;
 
  type line_signal_vector is array (0 to ILINE_TOTAL-1) of line_signal_type;

  type RegistersType is record
      iline : iline_vector;
      iline_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      addr_processing : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      state : std_logic_vector(1 downto 0);
      delay_valid : std_logic;
      delay_data : std_logic_vector(31 downto 0);
  end record;

  type adr_type is array (0 to 1) of std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_ctrl_valid, i_req_ctrl_addr,
                i_resp_ctrl_ready, i_req_mem_ready, 
                i_resp_mem_data_valid, i_resp_mem_data, r)
    variable v : RegistersType;
    variable w_need_mem_req : std_logic;
    variable wb_hit_word : std_logic_vector(31 downto 0);
    variable wb_l : line_signal_vector;
    variable w_reuse_lastline : std_logic;

    variable w_o_req_ctrl_ready : std_logic;
    variable w_o_req_mem_valid : std_logic;
    variable wb_o_req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable w_req_ctrl_valid : std_logic;
    variable w_req_fire : std_logic;
    variable w_o_resp_valid : std_logic;
    variable wb_o_resp_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_resp_data : std_logic_vector(31 downto 0);
    variable wb_req_addr : adr_type;
    variable wb_hold_addr : adr_type;
  begin

    v := r;

    w_req_ctrl_valid := i_req_ctrl_valid;
    wb_req_addr(0) := i_req_ctrl_addr;
    wb_req_addr(1) := i_req_ctrl_addr + 2;

    wb_hold_addr(0) := r.addr_processing;
    wb_hold_addr(1) := r.addr_processing + 2;

    for i in 0 to ILINE_TOTAL-1 loop
        wb_l(i).hit := (others => '0');
        wb_l(i).hit_data := (others => '0');
        if wb_req_addr(i)(BUS_ADDR_WIDTH-1 downto 3) = r.iline(0).addr then
            wb_l(i).hit(Hit_Line1) := w_req_ctrl_valid;
            wb_l(i).hit_data := r.iline(0).data;
        elsif wb_req_addr(i)(BUS_ADDR_WIDTH-1 downto 3) = r.iline(1).addr then
            wb_l(i).hit(Hit_Line2) := w_req_ctrl_valid;
            wb_l(i).hit_data := r.iline(1).data;
        elsif wb_req_addr(i)(BUS_ADDR_WIDTH-1 downto 3) =
            r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3) then
            wb_l(i).hit(Hit_Response) := i_resp_mem_data_valid;
            wb_l(i).hit_data := i_resp_mem_data;
        end if;

        wb_l(i).hit_hold := (others => '0');
        wb_l(i).hold_data := (others => '0');
        if wb_hold_addr(i)(BUS_ADDR_WIDTH-1 downto 3) = r.iline(0).addr then
            wb_l(i).hit_hold(Hit_Line1) := '1';
            wb_l(i).hold_data := r.iline(0).data;
        elsif wb_hold_addr(i)(BUS_ADDR_WIDTH-1 downto 3) = r.iline(1).addr then
            wb_l(i).hit_hold(Hit_Line2) := '1';
            wb_l(i).hold_data := r.iline(1).data;
        elsif wb_hold_addr(i)(BUS_ADDR_WIDTH-1 downto 3) =
            r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3) then
            wb_l(i).hold_data := i_resp_mem_data;
        end if;
    end loop;

    wb_hit_word := (others => '0');
    w_need_mem_req := '1';
    if wb_l(0).hit /= "000" and wb_l(1).hit /= "000" then
        w_need_mem_req := '0';
    end if;
    case r.addr_processing(2 downto 1) is
    when "00" =>
        wb_hit_word := wb_l(0).hold_data(31 downto 0);
    when "01" =>
        wb_hit_word := wb_l(0).hold_data(47 downto 16);
    when "10" =>
        wb_hit_word := wb_l(0).hold_data(63 downto 32);
    when others =>
        wb_hit_word := wb_l(1).hold_data(15 downto 0) &
                       wb_l(0).hold_data(63 downto 48);
    end case;

    if w_req_ctrl_valid = '1' and w_need_mem_req = '0' then
        v.delay_valid := '1';
        case i_req_ctrl_addr(2 downto 1) is
        when "00" =>
            v.delay_data := wb_l(0).hit_data(31 downto 0);
        when "01" =>
            v.delay_data := wb_l(0).hit_data(47 downto 16);
        when "10" =>
            v.delay_data := wb_l(0).hit_data(63 downto 32);
        when others =>
            v.delay_data := wb_l(1).hit_data(15 downto 0) &
                            wb_l(0).hit_data(63 downto 48);
        end case;
    elsif i_resp_ctrl_ready = '1' then
        v.delay_valid := '0';
        v.delay_data := (others => '0');
    end if;

    w_o_req_mem_valid := w_need_mem_req and w_req_ctrl_valid;
    if wb_l(0).hit = "000" then
        wb_o_req_mem_addr := wb_req_addr(0)(BUS_ADDR_WIDTH-1 downto 3)  & "000";
    else
        wb_o_req_mem_addr := wb_req_addr(1)(BUS_ADDR_WIDTH-1 downto 3)  & "000";
    end if;
    w_o_req_ctrl_ready := not w_need_mem_req or i_req_mem_ready;
    w_req_fire := w_req_ctrl_valid and w_o_req_ctrl_ready;

    if (w_o_req_mem_valid and i_req_mem_ready) = '1' then
        v.iline_addr_req := wb_o_req_mem_addr;
    end if;

    case r.state is
    when State_Idle =>
        if i_req_ctrl_valid = '1' then
            if w_need_mem_req = '0' then
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
        elsif w_need_mem_req = '0' then
            --! Fetcher can change request address while request wasn't
            --! accepteed.
            v.state := State_WaitAccept;
        end if;
    when State_WaitResp =>
        if i_resp_mem_data_valid = '1' then
            if i_resp_ctrl_ready = '0' then
                v.state := State_WaitAccept;
            elsif w_req_ctrl_valid = '0' then
                v.state := State_Idle;
            else
                -- New request
                if w_need_mem_req = '0' then
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
            if w_req_ctrl_valid = '0' then
                v.state := State_Idle;
            else
                if w_need_mem_req = '0' then
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


    if w_req_fire = '1' then
        v.addr_processing := i_req_ctrl_addr;
    end if;

    w_reuse_lastline := '0';

    if i_resp_mem_data_valid = '1' then
        --! Condition to avoid removing the last line:
        if i_resp_ctrl_ready = '1' then
            if (wb_l(0).hit(Hit_Line2) or wb_l(1).hit(Hit_Line2)) = '1'
                and r.iline(1).addr /= i_req_ctrl_addr(BUS_ADDR_WIDTH-1 downto 3) then
                w_reuse_lastline := w_need_mem_req;
            end if;
        else
            if (wb_l(0).hit_hold(Hit_Line2) or wb_l(1).hit_hold(Hit_Line2)) = '1'
                and (wb_l(0).hit_hold(Hit_Line1) or wb_l(1).hit_hold(Hit_Line1)) = '0'
                and r.iline(1).addr /= r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3) then
                w_reuse_lastline := '1';
            end if;
		  end if;
        if w_reuse_lastline = '0' then
            v.iline(1).addr := r.iline(0).addr;
            v.iline(1).data := r.iline(0).data;
        end if;
        
        v.iline(0).addr := r.iline_addr_req(BUS_ADDR_WIDTH-1 downto 3);
        v.iline(0).data := i_resp_mem_data;
    end if;

    if r.state = State_WaitAccept then
        w_o_resp_valid := '1';
    else
        w_o_resp_valid := i_resp_mem_data_valid;
    end if;
    if r.delay_valid = '1' then
        wb_o_resp_data := r.delay_data;
    else
        wb_o_resp_data := wb_hit_word;
    end if;
    wb_o_resp_addr := r.addr_processing;


    if i_nrst = '0' then
        v.iline(0).addr := (others => '1');
        v.iline(0).data := (others => '0');
        v.iline(1).addr := (others => '1');
        v.iline(1).data := (others => '0');
        v.iline_addr_req := (others => '0');
        v.addr_processing := (others => '0');
        v.state := State_Idle;
        v.delay_valid := '0';
        v.delay_data := (others => '0');
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
