--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
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
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity CacheTop is generic (
    memtech : integer;
    async_reset : boolean
  );
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
    o_resp_ctrl_load_fault : out std_logic;
    i_resp_ctrl_ready : in std_logic;
    -- Data path:
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_size : in std_logic_vector(1 downto 0);
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
    i_req_mem_ready : in std_logic;                                    -- AXI request was accepted
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);  -- burst transaction length
    o_req_mem_len : out std_logic_vector(7 downto 0);                  -- burst length
    o_req_mem_burst : out std_logic_vector(1 downto 0);                -- burst type: "00" FIX; "01" INCR; "10" WRAP
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_load_fault : in std_logic;                             -- Bus response with SLVERR or DECERR on read
    i_resp_mem_store_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on write
    i_resp_mem_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -- Debug signals:
    i_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- clear ICache address from debug interface
    i_flush_valid : in std_logic;                                      -- address to clear icache is valid
    o_istate : out std_logic_vector(1 downto 0);                      -- ICache state machine value
    o_dstate : out std_logic_vector(1 downto 0);                      -- DCache state machine value
    o_cstate : out std_logic_vector(1 downto 0)                       -- cachetop state machine value
  );
end; 
 
architecture arch_CacheTop of CacheTop is
  constant State_Idle : std_logic_vector(1 downto 0) := "00";
  constant State_IMem : std_logic_vector(1 downto 0) := "01";
  constant State_DMem : std_logic_vector(1 downto 0) := "10";

  type CacheOutputType is record
      req_mem_valid : std_logic;
      req_mem_write : std_logic;
      req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_mem_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
      req_mem_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      req_mem_len : std_logic_vector(7 downto 0);
      req_mem_burst : std_logic_vector(1 downto 0);
      req_mem_last : std_logic;
  end record;

  type RegistersType is record
      state : std_logic_vector(1 downto 0);
  end record;

  signal i :  CacheOutputType;
  signal d :  CacheOutputType;
  signal r, rin : RegistersType;
  -- Memory Control interface:
  signal w_ctrl_resp_mem_data_valid : std_logic;
  signal wb_ctrl_resp_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal w_ctrl_resp_mem_load_fault : std_logic;
  signal w_ctrl_req_ready : std_logic;
  -- Memory Data interface:
  signal w_data_resp_mem_data_valid : std_logic;
  signal wb_data_resp_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal w_data_resp_mem_load_fault : std_logic;
  signal w_data_req_ready : std_logic;

  component ICacheLru is generic (
    memtech : integer;
    async_reset : boolean;
    index_width : integer
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    o_resp_ctrl_load_fault : out std_logic;
    i_resp_ctrl_ready : in std_logic;
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
    i_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_flush_valid : in std_logic;
    o_istate : out std_logic_vector(1 downto 0)
  );
  end component; 

  component DCache is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
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
    o_dstate : out std_logic_vector(1 downto 0)
  );
  end component; 

