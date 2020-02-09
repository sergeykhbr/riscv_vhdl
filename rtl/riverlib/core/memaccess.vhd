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
use ieee.std_logic_misc.all;  -- or_reduce()
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
    i_e_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);          -- Execution stage instruction pointer
    i_e_instr : in std_logic_vector(31 downto 0);                     -- Execution stage instruction value
    i_e_flushd : in std_logic;
    o_flushd : out std_logic;

    i_memop_waddr : in std_logic_vector(5 downto 0);                  -- Register address to be written (0=no writing)
    i_memop_wtag : in std_logic_vector(3 downto 0);
    i_memop_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);       -- Register value to be written
    i_memop_sign_ext : in std_logic;                                  -- Load data with sign extending (if less than 8 Bytes)
    i_memop_load : in std_logic;                                      -- Load data from memory and write to i_res_addr
    i_memop_store : in std_logic;                                     -- Store i_res_data value into memory
    i_memop_size : in std_logic_vector(1 downto 0);                   -- Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    i_memop_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);    -- Memory access address
    o_memop_ready : out std_logic;                                    -- Ready to accept memop request
    o_wb_wena : out std_logic;                                        -- Write enable signal
    o_wb_waddr : out std_logic_vector(5 downto 0);                    -- Output register address (0 = x0 = no write)
    o_wb_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);         -- Register value
    o_wb_wtag : out std_logic_vector(3 downto 0);
    i_wb_ready : in std_logic;

    -- Memory interface:
    i_mem_req_ready : in std_logic;
    o_mem_valid : out std_logic;                                      -- Memory request is valid
    o_mem_write : out std_logic;                                      -- Memory write request
    o_mem_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);     -- Data path requested address
    o_mem_wdata : out std_logic_vector(63 downto 0);                  -- Data path requested data (write transaction)
    o_mem_wstrb : out std_logic_vector(7 downto 0);                   -- 8-bytes aligned strobs
    i_mem_data_valid : in std_logic;                                  -- Data path memory response is valid
    i_mem_data_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- Data path memory response address
    i_mem_data : in std_logic_vector(63 downto 0);                    -- Data path memory response value
    o_mem_resp_ready : out std_logic
  );
end; 
 
architecture arch_MemAccess of MemAccess is

  constant State_Idle : std_logic_vector(1 downto 0) := "00";
  constant State_WaitReqAccept : std_logic_vector(1 downto 0) := "01";
  constant State_WaitResponse : std_logic_vector(1 downto 0) := "10";
  constant State_Hold : std_logic_vector(1 downto 0) := "11";
 
  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type RegistersType is record
      state : std_logic_vector(1 downto 0);
      memop_w : std_logic;
      memop_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      memop_wdata : std_logic_vector(63 downto 0);
      memop_wstrb : std_logic_vector(7 downto 0);
      memop_sign_ext : std_logic;
      memop_size : std_logic_vector(1 downto 0);
      
      memop_res_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      memop_res_instr : std_logic_vector(31 downto 0);
      memop_res_addr : std_logic_vector(5 downto 0);
      memop_res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
      memop_res_wena : std_logic;
      memop_wtag : std_logic_vector(3 downto 0);

      hold_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
  end record;
 
  constant R_RESET : RegistersType := (
    State_Idle,                              -- state
    '0', (others => '0'),                    -- memop_w, memop_addr
    (others => '0'), (others => '0'),        -- memop_wdata, memop_wstrb
    '0', (others => '0'),                    -- memop_sign_ext, memop_size
    (others => '0'), (others => '0'),        -- memop_res_pc, memop_res_instr
    (others => '0'),                         -- memop_res_addr
    (others => '0'), '0',                    -- memop_res_data, memop_res_wena
    (others => '0'),                         -- memop_wtag
    (others => '0')                          -- hold_rdata
  );

  signal r, rin : RegistersType;

  -- TODO: move into separate module
  -- queue signals before move into separate module
  constant QUEUE_WIDTH : integer := 1   -- i_e_flushd
                                  + 4   -- wtag
                                  + 64  -- wdata width
                                  + 8   -- wdata btyes
                                  + RISCV_ARCH 
                                  + 6
                                  + 32
                                  + CFG_CPU_ADDR_BITS
                                  + 2
                                  + 1
                                  + 1
                                  + CFG_CPU_ADDR_BITS
                                  ;

  signal queue_we : std_logic;
  signal queue_re : std_logic;
  signal queue_data_i : std_logic_vector(QUEUE_WIDTH-1 downto 0);
  signal queue_data_o : std_logic_vector(QUEUE_WIDTH-1 downto 0);
  signal queue_nempty : std_logic;
  signal queue_full : std_logic;

