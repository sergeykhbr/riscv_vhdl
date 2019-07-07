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


entity InstrFetch is generic (
    async_reset : boolean
  );
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
    i_minus2 : in std_logic;
    i_minus4 : in std_logic;

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

      resp_address : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      resp_data : std_logic_vector(31 downto 0);
      resp_valid : std_logic;
      resp_address_z : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      resp_data_z : std_logic_vector(31 downto 0);
      minus2 : std_logic;
      minus4 : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'), (others => '0'), (others => '0'),
    (others => '1'),  -- br_address
    (others =>'0'), (others =>'0'),
    (others =>'0'), (others =>'0'), '0', (others =>'0'), (others =>'0'), '0', '0'
  );

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_pipeline_hold, i_mem_req_ready, i_mem_data_valid,
                i_mem_data_addr, i_mem_data, i_mem_load_fault, i_e_npc,
                i_predict_npc, i_minus2, i_minus4,
                i_br_fetch_valid, i_br_address_fetch, i_br_instr_fetch, r)
    variable v : RegistersType;
    variable w_o_req_valid : std_logic;
    variable w_o_req_fire : std_logic;
    variable w_o_hold : std_logic;
    variable wb_o_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_instr : std_logic_vector(31 downto 0);
  begin

    v := r;

    w_o_req_valid := i_nrst and not i_pipeline_hold
        and not (r.wait_resp and not i_mem_data_valid);
    w_o_req_fire := i_mem_req_ready and w_o_req_valid;

    w_o_hold := not (r.wait_resp and i_mem_data_valid);

    if w_o_req_fire = '1' then
        v.wait_resp := '1';
    elsif i_mem_data_valid = '1' and i_pipeline_hold = '0' then
        v.wait_resp := '0';
    end if;

    if i_mem_data_valid = '1' and r.wait_resp = '1' and i_pipeline_hold = '0' then
        v.resp_valid := '1';
        v.minus2 := i_minus2;
        v.minus4 := i_minus4;
--        if i_mem_data_addr = r.br_address then
--            v.resp_address := r.br_address;
--            v.resp_data := r.br_instr;
--            v.br_address := (others => '1');
--        else
            v.resp_address := i_mem_data_addr;
            v.resp_data := i_mem_data;
--        end if;
        v.resp_address_z := r.resp_address;
        v.resp_data_z := r.resp_data;
    end if;

    if r.minus4 = '1' then
        wb_o_pc := r.resp_address_z;
        wb_o_instr := r.resp_data_z;
    elsif r.minus2 = '1' then
        wb_o_pc := r.resp_address - 2;
        wb_o_instr := r.resp_data(15 downto 0) & r.resp_data_z(31 downto 16);
    else
        wb_o_pc := r.resp_address;
        wb_o_instr := r.resp_data;
    end if;


    if i_br_fetch_valid = '1' then
        v.br_address := i_br_address_fetch;
        v.br_instr := i_br_instr_fetch;
    end if;

    -- Breakpoint skip logic that allows to continue execution
    -- without actual breakpoint remove only once 
    if wb_o_pc = r.br_address then
        wb_o_instr := r.br_instr;
        if i_mem_data_valid = '1' and r.wait_resp = '1' and i_pipeline_hold = '0' then
            v.br_address := (others => '1');
        end if;
    end if;

    
    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_mem_addr_valid <= w_o_req_valid;
    o_mem_addr <= i_predict_npc;
    o_mem_req_fire <= w_o_req_fire;
    o_ex_load_fault <= '0';    -- TODO
    o_valid <= r.resp_valid and not (i_pipeline_hold or w_o_hold);
    o_pc <= wb_o_pc;
    o_instr <= wb_o_instr;
    o_mem_resp_ready <= r.wait_resp and not i_pipeline_hold;
    o_hold <= w_o_hold;
    o_instr_buf <= r.instr_buf;
    
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