begin

    i0 : ICacheLru generic map (
        memtech => memtech,
        async_reset => async_reset,
        index_width => CFG_IINDEX_WIDTH
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_ctrl_valid => i_req_ctrl_valid,
        i_req_ctrl_addr => i_req_ctrl_addr,
        o_req_ctrl_ready => o_req_ctrl_ready,
        o_resp_ctrl_valid => o_resp_ctrl_valid,
        o_resp_ctrl_addr => o_resp_ctrl_addr,
        o_resp_ctrl_data => o_resp_ctrl_data,
        o_resp_ctrl_load_fault => o_resp_ctrl_load_fault,
        i_resp_ctrl_ready => i_resp_ctrl_ready,
        i_req_mem_ready => w_ctrl_req_ready,
        o_req_mem_valid => i.req_mem_valid,
        o_req_mem_write => i.req_mem_write,
        o_req_mem_addr => i.req_mem_addr,
        o_req_mem_strob => i.req_mem_strob,
        o_req_mem_data => i.req_mem_wdata,
        o_req_mem_len => i.req_mem_len,
        o_req_mem_burst => i.req_mem_burst,
        o_req_mem_last => i.req_mem_last,
        i_resp_mem_data_valid => w_ctrl_resp_mem_data_valid,
        i_resp_mem_data => wb_ctrl_resp_mem_data,
        i_resp_mem_load_fault => w_ctrl_resp_mem_load_fault,
        i_flush_address => i_flush_address,
        i_flush_valid => i_flush_valid,
        o_istate => o_istate);

    d0 : DCache generic map (
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_data_valid => i_req_data_valid,
        i_req_data_write => i_req_data_write,
        i_req_data_sz => i_req_data_size,
        i_req_data_addr => i_req_data_addr,
        i_req_data_data => i_req_data_data,
        o_req_data_ready => o_req_data_ready,
        o_resp_data_valid => o_resp_data_valid,
        o_resp_data_addr => o_resp_data_addr,
        o_resp_data_data => o_resp_data_data,
        o_resp_data_load_fault => o_resp_data_load_fault,
        o_resp_data_store_fault => o_resp_data_store_fault,
        o_resp_data_store_fault_addr => o_resp_data_store_fault_addr,
        i_resp_data_ready => i_resp_data_ready,
        i_req_mem_ready => w_data_req_ready,
        o_req_mem_valid => d.req_mem_valid,
        o_req_mem_write => d.req_mem_write,
        o_req_mem_addr => d.req_mem_addr,
        o_req_mem_strob => d.req_mem_strob,
        o_req_mem_data => d.req_mem_wdata,
        o_req_mem_len => d.req_mem_len,
        o_req_mem_burst => d.req_mem_burst,
        o_req_mem_last => d.req_mem_last,
        i_resp_mem_data_valid => w_data_resp_mem_data_valid,
        i_resp_mem_data => wb_data_resp_mem_data,
        i_resp_mem_load_fault => w_data_resp_mem_load_fault,
        i_resp_mem_store_fault => i_resp_mem_store_fault,
        i_resp_mem_store_fault_addr => i_resp_mem_store_fault_addr,
        o_dstate => o_dstate);

  comb : process(i_nrst, i_req_mem_ready, i_resp_mem_data_valid, i_resp_mem_data,
                 i_resp_mem_load_fault, i, d, r)
    variable v : RegistersType;
    variable w_req_mem_valid : std_logic;
    variable wb_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_mem_len : std_logic_vector(7 downto 0);
    variable wb_mem_burst : std_logic_vector(1 downto 0);
    variable w_mem_write : std_logic;
    variable v_data_req_ready : std_logic;
    variable v_data_resp_mem_data_valid : std_logic;
    variable v_ctrl_req_ready : std_logic;
    variable v_ctrl_resp_mem_data_valid : std_logic;
  begin

    v := r;

    -- default is data path
    w_req_mem_valid := '0';
    wb_mem_addr := d.req_mem_addr;
    w_mem_write := d.req_mem_write;
    wb_mem_len := d.req_mem_len;
    wb_mem_burst := d.req_mem_burst;
    v_data_req_ready := '0';
    v_data_resp_mem_data_valid := '0';
    v_ctrl_req_ready := '0';
    v_ctrl_resp_mem_data_valid := '0';
   
    case r.state is
    when State_Idle =>
        w_req_mem_valid := i.req_mem_valid or d.req_mem_valid;
        if d.req_mem_valid = '1' then
            v_data_req_ready := i_req_mem_ready;
            if i_req_mem_ready = '1' then
                v.state := State_DMem;
            end if;
        elsif i.req_mem_valid = '1' then
            v_ctrl_req_ready := i_req_mem_ready;
            wb_mem_addr := i.req_mem_addr;
            wb_mem_len := i.req_mem_len;
            wb_mem_burst := i.req_mem_burst;
            w_mem_write := i.req_mem_write;
            if i_req_mem_ready = '1' then
                v.state := State_IMem;
            end if;
        end if;

    when State_DMem =>
        w_req_mem_valid := d.req_mem_last and (i.req_mem_valid or d.req_mem_valid);
        if i_resp_mem_data_valid = '1' and d.req_mem_last = '1' then
            if d.req_mem_valid = '1' then
                v_data_req_ready := i_req_mem_ready;
                if i_req_mem_ready = '1' then
                     v.state := State_DMem;
                else
                    v.state := State_Idle;
                end if;
            elsif i.req_mem_valid = '1' then
                v_ctrl_req_ready := i_req_mem_ready;
                wb_mem_addr := i.req_mem_addr;
                wb_mem_len := i.req_mem_len;
                wb_mem_burst := i.req_mem_burst;
                w_mem_write := i.req_mem_write;
                if i_req_mem_ready = '1' then
                    v.state := State_IMem;
                else
                    v.state := State_Idle;
                end if;
            else
                v.state := State_Idle;
            end if;
        end if;
        v_data_resp_mem_data_valid := i_resp_mem_data_valid;
        
    when State_IMem =>
        w_req_mem_valid := i.req_mem_last and (i.req_mem_valid or d.req_mem_valid);
        if i_resp_mem_data_valid = '1' and i.req_mem_last = '1' then
            if d.req_mem_valid = '1' then
                v_data_req_ready := i_req_mem_ready;
                if i_req_mem_ready = '1' then
                    v.state := State_DMem;
                else
                    v.state := State_Idle;
                end if;
            elsif i.req_mem_valid = '1' then
                v_ctrl_req_ready := i_req_mem_ready;
                wb_mem_addr := i.req_mem_addr;
                wb_mem_len := i.req_mem_len;
                wb_mem_burst := i.req_mem_burst;
                w_mem_write := i.req_mem_write;
                if i_req_mem_ready = '1' then
                    v.state := State_IMem;
                else
                    v.state := State_Idle;
                end if;
            else
                v.state := State_Idle;
            end if;
        end if;
        v_ctrl_resp_mem_data_valid := i_resp_mem_data_valid;
        
    when others =>
    end case;

    if not async_reset and i_nrst = '0' then
        v.state := State_Idle;
    end if;

    w_data_req_ready <= v_data_req_ready;
    w_data_resp_mem_data_valid <= v_data_resp_mem_data_valid;
    wb_data_resp_mem_data <= i_resp_mem_data;
    w_data_resp_mem_load_fault <= i_resp_mem_load_fault;

    w_ctrl_req_ready <= v_ctrl_req_ready;
    w_ctrl_resp_mem_data_valid <= v_ctrl_resp_mem_data_valid;
    wb_ctrl_resp_mem_data <= i_resp_mem_data;
    w_ctrl_resp_mem_load_fault <= i_resp_mem_load_fault;

    o_req_mem_valid <= w_req_mem_valid;
    o_req_mem_addr <= wb_mem_addr;
    o_req_mem_len <= wb_mem_len;
    o_req_mem_burst <= wb_mem_burst;
    o_req_mem_write <= w_mem_write;
    o_req_mem_strob <= d.req_mem_strob;
    o_req_mem_data <= d.req_mem_wdata;
    o_cstate <= r.state;
    
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r.state <= State_Idle;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
