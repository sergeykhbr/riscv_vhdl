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
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
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
    i_resp_data_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;                                   -- AXI request was accepted
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    -- Debug signals:
    o_istate : out std_logic_vector(1 downto 0);                      -- ICache state machine value
    o_istate_z : out std_logic_vector(1 downto 0);                    -- ICache previous state (debug purpose)
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
  signal w_ctrl_req_ready : std_logic;
  -- Memory Data interface:
  signal w_data_resp_mem_data_valid : std_logic;
  signal wb_data_resp_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal w_data_req_ready : std_logic;

  component ICache is port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    i_resp_ctrl_ready : in std_logic;
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_istate : out std_logic_vector(1 downto 0);
    o_istate_z : out std_logic_vector(1 downto 0)
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
    o_req_data_ready : out std_logic;
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_resp_data_ready : in std_logic;
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_dstate : out std_logic_vector(1 downto 0)
  );
  end component; 

begin

    i0 : ICache port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_ctrl_valid => i_req_ctrl_valid,
        i_req_ctrl_addr => i_req_ctrl_addr,
        o_req_ctrl_ready => o_req_ctrl_ready,
        o_resp_ctrl_valid => o_resp_ctrl_valid,
        o_resp_ctrl_addr => o_resp_ctrl_addr,
        o_resp_ctrl_data => o_resp_ctrl_data,
        i_resp_ctrl_ready => i_resp_ctrl_ready,
        i_req_mem_ready => w_ctrl_req_ready,
        o_req_mem_valid => i.req_mem_valid,
        o_req_mem_write => i.req_mem_write,
        o_req_mem_addr => i.req_mem_addr,
        o_req_mem_strob => i.req_mem_strob,
        o_req_mem_data => i.req_mem_wdata,
        i_resp_mem_data_valid => w_ctrl_resp_mem_data_valid,
        i_resp_mem_data => wb_ctrl_resp_mem_data,
        o_istate => o_istate,
        o_istate_z => o_istate_z);

    d0 : DCache port map (
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
        i_resp_data_ready => i_resp_data_ready,
        i_req_mem_ready => w_data_req_ready,
        o_req_mem_valid => d.req_mem_valid,
        o_req_mem_write => d.req_mem_write,
        o_req_mem_addr => d.req_mem_addr,
        o_req_mem_strob => d.req_mem_strob,
        o_req_mem_data => d.req_mem_wdata,
        i_resp_mem_data_valid => w_data_resp_mem_data_valid,
        i_resp_mem_data => wb_data_resp_mem_data,
        o_dstate => o_dstate);

  comb : process(i_nrst, i_req_mem_ready, i_resp_mem_data_valid, i_resp_mem_data, 
                 i, d, r)
    variable v : RegistersType;
    variable w_mem_write : std_logic;
    variable wb_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_mem_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    variable wb_mem_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  begin

    v := r;

    w_mem_write := '0';
    wb_mem_addr := (others => '0');
    wb_mem_strob := (others => '0');
    wb_mem_wdata := (others => '0');

    w_data_req_ready <= '0';
    w_data_resp_mem_data_valid <= '0';
    wb_data_resp_mem_data <= (others => '0');
    w_ctrl_req_ready <= '0';
    w_ctrl_resp_mem_data_valid <= '0';
    wb_ctrl_resp_mem_data <= (others => '0');
   
    case r.state is
    when State_Idle =>
        if d.req_mem_valid = '1' then
          w_data_req_ready <= i_req_mem_ready;
          w_mem_write := d.req_mem_write;
          wb_mem_addr := d.req_mem_addr;
          wb_mem_strob := d.req_mem_strob;
          wb_mem_wdata := d.req_mem_wdata;
          if i_req_mem_ready = '1' then
            v.state := State_DMem;
          end if;
        elsif i.req_mem_valid = '1' then
          w_ctrl_req_ready <= i_req_mem_ready;
          w_mem_write := i.req_mem_write;
          wb_mem_addr := i.req_mem_addr;
          wb_mem_strob := i.req_mem_strob;
          wb_mem_wdata := i.req_mem_wdata;
          if i_req_mem_ready = '1' then
            v.state := State_IMem;
          end if;
        end if;

    when State_DMem =>
        w_data_req_ready <= i_req_mem_ready;
        w_mem_write := d.req_mem_write;
        wb_mem_addr := d.req_mem_addr;
        wb_mem_strob := d.req_mem_strob;
        wb_mem_wdata := d.req_mem_wdata;
        if i_resp_mem_data_valid = '1' then
          if (not d.req_mem_valid and i.req_mem_valid) = '1' then
            v.state := State_IMem;
            w_data_req_ready <= '0';
            w_ctrl_req_ready <= i_req_mem_ready;
            w_mem_write := i.req_mem_write;
            wb_mem_addr := i.req_mem_addr;
            wb_mem_strob := i.req_mem_strob;
            wb_mem_wdata := i.req_mem_wdata;
          elsif (d.req_mem_valid or i.req_mem_valid) = '0' then
            v.state := State_Idle;
          end if;
        end if;
        w_data_resp_mem_data_valid <= i_resp_mem_data_valid;
        wb_data_resp_mem_data <= i_resp_mem_data;
        
    when State_IMem =>
        w_ctrl_req_ready <= i_req_mem_ready;
        w_mem_write := i.req_mem_write;
        wb_mem_addr := i.req_mem_addr;
        wb_mem_strob := i.req_mem_strob;
        wb_mem_wdata := i.req_mem_wdata;
        if i_resp_mem_data_valid = '1' then
          if d.req_mem_valid = '1' then
            v.state := State_DMem;
            w_data_req_ready <= i_req_mem_ready;
            w_ctrl_req_ready <= '0';
            w_mem_write := d.req_mem_write;
            wb_mem_addr := d.req_mem_addr;
            wb_mem_strob := d.req_mem_strob;
            wb_mem_wdata := d.req_mem_wdata;
          elsif (d.req_mem_valid or i.req_mem_valid) = '0' then
            v.state := State_Idle;
          end if;
        end if;
        w_ctrl_resp_mem_data_valid <= i_resp_mem_data_valid;
        wb_ctrl_resp_mem_data <= i_resp_mem_data;
        
    when others =>
    end case;

    if i_nrst = '0' then
        v.state := State_Idle;
    end if;

    o_req_mem_valid <= i.req_mem_valid or d.req_mem_valid;
    o_req_mem_write <= w_mem_write;
    o_req_mem_addr <= wb_mem_addr;
    o_req_mem_strob <= wb_mem_strob;
    o_req_mem_data <= wb_mem_wdata;
    o_cstate <= r.state;
    
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
