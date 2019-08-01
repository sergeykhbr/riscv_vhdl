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


entity MemAccess is generic (
    async_reset : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_e_valid : in std_logic;                                         -- Execution stage outputs are valid
    i_e_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);          -- Execution stage instruction pointer
    i_e_instr : in std_logic_vector(31 downto 0);                     -- Execution stage instruction value

    i_res_addr : in std_logic_vector(5 downto 0);                     -- Register address to be written (0=no writing)
    i_res_data : in std_logic_vector(RISCV_ARCH-1 downto 0);          -- Register value to be written
    i_memop_sign_ext : in std_logic;                                  -- Load data with sign extending (if less than 8 Bytes)
    i_memop_load : in std_logic;                                      -- Load data from memory and write to i_res_addr
    i_memop_store : in std_logic;                                     -- Store i_res_data value into memory
    i_memop_size : in std_logic_vector(1 downto 0);                   -- Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    i_memop_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Memory access address
    o_wena : out std_logic;                                           -- Write enable signal
    o_waddr : out std_logic_vector(5 downto 0);                       -- Output register address (0 = x0 = no write)
    o_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);            -- Register value

    -- Memory interface:
    i_mem_req_ready : in std_logic;
    o_mem_valid : out std_logic;                                      -- Memory request is valid
    o_mem_write : out std_logic;                                      -- Memory write request
    o_mem_sz : out std_logic_vector(1 downto 0);                      -- Encoded data size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);     -- Data path requested address
    o_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);     -- Data path requested data (write transaction)
    i_mem_data_valid : in std_logic;                                  -- Data path memory response is valid
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Data path memory response address
    i_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);      -- Data path memory response value
    o_mem_resp_ready : out std_logic;

    o_hold : out std_logic;                                           -- Memory operation is more than 1 clock
    o_valid : out std_logic;                                          -- Output is valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);           -- Valid instruction pointer
    o_instr : out std_logic_vector(31 downto 0)                       -- Valid instruction value
  );
end; 
 
architecture arch_MemAccess of MemAccess is

  type RegistersType is record
      requested : std_logic;
      req_valid : std_logic;
      req_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_instr : std_logic_vector(31 downto 0);
      req_res_addr : std_logic_vector(5 downto 0);
      req_res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
      req_memop_sign_ext : std_logic;
      req_memop : std_logic;
      req_memop_store : std_logic;
      req_memop_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_memop_size : std_logic_vector(1 downto 0);
      req_wena : std_logic;
  end record;
 
  constant R_RESET : RegistersType := (
    '0', '0', (others => '0'),               -- requested, req_valid, req_pc,
    (others => '0'), (others => '0'),        -- req_instr, req_res_addr
    (others => '0'), '0', '0',               -- req_res_data, req_memop_sign_ext, req_memop
    '0', (others => '0'),                    -- req_memop_store, req_memop_addr
    (others => '0'), '0'                     -- req_memop_size, req_wena
  );

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_mem_req_ready, i_e_valid, i_e_pc, i_e_instr, i_res_addr, i_res_data,
                i_memop_sign_ext, i_memop_load, i_memop_store, i_memop_size,
                i_memop_addr, i_mem_data_valid, i_mem_data_addr, i_mem_data, r)
    variable v : RegistersType;
    variable w_hold_req : std_logic;
    variable w_hold_resp : std_logic;
    variable w_hold : std_logic;
    variable w_memop : std_logic;
    variable w_valid : std_logic;
    variable wb_res_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable w_o_mem_valid : std_logic;
    variable w_o_mem_write : std_logic;
    variable wb_o_mem_sz : std_logic_vector(1 downto 0);
    variable wb_o_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  begin

    v := r;

    w_memop := i_memop_load or i_memop_store;

    w_hold_req := not i_mem_req_ready and 
            ((i_e_valid and w_memop) or (not r.requested and r.req_valid));
    w_hold_resp := r.requested and r.req_valid
                and r.req_memop and not i_mem_data_valid;

    w_hold := w_hold_req or w_hold_resp;

    -- warning: exec stage forms valid signal only 1 clock from trigger and ignores
    --             hold signal on the same clock. NEED TO FIX IT IN Executor!!!
    if i_e_valid = '1' then
        v.requested := not w_hold;
        v.req_valid := '1';
        v.req_pc := i_e_pc;
        v.req_instr := i_e_instr;
        v.req_res_addr := i_res_addr;
        v.req_res_data := i_res_data;
        v.req_memop_sign_ext := i_memop_sign_ext;
        v.req_memop := w_memop;
        v.req_memop_store := i_memop_store;
        v.req_memop_addr := i_memop_addr;
        v.req_memop_size := i_memop_size;
        if i_res_addr = "000000" then
            v.req_wena := '0';
        else
            v.req_wena := '1';
        end if;
    elsif r.req_valid = '1' and w_hold = '0' then
        if r.requested = '0' then
            -- address was requested while bus not ready
            v.requested := '1';
        else
            v.requested := '0';
            v.req_valid := '0';
            v.req_wena := '0';
        end if;
    end if;
    
    if r.req_memop = '1' then
        if r.req_memop_sign_ext = '1' then
            case r.req_memop_size is
            when MEMOP_1B =>
                wb_res_wdata := i_mem_data;
                if i_mem_data(7) = '1' then
                    wb_res_wdata(63 downto 8) := (others => '1');
                end if;
            when MEMOP_2B =>
                wb_res_wdata := i_mem_data;
                if i_mem_data(15) = '1' then
                    wb_res_wdata(63 downto 16) := (others => '1');
                end if;
            when MEMOP_4B =>
                wb_res_wdata := i_mem_data;
                if i_mem_data(31) = '1' then
                    wb_res_wdata(63 downto 32) := (others => '1');
                end if;
            when others =>
                wb_res_wdata := i_mem_data;
            end case;
        else
            wb_res_wdata := i_mem_data;
        end if;
    else
        wb_res_wdata := r.req_res_data;
    end if;

    if r.requested = '0' and r.req_valid = '1' then
        -- This branch should be deleted after proper HOLD implementation in Executor!!!
        w_o_mem_valid := '1';
        w_o_mem_write := r.req_memop_store;
        wb_o_mem_sz := r.req_memop_size;
        wb_o_mem_addr := r.req_memop_addr;
        wb_o_mem_data := r.req_res_data;
    else
        w_o_mem_valid := i_e_valid and w_memop;
        w_o_mem_write := i_memop_store;
        wb_o_mem_sz := i_memop_size;
        wb_o_mem_addr := i_memop_addr;
        wb_o_mem_data := i_res_data;
    end if;
    w_valid := r.requested and r.req_valid and not w_hold_resp;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_mem_resp_ready <= '1';

    o_mem_valid <= w_o_mem_valid;
    o_mem_write <= w_o_mem_write;
    o_mem_sz <= wb_o_mem_sz;
    o_mem_addr <= wb_o_mem_addr;
    o_mem_data <= wb_o_mem_data;

    o_wena <= r.req_wena;
    o_waddr <= r.req_res_addr;
    o_wdata <= wb_res_wdata;
    o_valid <= w_valid;
    o_pc <= r.req_pc;
    o_instr <= r.req_instr;
    o_hold <= w_hold;
    
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
