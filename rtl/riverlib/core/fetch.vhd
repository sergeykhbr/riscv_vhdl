-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2018 GNSS Sensor Ltd. All right reserved.
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
    i_mem_load_fault : in std_logic;
    o_mem_resp_ready : out std_logic;

    i_e_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_predict_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_predict : in std_logic;
    o_predict_miss : out std_logic;

    o_mem_req_fire : out std_logic;                    -- used by branch predictor to form new npc value
    o_ex_load_fault : out std_logic;
    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_hold : out std_logic;                                -- Hold due no response from icache yet
    i_br_fetch_valid : in std_logic;                       -- Fetch injection address/instr are valid
    i_br_address_fetch : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Fetch injection address to skip ebreak instruciton only once
    i_br_instr_fetch : in std_logic_vector(31 downto 0);   -- Real instruction value that was replaced by ebreak
    o_instr_buf : out std_logic_vector(DBG_FETCH_TRACE_SIZE*64-1 downto 0)    -- trace last fetched instructions
  );
end; 
 
architecture arch_InstrFetch of InstrFetch is

  type RegistersType is record
      wait_resp : std_logic;
      pipeline_init : std_logic_vector(4 downto 0);
      pc_z1 : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      raddr_not_resp_yet : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      br_address : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      br_instr : std_logic_vector(31 downto 0);
      instr_buf : std_logic_vector(DBG_FETCH_TRACE_SIZE*64-1 downto 0);
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_pipeline_hold, i_mem_req_ready, i_mem_data_valid,
                i_mem_data_addr, i_mem_data, i_mem_load_fault, i_e_npc,
                i_predict_npc, i_predict, i_br_fetch_valid, i_br_address_fetch,
                i_br_instr_fetch, r)
    variable v : RegistersType;
    variable wb_o_addr_req : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable w_predict_miss : std_logic;
    variable w_o_req_valid : std_logic;
    variable w_o_req_fire : std_logic;
    variable w_resp_fire : std_logic;
    variable w_o_hold : std_logic;
    variable w_o_mem_resp_ready : std_logic;
    variable wb_o_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_instr : std_logic_vector(31 downto 0);
  begin

    v := r;

    w_o_req_valid := i_nrst and not i_pipeline_hold
        and not (r.wait_resp and not i_mem_data_valid);
    w_o_req_fire := w_o_req_valid and i_mem_req_ready;

    w_o_mem_resp_ready := not i_pipeline_hold;
    w_resp_fire := i_mem_data_valid and w_o_mem_resp_ready;

    w_predict_miss := '1';
    if (i_e_npc = r.pc_z1)
        or (i_e_npc = r.raddr_not_resp_yet) then
      w_predict_miss := '0';
    end if;

    if w_predict_miss = '1' then
        wb_o_addr_req := i_e_npc;
    else
        wb_o_addr_req := i_predict_npc;
    end if;

    -- Debug last fetched instructions buffer:
    if w_o_req_fire = '1' then
        v.instr_buf(DBG_FETCH_TRACE_SIZE*64-1 downto 64) := 
             r.instr_buf((DBG_FETCH_TRACE_SIZE-1)*64-1 downto 0);
        if w_resp_fire = '1' then
            v.instr_buf(95 downto 64) := i_mem_data;
        end if;
        v.instr_buf(63 downto 32) := wb_o_addr_req;
        v.instr_buf(31 downto 0) := (others => '0');
    elsif w_resp_fire = '1' then
        v.instr_buf(31 downto 0) := i_mem_data;
    end if;


    if w_o_req_fire = '1'then
      v.wait_resp := '1';
      v.pc_z1 := r.raddr_not_resp_yet;
      v.raddr_not_resp_yet := wb_o_addr_req; -- Address already requested but probably not responded yet.
                                             -- Avoid marking such request as 'miss'.
      v.pipeline_init := r.pipeline_init(3 downto 0) & '1';
    elsif (i_mem_data_valid and not w_o_req_fire and not i_pipeline_hold) = '1' then
      v.wait_resp := '0';
    end if;

    if i_br_fetch_valid = '1' then
        v.br_address := i_br_address_fetch;
        v.br_instr := i_br_instr_fetch;
    end if;
 
    if i_mem_data_addr = r.br_address then
        wb_o_pc := r.br_address;
        wb_o_instr := r.br_instr;
        if w_resp_fire = '1' then
            v.br_address := (others => '1');
        end if;
    else
        wb_o_pc := i_mem_data_addr;
        wb_o_instr := i_mem_data;
    end if;
    
    if i_nrst = '0' then
        v.wait_resp := '0';
        v.pipeline_init := (others => '0');
        v.pc_z1 := (others => '0');
        v.raddr_not_resp_yet := (others => '0');
        v.br_address := (others => '1');
        v.br_instr := (others => '0');
        v.instr_buf := (others => '0');
    end if;

    o_mem_addr_valid <= w_o_req_valid;
    o_mem_addr <= wb_o_addr_req;
    o_mem_req_fire <= w_o_req_fire;
    o_ex_load_fault <= '0';    -- TODO
    o_valid <= w_resp_fire;
    o_pc <= wb_o_pc;
    o_instr <= wb_o_instr;
    o_predict_miss <= w_predict_miss;
    o_mem_resp_ready <= w_o_mem_resp_ready;
    o_hold <= not w_resp_fire;
    o_instr_buf <= r.instr_buf;
    
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
