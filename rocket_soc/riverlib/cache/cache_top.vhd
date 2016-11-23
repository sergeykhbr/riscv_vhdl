-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Memory Cache Top level.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity CacheTop is
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    -- Control path:
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    -- Data path:
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_sz : in std_logic_vector(1 downto 0);
    i_req_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_req_data_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    -- Memory interface:
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_hold : out std_logic
  );
end; 
 
architecture arch_CacheTop of CacheTop is
  constant State_Idle : std_logic_vector(1 downto 0) := "00";
  constant State_IMem : std_logic_vector(1 downto 0) := "01";
  constant State_DMem : std_logic_vector(1 downto 0) := "10";

  type RegistersType is record
      state : std_logic_vector(1 downto 0);
  end record;

  signal r, rin : RegistersType;
  -- Memory Control interface:
  signal w_ctrl_req_mem_valid : std_logic;
  signal w_ctrl_req_mem_write : std_logic;
  signal wb_ctrl_req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal wb_ctrl_req_mem_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
  signal wb_ctrl_req_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal w_ctrl_resp_mem_data_valid : std_logic;
  signal wb_ctrl_resp_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  -- Memory Data interface:
  signal w_data_req_mem_valid : std_logic;
  signal w_data_req_mem_write : std_logic;
  signal wb_data_req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal wb_data_req_mem_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
  signal wb_data_req_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal w_data_resp_mem_data_valid : std_logic;
  signal wb_data_resp_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);

  component ICache is port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0)
  );
  end component; 

  component DCache is port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_sz : in std_logic_vector(1 downto 0);
    i_req_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_req_data_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0)
  );
  end component; 

begin

    i0 : ICache port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_ctrl_valid => i_req_ctrl_valid,
        i_req_ctrl_addr => i_req_ctrl_addr,
        o_resp_ctrl_valid => o_resp_ctrl_valid,
        o_resp_ctrl_addr => o_resp_ctrl_addr,
        o_resp_ctrl_data => o_resp_ctrl_data,
        o_req_mem_valid => w_ctrl_req_mem_valid,
        o_req_mem_write => w_ctrl_req_mem_write,
        o_req_mem_addr => wb_ctrl_req_mem_addr,
        o_req_mem_strob => wb_ctrl_req_mem_strob,
        o_req_mem_data => wb_ctrl_req_mem_data,
        i_resp_mem_data_valid => w_ctrl_resp_mem_data_valid,
        i_resp_mem_data => wb_ctrl_resp_mem_data);

    d0 : DCache port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_data_valid => i_req_data_valid,
        i_req_data_write => i_req_data_write,
        i_req_data_sz => i_req_data_sz,
        i_req_data_addr => i_req_data_addr,
        i_req_data_data => i_req_data_data,
        o_resp_data_valid => o_resp_data_valid,
        o_resp_data_addr => o_resp_data_addr,
        o_resp_data_data => o_resp_data_data,
        o_req_mem_valid => w_data_req_mem_valid,
        o_req_mem_write => w_data_req_mem_write,
        o_req_mem_addr => wb_data_req_mem_addr,
        o_req_mem_strob => wb_data_req_mem_strob,
        o_req_mem_data => wb_data_req_mem_data,
        i_resp_mem_data_valid => w_data_resp_mem_data_valid,
        i_resp_mem_data => wb_data_resp_mem_data);

  comb : process(i_nrst, i_resp_mem_data_valid, i_resp_mem_data, 
        w_ctrl_req_mem_valid, w_data_req_mem_valid, w_data_req_mem_write,
        wb_data_req_mem_addr, wb_data_req_mem_strob, wb_data_req_mem_data,
        w_ctrl_req_mem_write, wb_ctrl_req_mem_addr, wb_ctrl_req_mem_strob,
        wb_ctrl_req_mem_data, r)
    variable v : RegistersType;
    variable w_mem_valid : std_logic;
    variable w_mem_write : std_logic;
    variable wb_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_mem_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    variable wb_mem_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable w_hold : std_logic;
  begin

    v := r;

    w_data_resp_mem_data_valid <= '0';
    wb_data_resp_mem_data <= (others => '0');
    w_ctrl_resp_mem_data_valid <= '0';
    wb_ctrl_resp_mem_data <= (others => '0');
    case r.state is
    when State_Idle =>
    when State_DMem =>
        w_data_resp_mem_data_valid <= i_resp_mem_data_valid;
        wb_data_resp_mem_data <= i_resp_mem_data;
    when State_IMem =>
        w_ctrl_resp_mem_data_valid <= i_resp_mem_data_valid;
        wb_ctrl_resp_mem_data <= i_resp_mem_data;
    when others =>
    end case;

    w_mem_valid := w_ctrl_req_mem_valid or w_data_req_mem_valid;
    if w_data_req_mem_valid = '1' then
        v.state := State_DMem;
        w_mem_write := w_data_req_mem_write;
        wb_mem_addr := wb_data_req_mem_addr;
        wb_mem_strob := wb_data_req_mem_strob;
        wb_mem_wdata := wb_data_req_mem_data;
    elsif w_ctrl_req_mem_valid = '1' then
        v.state := State_IMem;
        w_mem_write := w_ctrl_req_mem_write;
        wb_mem_addr := wb_ctrl_req_mem_addr;
        wb_mem_strob := wb_ctrl_req_mem_strob;
        wb_mem_wdata := wb_ctrl_req_mem_data;
    else
        v.state := State_Idle;
        w_mem_write := '0';
        wb_mem_addr := (others => '0');
        wb_mem_strob := (others => '0');
        wb_mem_wdata := (others => '0');
    end if;

    w_hold := '0';
    if (i_resp_mem_data_valid = '0' and w_mem_valid = '1' and (r.state /= State_Idle)) 
        or (w_ctrl_req_mem_valid = '1' and w_data_req_mem_valid = '1') then
        w_hold := '1';
    end if;

    if i_nrst = '0' then
        v.state := State_Idle;
    end if;

    o_req_mem_valid <= w_mem_valid;
    o_req_mem_write <= w_mem_write;
    o_req_mem_addr <= wb_mem_addr;
    o_req_mem_strob <= wb_mem_strob;
    o_req_mem_data <= wb_mem_wdata;
    o_hold <= w_hold;
    
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