begin

  queue0 : Queue generic map (
    async_reset => async_reset,
    szbits => 2,
    dbits => QUEUE_WIDTH
  ) port map (
    i_clk => i_clk,
    i_nrst => i_nrst,
    i_re => queue_re,
    i_we => queue_we,
    i_wdata => queue_data_i,
    o_rdata => queue_data_o,
    o_full => queue_full,
    o_nempty => queue_nempty
  );

 
  comb : process(i_nrst, i_e_valid, i_e_pc, i_e_instr, i_memop_waddr, i_memop_wtag, i_memop_wdata,
                 i_memop_sign_ext, i_memop_load, i_memop_store, i_memop_size, i_memop_addr,
                 i_wb_ready, i_mem_data_addr, i_mem_req_ready, i_mem_data_valid,
                 i_mem_data_addr, i_mem_data, i_e_flushd,
                 queue_data_o, queue_nempty, queue_full, r)
    variable v : RegistersType;
    variable vb_memop_wdata : std_logic_vector(63 downto 0);
    variable vb_memop_wstrb : std_logic_vector(7 downto 0);
    variable v_mem_valid : std_logic;
    variable v_mem_write : std_logic;
    variable v_mem_sign_ext : std_logic;
    variable vb_mem_sz : std_logic_vector(1 downto 0);
    variable vb_mem_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_mem_rdata : std_logic_vector(63 downto 0);
    variable v_queue_re : std_logic;
    variable v_flushd : std_logic;
    variable vb_mem_wtag : std_logic_vector(3 downto 0);
    variable vb_mem_wdata : std_logic_vector(63 downto 0);
    variable vb_mem_wstrb : std_logic_vector(7 downto 0);
    variable vb_mem_resp_shifted : std_logic_vector(63 downto 0);
    variable vb_mem_data_unsigned : std_logic_vector(63 downto 0);
    variable vb_mem_data_signed : std_logic_vector(63 downto 0);
    variable vb_res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_res_addr : std_logic_vector(5 downto 0);
    variable vb_e_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_e_instr : std_logic_vector(31 downto 0);
    variable v_memop_ready : std_logic;
    variable v_o_wena : std_logic;
    variable vb_o_waddr : std_logic_vector(5 downto 0);
    variable vb_o_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_o_wtag : std_logic_vector(3 downto 0);
  begin

    v := r;

    v_mem_valid := '0';
    v_queue_re := '0';

    vb_mem_resp_shifted := (others => '0');
    vb_mem_data_unsigned := (others => '0');
    vb_mem_data_signed := (others => '0');

    vb_memop_wdata := (others => '0');
    vb_memop_wstrb := (others => '0');

    v_o_wena := '0';
    vb_o_waddr := (others => '0');
    vb_o_wdata := (others => '0');
    vb_o_wtag := (others => '0');


    case i_memop_size is
    when "00" =>
        vb_memop_wdata := i_memop_wdata(7 downto 0) & i_memop_wdata(7 downto 0)
                        & i_memop_wdata(7 downto 0) & i_memop_wdata(7 downto 0)
                        & i_memop_wdata(7 downto 0) & i_memop_wdata(7 downto 0)
                        & i_memop_wdata(7 downto 0) & i_memop_wdata(7 downto 0);
        if i_memop_addr(2 downto 0) = "000" then
            vb_memop_wstrb := X"01";
        elsif i_memop_addr(2 downto 0) = "001" then
            vb_memop_wstrb := X"02";
        elsif i_memop_addr(2 downto 0) = "010" then
            vb_memop_wstrb := X"04";
        elsif i_memop_addr(2 downto 0) = "011" then
            vb_memop_wstrb := X"08";
        elsif i_memop_addr(2 downto 0) = "100" then
            vb_memop_wstrb := X"10";
        elsif i_memop_addr(2 downto 0) = "101" then
            vb_memop_wstrb := X"20";
        elsif i_memop_addr(2 downto 0) = "110" then
            vb_memop_wstrb := X"40";
        elsif i_memop_addr(2 downto 0) = "111" then
            vb_memop_wstrb := X"80";
        end if;
    when "01" =>
        vb_memop_wdata := i_memop_wdata(15 downto 0) & i_memop_wdata(15 downto 0)
                        & i_memop_wdata(15 downto 0) & i_memop_wdata(15 downto 0);
        if i_memop_addr(2 downto 1) = "00" then
            vb_memop_wstrb := X"03";
        elsif i_memop_addr(2 downto 1) = "01" then
            vb_memop_wstrb := X"0C";
        elsif i_memop_addr(2 downto 1) = "10" then
            vb_memop_wstrb := X"30";
        else
            vb_memop_wstrb := X"C0";
        end if;
    when "10" =>
        vb_memop_wdata := i_memop_wdata(31 downto 0) & i_memop_wdata(31 downto 0);
        if i_memop_addr(2) = '1' then
            vb_memop_wstrb := X"F0";
        else
            vb_memop_wstrb := X"0F";
        end if;
    when "11" =>
        vb_memop_wdata := i_memop_wdata;
        vb_memop_wstrb := X"FF";
    when others =>
    end case;

    -- Form Queue inputs:
    queue_data_i <= i_e_flushd & i_memop_wtag & vb_memop_wdata & vb_memop_wstrb &
                    i_memop_wdata & i_memop_waddr & i_e_instr & i_e_pc &
                    i_memop_size & i_memop_sign_ext & i_memop_store &
                    i_memop_addr;
    queue_we <= i_e_valid and (i_memop_load or i_memop_store or i_e_flushd);

    -- Split Queue outputs:
    v_flushd := queue_data_o(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+46);
    vb_mem_wtag := queue_data_o(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+45 downto
                                2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+42);
    vb_mem_wdata := queue_data_o(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+64+42-1 downto
                                 2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+42);
    vb_mem_wstrb := queue_data_o(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+8+42-1 downto
                                      2*CFG_CPU_ADDR_BITS+RISCV_ARCH+42);
    vb_res_data := queue_data_o(2*CFG_CPU_ADDR_BITS+RISCV_ARCH+42-1 downto
                                      2*CFG_CPU_ADDR_BITS+42);
    vb_res_addr := queue_data_o(2*CFG_CPU_ADDR_BITS+42-1 downto
                                2*CFG_CPU_ADDR_BITS+36);
    vb_e_instr := queue_data_o(2*CFG_CPU_ADDR_BITS+36-1 downto
                               2*CFG_CPU_ADDR_BITS+4);
    vb_e_pc := queue_data_o(2*CFG_CPU_ADDR_BITS+4-1 downto CFG_CPU_ADDR_BITS+4);
    vb_mem_sz := queue_data_o(CFG_CPU_ADDR_BITS+3 downto CFG_CPU_ADDR_BITS+2);
    v_mem_sign_ext := queue_data_o(CFG_CPU_ADDR_BITS+1);
    v_mem_write := queue_data_o(CFG_CPU_ADDR_BITS);
    vb_mem_addr := queue_data_o(CFG_CPU_ADDR_BITS-1 downto 0);

    case r.memop_addr(2 downto 0) is
    when "001" => vb_mem_resp_shifted := zero64(7 downto 0) & i_mem_data(63 downto 8);
    when "010" => vb_mem_resp_shifted := zero64(15 downto 0) & i_mem_data(63 downto 16);
    when "011" => vb_mem_resp_shifted := zero64(23 downto 0) & i_mem_data(63 downto 24);
    when "100" => vb_mem_resp_shifted := zero64(31 downto 0) & i_mem_data(63 downto 32);
    when "101" => vb_mem_resp_shifted := zero64(39 downto 0) & i_mem_data(63 downto 40);
    when "110" => vb_mem_resp_shifted := zero64(47 downto 0) & i_mem_data(63 downto 48);
    when "111" => vb_mem_resp_shifted := zero64(55 downto 0) & i_mem_data(63 downto 56);
    when others => vb_mem_resp_shifted := i_mem_data;
    end case;

    case r.memop_size is
    when MEMOP_1B =>
        vb_mem_data_unsigned(7 downto 0) := vb_mem_resp_shifted(7 downto 0);
        vb_mem_data_signed(7 downto 0) := vb_mem_resp_shifted(7 downto 0);
        vb_mem_data_signed(63 downto 8) := (others => vb_mem_resp_shifted(7));
    when MEMOP_2B =>
        vb_mem_data_unsigned(15 downto 0) := vb_mem_resp_shifted(15 downto 0);
        vb_mem_data_signed(15 downto 0) := vb_mem_resp_shifted(15 downto 0);
        vb_mem_data_signed(63 downto 16) := (others => vb_mem_resp_shifted(15));
    when MEMOP_4B =>
        vb_mem_data_unsigned(31 downto 0) := vb_mem_resp_shifted(31 downto 0);
        vb_mem_data_signed(31 downto 0) := vb_mem_resp_shifted(31 downto 0);
        vb_mem_data_signed(63 downto 32) := (others => vb_mem_resp_shifted(31));
    when others =>
        vb_mem_data_unsigned := vb_mem_resp_shifted;
        vb_mem_data_signed := vb_mem_resp_shifted;
    end case;

    if r.memop_w = '0' then
        if r.memop_sign_ext = '1' then
            vb_mem_rdata := vb_mem_data_signed;
        else
            vb_mem_rdata := vb_mem_data_unsigned;
        end if;
    else
        vb_mem_rdata := r.memop_res_data;
    end if;


    case r.state is
    when State_Idle =>
        v_queue_re := '1';
        if queue_nempty = '1' then
            v_mem_valid := not v_flushd;
            v.memop_res_pc := vb_e_pc;
            v.memop_res_instr := vb_e_instr;
            v.memop_res_addr := vb_res_addr;
            v.memop_res_data := vb_res_data;
            v.memop_res_wena := or_reduce(vb_res_addr);
            v.memop_addr := vb_mem_addr;
            v.memop_wdata := vb_mem_wdata;
            v.memop_wtag := vb_mem_wtag;
            v.memop_wstrb := vb_mem_wstrb;
            v.memop_w := v_mem_write;
            v.memop_sign_ext := v_mem_sign_ext;
            v.memop_size := vb_mem_sz;

            if v_flushd = '1' then
                -- do nothing
            elsif i_mem_req_ready = '1' then
                v.state := State_WaitResponse;
            else
                v.state := State_WaitReqAccept;
            end if;
        end if;
    when State_WaitReqAccept =>
        v_mem_valid := '1';
        v_mem_write := r.memop_w;
        vb_mem_sz := r.memop_size;
        vb_mem_addr := r.memop_addr;
        vb_mem_wdata := r.memop_wdata;
        vb_mem_wstrb := r.memop_wstrb;
        vb_res_data := r.memop_res_data;
        if i_mem_req_ready = '1' then
            v.state := State_WaitResponse;
        end if;
    when State_WaitResponse =>
        if i_mem_data_valid = '0' then
            -- Do nothing
        else
            v_o_wena := r.memop_res_wena;
            vb_o_waddr := r.memop_res_addr;
            vb_o_wdata := vb_mem_rdata;
            vb_o_wtag := r.memop_wtag;

            v_queue_re := '1';
            if r.memop_res_wena = '1' and i_wb_ready = '0' then
                -- Inject only one clock hold-on and wait a couple of clocks
                v_queue_re := '0';
                v.state := State_Hold;
                v.hold_rdata := vb_mem_rdata;
            elsif queue_nempty = '1' then
                v_mem_valid := not v_flushd;
                v.memop_res_pc := vb_e_pc;
                v.memop_res_instr := vb_e_instr;
                v.memop_res_addr := vb_res_addr;
                v.memop_res_data := vb_res_data;
                v.memop_res_wena := or_reduce(vb_res_addr);
                v.memop_addr := vb_mem_addr;
                v.memop_wdata := vb_mem_wdata;
                v.memop_wtag := vb_mem_wtag;
                v.memop_wstrb := vb_mem_wstrb;
                v.memop_w := v_mem_write;
                v.memop_sign_ext := v_mem_sign_ext;
                v.memop_size := vb_mem_sz;

                if v_flushd = '1' then
                    v.state := State_Idle;
                elsif i_mem_req_ready = '1' then
                    v.state := State_WaitResponse;
                else
                    v.state := State_WaitReqAccept;
                end if;
            else
                v.state := State_Idle;
            end if;
        end if;
    when State_Hold =>
        v_o_wena := r.memop_res_wena;
        vb_o_waddr := r.memop_res_addr;
        vb_o_wdata := r.hold_rdata;
        vb_o_wtag := r.memop_wtag;
        if i_wb_ready = '1' then
            v_queue_re := '1';
            if queue_nempty = '1' then
                v_mem_valid := not v_flushd;
                v.memop_res_pc := vb_e_pc;
                v.memop_res_instr := vb_e_instr;
                v.memop_res_addr := vb_res_addr;
                v.memop_res_data := vb_res_data;
                v.memop_res_wena := or_reduce(vb_res_addr);
                v.memop_addr := vb_mem_addr;
                v.memop_wdata := vb_mem_wdata;
                v.memop_wtag := vb_mem_wtag;
                v.memop_wstrb := vb_mem_wstrb;
                v.memop_w := v_mem_write;
                v.memop_sign_ext := v_mem_sign_ext;
                v.memop_size := vb_mem_sz;

                if v_flushd = '1' then
                    v.state := State_Idle;
                elsif i_mem_req_ready = '1' then
                    v.state := State_WaitResponse;
                else
                    v.state := State_WaitReqAccept;
                end if;
            else
                v.state := State_Idle;
            end if;
        end if;
    when others =>
    end case;

    v_memop_ready := '1';
    if queue_full = '1' then
        v_memop_ready := '0';
    end if;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    queue_re <= v_queue_re;

    o_flushd <= queue_nempty and v_flushd and v_queue_re;
    o_mem_resp_ready <= '1';

    o_mem_valid <= v_mem_valid;
    o_mem_write <= v_mem_write;
    o_mem_addr <= vb_mem_addr(CFG_CPU_ADDR_BITS-1 downto 3) & "000";
    o_mem_wdata <= vb_mem_wdata;
    o_mem_wstrb <= vb_mem_wstrb;

    o_memop_ready <= v_memop_ready;
    o_wb_wena <= v_o_wena;
    o_wb_waddr <= vb_o_waddr;
    o_wb_wdata <= vb_o_wdata;
    o_wb_wtag <= vb_o_wtag;
    
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
