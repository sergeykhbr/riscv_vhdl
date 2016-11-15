-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     CPU Fetch Instruction stage.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity InstrFetch is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_cache_hold : in std_logic;
    i_pipeline_hold : in std_logic;
    o_mem_addr_valid : out std_logic;
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data : in std_logic_vector(31 downto 0);

    i_e_npc_valid : in std_logic;
    i_e_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_predict_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_predict_miss : out std_logic;

    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0)
  );
end; 
 
architecture arch_InstrFetch of InstrFetch is

  type RegistersType is record
      f_valid : std_logic;
      pc_z0 : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      pc_z1 : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      raddr_not_resp_yet : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      instr : std_logic_vector(31 downto 0);
      wait_resp : std_logic;
      is_postponed : std_logic;
      postponed_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      postponed_instr : std_logic_vector(31 downto 0);
  end record;

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_cache_hold, i_pipeline_hold, i_mem_data_valid,
                i_mem_data_addr, i_mem_data, i_e_npc_valid, i_e_npc,
                i_predict_npc, r)
    variable v : RegistersType;
    variable w_mem_addr_valid : std_logic;
    variable wb_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable w_any_hold : std_logic;
    variable w_wrong_address : std_logic;
    variable w_predict_miss : std_logic;
    variable w_o_valid : std_logic;
  begin

    v := r;

    if (i_nrst = '0' or i_pipeline_hold = '1' or
         (not i_mem_data_valid and r.wait_resp) = '1') then
        -- Do not request new data:
        w_mem_addr_valid := '0';
    else
        w_mem_addr_valid := '1';
    end if;
    v.wait_resp := w_mem_addr_valid;

    w_wrong_address := '1';
    if (i_e_npc = r.pc_z1) or (i_e_npc = r.pc_z0) or 
       (i_e_npc = i_predict_npc) or (i_e_npc = r.raddr_not_resp_yet) then
        w_wrong_address := '0';
    end if;

    w_predict_miss := '0';
    if w_wrong_address = '1' then
        wb_addr_req := i_e_npc;
        w_predict_miss := '1';
    else
        wb_addr_req := i_predict_npc;
    end if;
    
    v.raddr_not_resp_yet := wb_addr_req; -- Address already requested but probably not responded yet.
                                         -- Avoid marking such request as 'miss'.

    w_any_hold := i_cache_hold or i_pipeline_hold;
    v.is_postponed := r.is_postponed and w_any_hold;
    if i_mem_data_valid = '1' then
        v.f_valid := '1';
        if w_any_hold = '0' then
            -- direct transition:
            v.instr := i_mem_data;
            v.pc_z1 := r.pc_z0;
            v.pc_z0 := i_mem_data_addr;
        else
            -- Postpone recieved data when gold signal down:
            v.is_postponed := '1';
            v.postponed_pc := i_mem_data_addr;
            v.postponed_instr := i_mem_data;
        end if;
    elsif w_any_hold = '0' then
        if r.is_postponed = '1' then
            v.instr := r.postponed_instr;
            v.pc_z1 := r.pc_z0;
            v.pc_z0 := r.postponed_pc;
        else
            v.f_valid := '0';
        end if;
    end if;
    w_o_valid := r.f_valid and not w_any_hold;

    if i_nrst = '0' then
        v.f_valid := '0';
        v.pc_z0 := (others => '0');
        v.pc_z1 := (others => '0');
        v.raddr_not_resp_yet := (others => '0');
        v.is_postponed := '0';
        v.postponed_pc := (others => '0');
        v.postponed_instr := (others => '0');
        v.wait_resp := '0';
    end if;

    o_mem_addr_valid <= w_mem_addr_valid;
    o_mem_addr <= wb_addr_req;
    o_valid <= w_o_valid;
    o_pc <= r.pc_z0;
    o_instr <= r.instr;
    o_predict_miss <= w_predict_miss;
    
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
