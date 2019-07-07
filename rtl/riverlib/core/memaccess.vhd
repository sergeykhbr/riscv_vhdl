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
      valid : std_logic;
      pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      instr : std_logic_vector(31 downto 0);

      wena : std_logic;
      waddr : std_logic_vector(5 downto 0);
      sign_ext : std_logic;
      size : std_logic_vector(1 downto 0);
      wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      wait_req : std_logic;
      wait_req_write : std_logic;
      wait_req_sz : std_logic_vector(1 downto 0);
      wait_req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      wait_req_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      wait_resp : std_logic;
  end record;
 
  constant R_RESET : RegistersType := (
    '0', (others => '0'), (others => '0'),   -- valid, pc, instr
    '0', (others => '0'), '0',               -- wena, waddr, sign_ext
    "00", (others => '0'), '0',              -- size, wdata, wait_req
    '0', "00", (others => '0'),              -- wait_req_write, wait_req_sz, wait_req_addr
    (others => '0'), '0'                     -- wait_req_wdata, wait_resp
  );

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_mem_req_ready, i_e_valid, i_e_pc, i_e_instr, i_res_addr, i_res_data,
                i_memop_sign_ext, i_memop_load, i_memop_store, i_memop_size,
                i_memop_addr, i_mem_data_valid, i_mem_data_addr, i_mem_data, r)
    variable v : RegistersType;
    variable w_req_fire : std_logic;
    variable w_o_mem_valid : std_logic;
    variable w_o_mem_write : std_logic;
    variable wb_o_mem_sz : std_logic_vector(1 downto 0);
    variable wb_o_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_mem_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_res_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable w_memop : std_logic;
    variable w_o_valid : std_logic;
    variable w_o_wena : std_logic;
    variable w_o_hold : std_logic;
    variable w_mem_fire : std_logic;
  begin

    v := r;

    w_o_mem_valid := '0';
    w_o_mem_write := '0';
    wb_o_mem_sz := (others => '0');
    wb_o_mem_addr := (others => '0');
    wb_o_mem_wdata := (others => '0');
    v.valid := '0';
    w_o_hold := '0';
    
    w_memop := i_memop_load or i_memop_store;

    if r.wait_req = '1' then
        if i_mem_req_ready = '1' then
            v.wait_req := '0';
            v.wait_resp := '1';
        end if;
        w_o_mem_valid := '1';
        w_o_mem_write := r.wait_req_write;
        wb_o_mem_sz := r.wait_req_sz;
        wb_o_mem_addr := r.wait_req_addr;
        wb_o_mem_wdata := r.wait_req_wdata;
    elsif i_e_valid = '1' then
      v.valid := not w_memop;
      v.pc := i_e_pc;
      v.instr := i_e_instr;
      v.waddr := i_res_addr;
      v.wdata := i_res_data;
      if i_res_addr = "000000" then
          v.wena := '0';
      else
          v.wena := '1';
      end if;

      if w_memop = '1' then
        w_o_mem_valid := '1';
        w_o_mem_write := i_memop_store;
        wb_o_mem_sz := i_memop_size;
        wb_o_mem_addr := i_memop_addr;
        wb_o_mem_wdata := i_res_data;
        v.sign_ext := i_memop_sign_ext;
        v.size := i_memop_size;

        v.wait_resp := i_mem_req_ready;
        v.wait_req := not i_mem_req_ready;
        v.wait_req_write := i_memop_store;
        v.wait_req_sz := i_memop_size;
        v.wait_req_addr := i_memop_addr;
        v.wait_req_wdata := i_res_data;
      else
        w_o_mem_valid := '0';
        w_o_mem_write := '0';
        wb_o_mem_sz := (others => '0');
        wb_o_mem_addr := (others => '0');
        wb_o_mem_wdata := (others => '0');
        v.sign_ext := '0';
        v.size := (others => '0');
        v.wait_req_addr := (others => '0');
        v.wait_req := '0';
        v.wait_resp := '0';
      end if;
    elsif i_mem_data_valid = '1' then
      v.wait_resp := '0';
    end if;

    w_o_hold := (i_e_valid and w_memop) or r.wait_req 
            or (r.wait_resp and not i_mem_data_valid);

    w_mem_fire := i_mem_data_valid;
    
    if w_mem_fire = '1' then
        if r.sign_ext = '1' then
            case r.size is
            when MEMOP_1B =>
                wb_res_wdata := i_mem_data;
                wb_res_wdata(63 downto 8) := (others => i_mem_data(7));
            when MEMOP_2B =>
                wb_res_wdata := i_mem_data;
                wb_res_wdata(63 downto 16) := (others => i_mem_data(15));
            when MEMOP_4B =>
                wb_res_wdata := i_mem_data;
                wb_res_wdata(63 downto 32) := (others => i_mem_data(31));
            when others =>
                wb_res_wdata := i_mem_data;
            end case;
        else
            wb_res_wdata := i_mem_data;
        end if;
    else
        wb_res_wdata := r.wdata;
    end if;

    w_o_valid := r.valid or w_mem_fire;
    w_o_wena := r.wena and w_o_valid;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_mem_resp_ready <= '1';

    o_mem_valid <= w_o_mem_valid;
    o_mem_write <= w_o_mem_write;
    o_mem_sz <= wb_o_mem_sz;
    o_mem_addr <= wb_o_mem_addr;
    o_mem_data <= wb_o_mem_wdata;

    o_wena <= w_o_wena;
    o_waddr <= r.waddr;
    o_wdata <= wb_res_wdata;
    o_valid <= w_o_valid;
    o_pc <= r.pc;
    o_instr <= r.instr;
    o_hold <= w_o_hold;
    
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
