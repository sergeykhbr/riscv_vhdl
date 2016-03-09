-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @details   Implementation of the axictrl device. 
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;

--! @brief   AXI (NASTI) bus controller. 
--! @details Simplified version with the hardcoded priorities to bus access.
--!          Lower master index has a higher priority.
--! @todo    Round-robin algorithm for the master selection.
entity axictrl is
  generic (
    rdslave_with_waitstate : boolean := false
  );
  port (
    clk    : in std_logic;
    nrst   : in std_logic;
    slvoi  : in  nasti_slaves_out_vector;
    mstoi  : in  nasti_master_out_vector;
    slvio  : out nasti_slave_in_type;
    mstio  : out nasti_master_in_type
  );
end; 
 
architecture arch_axictrl of axictrl is

  constant MSTZERO : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0) := (others => '0');

  type reg_type is record
     mstidx : integer range 0 to CFG_NASTI_MASTER_TOTAL-1;
     mstsel : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
     cur_slave  : nasti_slave_out_type;  -- 1 clock wait state
  end record;

  signal rin, r : reg_type;
begin

  comblogic : process(mstoi, slvoi, r)
    variable v : reg_type;
    variable busreq : std_logic;
    variable mstsel : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    variable n, mstidx, mstidx_cur : integer range 0 to CFG_NASTI_MASTER_TOTAL-1;
    variable cur_master : nasti_master_out_type;
    variable cur_slave  : nasti_slave_out_type;
    variable busy : std_logic;
  begin

    v := r;
    mstsel := r.mstsel;

    busreq := '0';
    mstidx := 0;
    v.cur_slave := nasti_slave_out_none;
    cur_master := nasti_master_out_none;

    -- Select master bus:
    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       if  (mstoi(n).ar_valid or mstoi(n).aw_valid) = '1' then
          mstidx := n;
          busreq := '1';
       end if;
       cur_master.b_ready := cur_master.b_ready or mstoi(n).b_ready;
       cur_master.r_ready := cur_master.r_ready or mstoi(n).r_ready;
    end loop;

    busy := mstoi(r.mstidx).w_valid or mstoi(r.mstidx).r_ready;
    if (r.mstsel = MSTZERO) or ((busreq and not busy) = '1') then
       mstsel(r.mstidx) := '0';
       mstsel(mstidx) := busreq;
       v.mstidx := mstidx;
       mstidx_cur := mstidx;
    else
       mstidx_cur := r.mstidx;
    end if;

    if mstoi(mstidx_cur).aw_valid = '1' then
        cur_master.aw_valid := mstoi(mstidx_cur).aw_valid;
        cur_master.aw_bits := mstoi(mstidx_cur).aw_bits;
        cur_master.aw_id := mstoi(mstidx_cur).aw_id;
        cur_master.aw_user := mstoi(mstidx_cur).aw_user;
    end if;
    if mstoi(mstidx_cur).w_valid = '1' then
        cur_master.w_valid := mstoi(mstidx_cur).w_valid;
        cur_master.w_data := mstoi(mstidx_cur).w_data;
        cur_master.w_last := mstoi(mstidx_cur).w_last;
        cur_master.w_strb := mstoi(mstidx_cur).w_strb;
        cur_master.w_user := mstoi(mstidx_cur).w_user;
    end if;
    if mstoi(mstidx_cur).ar_valid = '1' then
        cur_master.ar_valid := mstoi(mstidx_cur).ar_valid;
        cur_master.ar_bits := mstoi(mstidx_cur).ar_bits;
        cur_master.ar_id := mstoi(mstidx_cur).ar_id;
        cur_master.ar_user := mstoi(mstidx_cur).ar_user;
    end if;


    -- Select slave bus:
    for n in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
       v.cur_slave.ar_ready := v.cur_slave.ar_ready or slvoi(n).ar_ready;
       v.cur_slave.aw_ready := v.cur_slave.aw_ready or slvoi(n).aw_ready;
       v.cur_slave.w_ready  := v.cur_slave.w_ready or slvoi(n).w_ready;

       if v.cur_slave.b_valid = '0' and slvoi(n).b_valid = '1' then
          v.cur_slave.b_valid := '1';
          v.cur_slave.b_resp  := slvoi(n).b_resp;
          v.cur_slave.b_id    := slvoi(n).b_id;
          v.cur_slave.b_user  := slvoi(n).b_user;
       end if;

       if v.cur_slave.r_valid = '0' and slvoi(n).r_valid = '1' then
          v.cur_slave.r_valid := '1';
          v.cur_slave.r_resp  := slvoi(n).r_resp;
          v.cur_slave.r_data  := slvoi(n).r_data;
          v.cur_slave.r_last  := slvoi(n).r_last;
          v.cur_slave.r_id    := slvoi(n).r_id;
          v.cur_slave.r_user  := slvoi(n).r_user;
       end if;
    end loop;


    v.mstsel := mstsel;
    if rdslave_with_waitstate then
      cur_slave := r.cur_slave;
    else
      cur_slave := v.cur_slave;
    end if;
 
    rin <= v;

    mstio.grant    <= mstsel;
    mstio.aw_ready <= cur_slave.aw_ready;
    mstio.w_ready  <= cur_slave.w_ready;
    mstio.b_valid  <= cur_slave.b_valid;
    mstio.b_resp   <= cur_slave.b_resp;
    mstio.b_id     <= cur_slave.b_id;
    mstio.b_user   <= cur_slave.b_user;
    mstio.ar_ready <= cur_slave.ar_ready;
    mstio.r_valid  <= cur_slave.r_valid;
    mstio.r_resp   <= cur_slave.r_resp;
    mstio.r_data   <= cur_slave.r_data;
    mstio.r_last   <= cur_slave.r_last;
    mstio.r_id     <= cur_slave.r_id;
    mstio.r_user   <= cur_slave.r_user;

    slvio.aw_valid <= cur_master.aw_valid;
    slvio.aw_bits  <= cur_master.aw_bits;
    slvio.aw_id    <= cur_master.aw_id;
    slvio.aw_user  <= cur_master.aw_user;
    slvio.w_valid  <= cur_master.w_valid;
    slvio.w_data   <= cur_master.w_data;
    slvio.w_last   <= cur_master.w_last;
    slvio.w_strb   <= cur_master.w_strb;
    slvio.w_user   <= cur_master.w_user;
    slvio.b_ready  <= cur_master.b_ready;
    slvio.ar_valid <= cur_master.ar_valid;
    slvio.ar_bits  <= cur_master.ar_bits;
    slvio.ar_id    <= cur_master.ar_id;
    slvio.ar_user  <= cur_master.ar_user;
    slvio.r_ready  <= cur_master.r_ready;

  end process;

  reg0 : process(clk, nrst) begin
     if nrst = '0' then
        r.mstidx <= 0;
        r.mstsel <= (others =>'0');
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 

  end process;
end;
