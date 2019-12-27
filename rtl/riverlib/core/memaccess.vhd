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
    i_e_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);          -- Execution stage instruction pointer
    i_e_instr : in std_logic_vector(31 downto 0);                     -- Execution stage instruction value

    i_res_addr : in std_logic_vector(5 downto 0);                     -- Register address to be written (0=no writing)
    i_res_data : in std_logic_vector(RISCV_ARCH-1 downto 0);          -- Register value to be written
    i_memop_sign_ext : in std_logic;                                  -- Load data with sign extending (if less than 8 Bytes)
    i_memop_load : in std_logic;                                      -- Load data from memory and write to i_res_addr
    i_memop_store : in std_logic;                                     -- Store i_res_data value into memory
    i_memop_size : in std_logic_vector(1 downto 0);                   -- Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    i_memop_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Memory access address
    o_memop_ready : out std_logic;                                    -- Ready to accept memop request
    o_wena : out std_logic;                                           -- Write enable signal
    o_waddr : out std_logic_vector(5 downto 0);                       -- Output register address (0 = x0 = no write)
    o_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);            -- Register value

    -- Memory interface:
    i_mem_req_ready : in std_logic;
    o_mem_valid : out std_logic;                                      -- Memory request is valid
    o_mem_write : out std_logic;                                      -- Memory write request
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);     -- Data path requested address
    o_mem_wdata : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);     -- Data path requested data (write transaction)
    o_mem_wstrb : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);    -- 8-bytes aligned strobs
    i_mem_data_valid : in std_logic;                                  -- Data path memory response is valid
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Data path memory response address
    i_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);      -- Data path memory response value
    o_mem_resp_ready : out std_logic;

    o_hold : out std_logic;                                           -- Memory operation is more than 1 clock
    o_valid : out std_logic;                                          -- Output is valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);           -- Valid instruction pointer
    o_instr : out std_logic_vector(31 downto 0);                      -- Valid instruction value
    o_wb_memop : out std_logic                                        -- memory operation write-back (for tracer only)
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
      memop_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      memop_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      memop_wstrb : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
      memop_sign_ext : std_logic;
      memop_size : std_logic_vector(1 downto 0);
      
      memop_res_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      memop_res_instr : std_logic_vector(31 downto 0);
      memop_res_addr : std_logic_vector(5 downto 0);
      memop_res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
      memop_res_wena : std_logic;

      reg_wb_valid : std_logic;
      reg_res_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      reg_res_instr : std_logic_vector(31 downto 0);
      reg_res_addr : std_logic_vector(5 downto 0);
      reg_res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
      reg_res_wena : std_logic;

      hold_res_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      hold_res_instr : std_logic_vector(31 downto 0);
      hold_res_waddr : std_logic_vector(5 downto 0);
      hold_res_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      hold_res_wena : std_logic;
  end record;
 
  constant R_RESET : RegistersType := (
    State_Idle,                              -- state
    '0', (others => '0'),                    -- memop_w, memop_addr
    (others => '0'), (others => '0'),        -- memop_wdata, memop_wstrb
    '0', (others => '0'),                    -- memop_sign_ext, memop_size
    (others => '0'), (others => '0'),        -- memop_res_pc, memop_res_instr
    (others => '0'),                         -- memop_res_addr
    (others => '0'), '0',                    -- memop_res_data, memop_wena
    '0',                                     -- reg_wb_valid
    (others => '0'), (others => '0'),        -- reg_res_pc, reg_res_instr
    (others => '0'),                         -- reg_res_addr
    (others => '0'), '0',                    -- reg_res_data, reg_wena
    (others => '0'), (others => '0'),        -- hold_res_pc, hold_res_instr
    (others => '0'),                         -- hold_res_addr
    (others => '0'), '0'                     -- hold_res_data, hold_wena
  );

  signal r, rin : RegistersType;

  -- TODO: move into separate module
  -- queue signals before move into separate module
  constant QUEUE_WIDTH : integer := BUS_DATA_WIDTH
                                  + BUS_DATA_BYTES
                                  + RISCV_ARCH 
                                  + 6
                                  + 32
                                  + BUS_ADDR_WIDTH
                                  + 2
                                  + 1
                                  + 1
                                  + BUS_ADDR_WIDTH
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

 
  comb : process(i_nrst, i_e_valid, i_e_pc, i_e_instr, i_res_addr, i_res_data,
                 i_memop_sign_ext, i_memop_load, i_memop_store, i_memop_size, i_memop_addr,
                 i_mem_data_addr, i_mem_req_ready, i_mem_data_valid,
                 i_mem_data_addr, i_mem_data,
                 queue_data_o, queue_nempty, queue_full, r)
    variable v : RegistersType;
    variable vb_memop_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable vb_memop_wstrb : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    variable v_mem_valid : std_logic;
    variable v_mem_write : std_logic;
    variable v_mem_sign_ext : std_logic;
    variable vb_mem_sz : std_logic_vector(1 downto 0);
    variable vb_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable vb_mem_rdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable v_hold : std_logic;
    variable v_hold_output : std_logic;
    variable v_wb_memop : std_logic;
    variable v_queue_re : std_logic;
    variable vb_mem_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable vb_mem_wstrb : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    variable vb_mem_resp_shifted : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable vb_mem_data_unsigned : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable vb_mem_data_signed : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable vb_res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_res_addr : std_logic_vector(5 downto 0);
    variable vb_e_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable vb_e_instr : std_logic_vector(31 downto 0);
    variable v_memop_ready : std_logic;
    variable v_o_wena : std_logic;
    variable v_o_valid : std_logic;
    variable vb_o_waddr : std_logic_vector(5 downto 0);
    variable vb_o_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_o_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable vb_o_instr : std_logic_vector(31 downto 0);
  begin

    v := r;

    v_mem_valid := '0';
    v_hold := '0';
    v_hold_output := '0';
    v_wb_memop := '0';
    v_queue_re := '0';

    vb_mem_resp_shifted := (others => '0');
    vb_mem_data_unsigned := (others => '0');
    vb_mem_data_signed := (others => '0');

    vb_memop_wdata := (others => '0');
    vb_memop_wstrb := (others => '0');

    case i_memop_size is
    when "00" =>
        vb_memop_wdata := i_res_data(7 downto 0) & i_res_data(7 downto 0)
                        & i_res_data(7 downto 0) & i_res_data(7 downto 0)
                        & i_res_data(7 downto 0) & i_res_data(7 downto 0)
                        & i_res_data(7 downto 0) & i_res_data(7 downto 0);
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
        vb_memop_wdata := i_res_data(15 downto 0) & i_res_data(15 downto 0)
                        & i_res_data(15 downto 0) & i_res_data(15 downto 0);
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
        vb_memop_wdata := i_res_data(31 downto 0) & i_res_data(31 downto 0);
        if i_memop_addr(2) = '1' then
            vb_memop_wstrb := X"F0";
        else
            vb_memop_wstrb := X"0F";
        end if;
    when "11" =>
        vb_memop_wdata := i_res_data;
        vb_memop_wstrb := X"FF";
    when others =>
    end case;

    -- Form Queue inputs:
    queue_data_i <= vb_memop_wdata & vb_memop_wstrb &
                    i_res_data & i_res_addr & i_e_instr & i_e_pc &
                    i_memop_size & i_memop_sign_ext & i_memop_store &
                    i_memop_addr;
    queue_we <= i_e_valid and (i_memop_load or i_memop_store);

    -- Split Queue outputs:
    vb_mem_wdata := queue_data_o(2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+BUS_DATA_WIDTH+42-1 downto
                                 2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+42);
    vb_mem_wstrb := queue_data_o(2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+42-1 downto
                                      2*BUS_ADDR_WIDTH+RISCV_ARCH+42);
    vb_res_data := queue_data_o(2*BUS_ADDR_WIDTH+RISCV_ARCH+42-1 downto
                                      2*BUS_ADDR_WIDTH+42);
    vb_res_addr := queue_data_o(2*BUS_ADDR_WIDTH+42-1 downto
                                2*BUS_ADDR_WIDTH+36);
    vb_e_instr := queue_data_o(2*BUS_ADDR_WIDTH+36-1 downto
                               2*BUS_ADDR_WIDTH+4);
    vb_e_pc := queue_data_o(2*BUS_ADDR_WIDTH+4-1 downto BUS_ADDR_WIDTH+4);
    vb_mem_sz := queue_data_o(BUS_ADDR_WIDTH+3 downto BUS_ADDR_WIDTH+2);
    v_mem_sign_ext := queue_data_o(BUS_ADDR_WIDTH+1);
    v_mem_write := queue_data_o(BUS_ADDR_WIDTH);
    vb_mem_addr := queue_data_o(BUS_ADDR_WIDTH-1 downto 0);

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
            v_mem_valid := '1';
            v.memop_res_pc := vb_e_pc;
            v.memop_res_instr := vb_e_instr;
            v.memop_res_addr := vb_res_addr;
            v.memop_res_data := vb_res_data;
            v.memop_res_wena := or_reduce(vb_res_addr);
            v.memop_addr := vb_mem_addr;
            v.memop_wdata := vb_mem_wdata;
            v.memop_wstrb := vb_mem_wstrb;
            v.memop_w := v_mem_write;
            v.memop_sign_ext := v_mem_sign_ext;
            v.memop_size := vb_mem_sz;

            if i_mem_req_ready = '1' then
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
            v_wb_memop := '1';
            v_queue_re := '1';
            if r.reg_wb_valid = '1' then
                -- Inject only one clock hold-on and wait a couple of clocks
                v_hold := '1';
                v_queue_re := '0';
                v_wb_memop := '0';
                v.state := State_Hold;
                v.hold_res_wena := r.memop_res_wena;
                v.hold_res_waddr := r.memop_res_addr;
                if r.memop_w = '0' then
                    v.hold_res_wdata := vb_mem_rdata;
                else
                    v.hold_res_wdata := r.memop_res_data;
                end if;
                v.hold_res_pc := r.memop_res_pc;
                v.hold_res_instr := r.memop_res_instr;
            elsif queue_nempty = '1' then
                v_mem_valid := '1';
                v.memop_res_pc := vb_e_pc;
                v.memop_res_instr := vb_e_instr;
                v.memop_res_addr := vb_res_addr;
                v.memop_res_data := vb_res_data;
                v.memop_res_wena := or_reduce(vb_res_addr);
                v.memop_addr := vb_mem_addr;
                v.memop_wdata := vb_mem_wdata;
                v.memop_wstrb := vb_mem_wstrb;
                v.memop_w := v_mem_write;
                v.memop_sign_ext := v_mem_sign_ext;
                v.memop_size := vb_mem_sz;

                if i_mem_req_ready = '1' then
                    v.state := State_WaitResponse;
                else
                    v.state := State_WaitReqAccept;
                end if;
            else
                v.state := State_Idle;
            end if;
        end if;
    when State_Hold =>
        if r.reg_wb_valid = '0' then
            v_hold_output := '1';
            v_wb_memop := '1';
            v_queue_re := '1';
            if queue_nempty = '1' then
                v_mem_valid := '1';
                v.memop_res_pc := vb_e_pc;
                v.memop_res_instr := vb_e_instr;
                v.memop_res_addr := vb_res_addr;
                v.memop_res_data := vb_res_data;
                v.memop_res_wena := or_reduce(vb_res_addr);
                v.memop_addr := vb_mem_addr;
                v.memop_wdata := vb_mem_wdata;
                v.memop_wstrb := vb_mem_wstrb;
                v.memop_w := v_mem_write;
                v.memop_sign_ext := v_mem_sign_ext;
                v.memop_size := vb_mem_sz;

                if i_mem_req_ready = '1' then
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

    if i_e_valid = '1' and (i_memop_load or i_memop_store) = '0' then
        v.reg_wb_valid := '1';
        v.reg_res_pc := i_e_pc;
        v.reg_res_instr := i_e_instr;
        v.reg_res_addr := i_res_addr;
        v.reg_res_data := i_res_data;
        v.reg_res_wena := or_reduce(i_res_addr);
    else
        v.reg_wb_valid := '0';
        v.reg_res_pc := (others => '0');
        v.reg_res_instr := (others => '0');
        v.reg_res_addr := (others => '0');
        v.reg_res_data := (others => '0');
        v.reg_res_wena := '0';
    end if;

    v_memop_ready := '1';
    if queue_full = '1' then
        v_memop_ready := '0';
    end if;

    v_o_valid := '0';
    if r.reg_wb_valid = '1' then
        v_o_valid := '1';
        v_o_wena := r.reg_res_wena;
        vb_o_waddr := r.reg_res_addr;
        vb_o_wdata := r.reg_res_data;
        vb_o_pc := r.reg_res_pc;
        vb_o_instr := r.reg_res_instr;
    elsif v_hold_output = '1' then
        v_o_valid := '1';
        v_o_wena := r.hold_res_wena;
        vb_o_waddr := r.hold_res_waddr;
        vb_o_wdata := r.hold_res_wdata;
        vb_o_pc := r.hold_res_pc;
        vb_o_instr := r.hold_res_instr;
    elsif v_wb_memop = '1' then
        v_o_valid := '1';
        v_o_wena := r.memop_res_wena;
        vb_o_waddr := r.memop_res_addr;
        vb_o_wdata := vb_mem_rdata;
        vb_o_pc := r.memop_res_pc;
        vb_o_instr := r.memop_res_instr;
    else
        v_o_wena := '0';
        vb_o_waddr := (others => '0');
        vb_o_wdata := (others => '0');
        vb_o_pc := (others => '0');
        vb_o_instr := (others => '0');
    end if;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    queue_re <= v_queue_re;

    o_mem_resp_ready <= '1';

    o_mem_valid <= v_mem_valid;
    o_mem_write <= v_mem_write;
    o_mem_addr <= vb_mem_addr(BUS_ADDR_WIDTH-1 downto CFG_LOG2_DATA_BYTES)
                & zero64(CFG_LOG2_DATA_BYTES-1 downto 0);
    o_mem_wdata <= vb_mem_wdata;
    o_mem_wstrb <= vb_mem_wstrb;

    o_hold <= v_hold;
    o_memop_ready <= v_memop_ready;
    o_wena <= v_o_wena;
    o_waddr <= vb_o_waddr;
    o_wdata <= vb_o_wdata;
    -- the following signal used to executation instruction count and debug
    o_valid <= v_o_valid;
    o_pc <= vb_o_pc;
    o_instr <= vb_o_instr;
    o_wb_memop <= v_wb_memop;
    
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
