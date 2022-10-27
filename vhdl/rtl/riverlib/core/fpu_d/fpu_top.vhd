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
library riverlib;
use riverlib.river_cfg.all;

entity FpuTop is 
  generic (
    async_reset : boolean
  );
  port (
    i_nrst         : in std_logic;
    i_clk          : in std_logic;
    i_ena          : in std_logic;
    i_ivec         : in std_logic_vector(Instr_FPU_Total-1 downto 0);
    i_a            : in std_logic_vector(63 downto 0);
    i_b            : in std_logic_vector(63 downto 0);
    o_res          : out std_logic_vector(63 downto 0);
    o_ex_invalidop : out std_logic;   -- Exception: invalid operation
    o_ex_divbyzero : out std_logic;   -- Exception: divide by zero
    o_ex_overflow  : out std_logic;   -- Exception: overflow
    o_ex_underflow : out std_logic;   -- Exception: underflow
    o_ex_inexact   : out std_logic;   -- Exception: inexact
    o_valid        : out std_logic;
    o_busy         : out std_logic
  );
end; 
 
architecture arch_FpuTop of FpuTop is

  component DoubleAdd is generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_add        : in std_logic;
    i_sub        : in std_logic;
    i_eq         : in std_logic;
    i_lt         : in std_logic;
    i_le         : in std_logic;
    i_max        : in std_logic;
    i_min        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    i_b          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_illegal_op : out std_logic;
    o_overflow   : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
  end component;

  component DoubleDiv is generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    i_b          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_illegal_op : out std_logic;
    o_divbyzero  : out std_logic;
    o_overflow   : out std_logic;
    o_underflow  : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
  end component;

  component DoubleMul is generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    i_b          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_illegal_op : out std_logic;
    o_overflow   : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
  end component;

  component Double2Long is generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_signed     : in std_logic;
    i_w32        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_overflow   : out std_logic;
    o_underflow  : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
  end component;

  component Long2Double is generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_signed     : in std_logic;
    i_w32        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
  end component;

  type RegistersType is record
    ivec : std_logic_vector(Instr_FPU_Total-1 downto 0);
    busy : std_logic;
    ready : std_logic;
    a : std_logic_vector(63 downto 0);
    b : std_logic_vector(63 downto 0);
    result : std_logic_vector(63 downto 0);
    ex_invalidop : std_logic;   -- Exception: invalid operation
    ex_divbyzero : std_logic;   -- Exception: divide by zero
    ex_overflow : std_logic;    -- Exception: overflow
    ex_underflow : std_logic;   -- Exception: underflow
    ex_inexact : std_logic;     -- Exception: inexact
    ena_fadd : std_logic;
    ena_fdiv : std_logic;
    ena_fmul : std_logic;
    ena_d2l : std_logic;
    ena_l2d : std_logic;
    ena_w32 : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    (others => '0'),                                -- ivec
    '0', '0', (others => '0'), (others => '0'),     -- busy, ready, a, b
    (others => '0'),                                -- result
    '0', '0', '0',                                  -- ex_invalidop, ex_divbyzero, ex_overflow
    '0', '0', '0',                                  -- ex_underflow, ex_inexact, ena_fadd
    '0', '0', '0', '0',                             -- ena_fdiv, ena_fmul, ena_d2l, ena_l2d
    '0'                                             -- ena_w32
  );

  signal r, rin : RegistersType;

  signal w_fadd_d : std_logic;
  signal w_fsub_d : std_logic;
  signal w_feq_d : std_logic;
  signal w_flt_d : std_logic;
  signal w_fle_d : std_logic;
  signal w_fmax_d : std_logic;
  signal w_fmin_d : std_logic;
  signal w_fcvt_signed : std_logic;
  signal wb_res_fadd : std_logic_vector(63 downto 0);
  signal w_valid_fadd : std_logic;
  signal w_illegalop_fadd : std_logic;
  signal w_overflow_fadd : std_logic;
  signal w_busy_fadd : std_logic;

  signal wb_res_fdiv : std_logic_vector(63 downto 0);
  signal w_valid_fdiv : std_logic;
  signal w_illegalop_fdiv : std_logic;
  signal w_divbyzero_fdiv : std_logic;
  signal w_overflow_fdiv : std_logic;
  signal w_underflow_fdiv : std_logic;
  signal w_busy_fdiv : std_logic;

  signal wb_res_fmul : std_logic_vector(63 downto 0);
  signal w_valid_fmul : std_logic;
  signal w_illegalop_fmul : std_logic;
  signal w_overflow_fmul : std_logic;
  signal w_busy_fmul : std_logic;

  signal wb_res_d2l : std_logic_vector(63 downto 0);
  signal w_valid_d2l : std_logic;
  signal w_overflow_d2l : std_logic;
  signal w_underflow_d2l : std_logic;
  signal w_busy_d2l : std_logic;

  signal wb_res_l2d : std_logic_vector(63 downto 0);
  signal w_valid_l2d : std_logic;
  signal w_busy_l2d : std_logic;

