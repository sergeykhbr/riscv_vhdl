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
    i_pipeline_hold : in std_logic;
    i_mem_req_ready : in std_logic;
    o_mem_addr_valid : out std_logic;
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data : in std_logic_vector(31 downto 0);
    o_mem_resp_ready : out std_logic;

    i_e_npc_valid : in std_logic;
    i_e_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_predict_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_predict_miss : out std_logic;

    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_hold : out std_logic
  );
end; 
 
architecture arch_InstrFetch of InstrFetch is

  type RegistersType is record
      wait_resp : std_logic;
      pipeline_init : std_logic_vector(4 downto 0);
      pc_z1 : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      raddr_not_resp_yet : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  end record;

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_pipeline_hold, i_mem_req_ready, i_mem_data_valid,
                i_mem_data_addr, i_mem_data, i_e_npc_valid, i_e_npc,
                i_predict_npc, r)
    variable v : RegistersType;
    variable wb_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable w_wrong_address : std_logic;
    variable w_req_valid : std_logic;
    variable w_predict_miss : std_logic;
    variable w_req_fire : std_logic;
    variable w_resp_fire : std_logic;
    variable w_o_hold : std_logic;
    variable w_o_mem_resp_ready : std_logic;
  begin

    v := r;

    w_req_valid := i_nrst and not i_pipeline_hold;
    w_req_fire := w_req_valid and i_mem_req_ready;

    w_wrong_address := '1';
    if (i_e_npc = r.pc_z1) or (i_e_npc = i_mem_data_addr) or 
       (i_e_npc = i_predict_npc) or (i_e_npc = r.raddr_not_resp_yet) then
        w_wrong_address := '0';
    end if;

    w_predict_miss := '0';
    if w_wrong_address = '1' then
        wb_addr_req := i_e_npc;
        w_predict_miss := w_req_fire;
    else
        wb_addr_req := i_predict_npc;
    end if;

    if w_req_fire = '1' then
        v.raddr_not_resp_yet := wb_addr_req; -- Address already requested but probably not responded yet.
                                             -- Avoid marking such request as 'miss'.
    end if;

    w_resp_fire := i_mem_data_valid and not i_pipeline_hold;
    if w_req_fire = '1'then
      v.wait_resp := '1';
      v.pc_z1 := i_mem_data_addr;
      v.pipeline_init := r.pipeline_init(3 downto 0) & '1';
    elsif (i_mem_data_valid and not w_req_fire) = '1' then
      v.wait_resp := '0';
    end if;
    
    w_o_hold := not i_mem_req_ready or (r.wait_resp and not i_mem_data_valid);
    -- Signal 'i_mem_req_ready' is also used to hold-on pipeline, so
    -- don't accept response if cannot send request. Maybe improved.
    w_o_mem_resp_ready := not i_pipeline_hold and i_mem_req_ready;

    if i_nrst = '0' then
        v.wait_resp := '0';
        v.pipeline_init := (others => '0');
        v.pc_z1 := (others => '0');
        v.raddr_not_resp_yet := (others => '0');
    end if;

    o_mem_addr_valid <= w_req_fire;
    o_mem_addr <= wb_addr_req;
    o_valid <= w_resp_fire;
    o_pc <= i_mem_data_addr;
    o_instr <= i_mem_data;
    o_predict_miss <= w_predict_miss;
    o_mem_resp_ready <= w_o_mem_resp_ready;
    o_hold <= w_o_hold;
    
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