begin
    fadd_d0 : DoubleAdd generic map (
      async_reset => async_reset
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_ena => r.ena_fadd,
      i_add => w_fadd_d,
      i_sub => w_fsub_d,
      i_eq => w_feq_d,
      i_lt => w_flt_d,
      i_le => w_fle_d,
      i_max => w_fmax_d,
      i_min => w_fmin_d,
      i_a => r.a,
      i_b => r.b,
      o_res => wb_res_fadd,
      o_illegal_op => w_illegalop_fadd,
      o_overflow => w_overflow_fadd,
      o_valid => w_valid_fadd,
      o_busy => w_busy_fadd
    );

    fdiv_d0 : DoubleDiv generic map (
      async_reset => async_reset
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_ena => r.ena_fdiv,
      i_a => r.a,
      i_b => r.b,
      o_res => wb_res_fdiv,
      o_illegal_op => w_illegalop_fdiv,
      o_divbyzero => w_divbyzero_fdiv,
      o_overflow => w_overflow_fdiv,
      o_underflow => w_underflow_fdiv,
      o_valid => w_valid_fdiv,
      o_busy => w_busy_fdiv
    );

    fmul_d0 : DoubleMul generic map (
      async_reset => async_reset
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_ena => r.ena_fmul,
      i_a => r.a,
      i_b => r.b,
      o_res => wb_res_fmul,
      o_illegal_op => w_illegalop_fmul,
      o_overflow => w_overflow_fmul,
      o_valid => w_valid_fmul,
      o_busy => w_busy_fmul
    );

    d2l_d0 : Double2Long generic map (
      async_reset => async_reset
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_ena => r.ena_d2l,
      i_signed => w_fcvt_signed,
      i_w32 => r.ena_w32,
      i_a => r.a,
      o_res => wb_res_d2l,
      o_overflow => w_overflow_d2l,
      o_underflow => w_underflow_d2l,
      o_valid => w_valid_d2l,
      o_busy => w_busy_d2l
    );

    l2d_d0 : Long2Double generic map (
      async_reset => async_reset
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_ena => r.ena_l2d,
      i_signed => w_fcvt_signed,
      i_w32 => r.ena_w32,
      i_a => r.a,
      o_res => wb_res_l2d,
      o_valid => w_valid_l2d,
      o_busy => w_busy_l2d
    );


  -- registers:
  comb : process(i_nrst, i_ena, i_ivec, i_a, i_b, r,
                wb_res_fadd, w_valid_fadd, w_illegalop_fadd, w_overflow_fadd, w_busy_fadd,
                wb_res_fdiv, w_valid_fdiv, w_illegalop_fdiv, w_divbyzero_fdiv, w_overflow_fdiv,
                w_underflow_fdiv, w_busy_fdiv,
                wb_res_fmul, w_valid_fmul, w_illegalop_fmul, w_overflow_fmul, w_busy_fmul,
                wb_res_d2l, w_valid_d2l, w_overflow_d2l, w_underflow_d2l, w_busy_d2l,
                wb_res_l2d, w_valid_l2d, w_busy_l2d)
    variable v : RegistersType;
    variable iv : std_logic_vector(Instr_FPU_Total-1 downto 0);
  begin

    v := r;
    iv := i_ivec;

    v.ena_fadd := '0';
    v.ena_fdiv := '0';
    v.ena_fmul := '0';
    v.ena_d2l := '0';
    v.ena_l2d := '0';
    v.ready := '0';
    if i_ena = '1' and r.busy = '0' then
        v.busy := '1';
        v.a := i_a;
        v.b := i_b;
        v.ivec := i_ivec;
        v.ex_invalidop := '0';
        v.ex_divbyzero := '0';
        v.ex_overflow := '0';
        v.ex_underflow := '0';
        v.ex_inexact := '0';

        v.ena_fadd := iv(Instr_FADD_D - Instr_FADD_D)
                    or iv(Instr_FSUB_D - Instr_FADD_D)
                    or iv(Instr_FLE_D - Instr_FADD_D)
                    or iv(Instr_FLT_D - Instr_FADD_D)
                    or iv(Instr_FEQ_D - Instr_FADD_D)
                    or iv(Instr_FMAX_D - Instr_FADD_D)
                    or iv(Instr_FMIN_D - Instr_FADD_D);
        v.ena_fdiv := iv(Instr_FDIV_D - Instr_FADD_D);
        v.ena_fmul := iv(Instr_FMUL_D - Instr_FADD_D);
        v.ena_d2l := iv(Instr_FCVT_LU_D - Instr_FADD_D)
                    or iv(Instr_FCVT_L_D - Instr_FADD_D)
                    or iv(Instr_FCVT_WU_D - Instr_FADD_D)
                    or iv(Instr_FCVT_W_D - Instr_FADD_D);
        v.ena_l2d := iv(Instr_FCVT_D_LU - Instr_FADD_D)
                    or iv(Instr_FCVT_D_L - Instr_FADD_D)
                    or iv(Instr_FCVT_D_WU - Instr_FADD_D)
                    or iv(Instr_FCVT_D_W - Instr_FADD_D);

        v.ena_w32 := iv(Instr_FCVT_WU_D - Instr_FADD_D)
                    or iv(Instr_FCVT_W_D - Instr_FADD_D)
                    or iv(Instr_FCVT_D_WU - Instr_FADD_D)
                    or iv(Instr_FCVT_D_W - Instr_FADD_D);
    end if;

    if r.busy = '1' and (r.ivec(Instr_FMOV_X_D - Instr_FADD_D)
                        or r.ivec(Instr_FMOV_D_X - Instr_FADD_D)) = '1' then
        v.busy := '0';
        v.ready := '1';
        v.result := r.a;
    elsif w_valid_fadd = '1' then
        v.busy := '0';
        v.ready := '1';
        v.result := wb_res_fadd;
        v.ex_invalidop := w_illegalop_fadd;
        v.ex_overflow := w_overflow_fadd;
    elsif w_valid_fdiv = '1' then
        v.busy := '0';
        v.ready := '1';
        v.result := wb_res_fdiv;
        v.ex_invalidop := w_illegalop_fdiv;
        v.ex_divbyzero := w_divbyzero_fdiv;
        v.ex_overflow := w_overflow_fdiv;
        v.ex_underflow := w_underflow_fdiv;
    elsif w_valid_fmul = '1' then
        v.busy := '0';
        v.ready := '1';
        v.result := wb_res_fmul;
        v.ex_invalidop := w_illegalop_fmul;
        v.ex_overflow := w_overflow_fmul;
    elsif w_valid_d2l = '1' then
        v.busy := '0';
        v.ready := '1';
        v.result := wb_res_d2l;
        v.ex_overflow := w_overflow_d2l;
        v.ex_underflow := w_underflow_d2l;
    elsif w_valid_l2d = '1' then
        v.busy := '0';
        v.ready := '1';
        v.result := wb_res_l2d;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;

    w_fadd_d <= r.ivec(Instr_FADD_D - Instr_FADD_D);
    w_fsub_d <= r.ivec(Instr_FSUB_D - Instr_FADD_D);
    w_feq_d <= r.ivec(Instr_FEQ_D - Instr_FADD_D);
    w_flt_d <= r.ivec(Instr_FLT_D - Instr_FADD_D);
    w_fle_d <= r.ivec(Instr_FLE_D - Instr_FADD_D);
    w_fmax_d <= r.ivec(Instr_FMAX_D - Instr_FADD_D);
    w_fmin_d <= r.ivec(Instr_FMIN_D - Instr_FADD_D);
    w_fcvt_signed <= r.ivec(Instr_FCVT_L_D - Instr_FADD_D) or
                     r.ivec(Instr_FCVT_D_L - Instr_FADD_D) or
                     r.ivec(Instr_FCVT_W_D - Instr_FADD_D) or
                     r.ivec(Instr_FCVT_D_W - Instr_FADD_D);

    o_res <= r.result;
    o_ex_invalidop <= r.ex_invalidop;
    o_ex_divbyzero <= r.ex_divbyzero;
    o_ex_overflow <= r.ex_overflow;
    o_ex_underflow <= r.ex_underflow;
    o_ex_inexact <= r.ex_inexact;
    o_valid <= r.ready;
    o_busy <= r.busy;
  
  -- registers:
  regs : process(i_nrst, i_clk)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
