------------------------------------------------------------------------------
--! @file
--! @brief UART controller for the AXI4 (NASTI) bus.
--! @author Sergey Khabarov
--! @details  Asynchronous UART. Implements 8-bit data frame with one stop-bit.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;
use rocketlib.types_rocket.all;

entity nasti_uart is
  generic (
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port (
    nrst   : in  std_ulogic;
    clk    : in  std_ulogic;
    i      : in  nasti_slave_in_type;
    o      : out nasti_slave_out_type;
    uarti  : in  uart_in_type;
    uarto  : out uart_out_type);
end;

architecture arch_nasti_uart of nasti_uart is

  constant fifosize : integer := 16;
  constant abits    : integer := 8;
  constant sbits    : integer := 12;
  constant parity   : integer := 1;
  constant flow     : integer := 1;
  
  constant ALIGNMENT_BYTES : integer := 4;
  type local_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1) 
       of integer;
  
  constant config : nasti_slave_config_type := (
     xindex,
     conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS)
  );

type rxfsmtype is (idle, startbit, data, cparity, stopbit);
type txfsmtype is (idle, data, cparity, stopbit);

type fifo is array (0 to fifosize - 1) of std_logic_vector(7 downto 0);

type uartregs is record
  bank_axi : nasti_slave_bank_type;

  rxen          :  std_ulogic;  -- receiver enabled
  txen          :  std_ulogic;  -- transmitter enabled
  rirqen        :  std_ulogic;  -- receiver irq enable
  tirqen        :  std_ulogic;  -- transmitter irq enable
  parsel        :  std_ulogic;  -- parity select
  paren         :  std_ulogic;  -- parity select
  flow          :  std_ulogic;  -- flow control enable
  loopb         :  std_ulogic;  -- loop back mode enable
  debug         :  std_ulogic;  -- debug mode enable
  rsempty       :  std_ulogic;  -- receiver shift register empty (internal)
  tsempty       :  std_ulogic;  -- transmitter shift register empty
  tsemptyirqen  :  std_ulogic;  -- generate irq when tx shift register is empty
  break         :  std_ulogic;  -- break detected
  breakirqen    :  std_ulogic;  -- generate irq when break has been received
  ovf           :  std_ulogic;  -- receiver overflow
  parerr        :  std_ulogic;  -- parity error
  frame         :  std_ulogic;  -- framing error
  ctsn          :  std_logic_vector(1 downto 0); -- clear to send
  rtsn          :  std_ulogic;  -- request to send
  extclken      :  std_ulogic;  -- use external baud rate clock
  extclk        :  std_ulogic;  -- rising edge detect register
  rhold         :  fifo;
  rshift        :  std_logic_vector(7 downto 0);
  tshift        :  std_logic_vector(10 downto 0);
  thold         :  fifo;
  irq           :  std_ulogic;  -- tx/rx interrupt (internal)
  irqpend       :  std_ulogic;  -- pending irq for delayed rx irq
  delayirqen    :  std_ulogic;  -- enable delayed rx irq
  tpar          :  std_ulogic;  -- tx data parity (internal)
  txstate       :  txfsmtype;
  txclk         :  std_logic_vector(2 downto 0);  -- tx clock divider
  txtick        :  std_ulogic;  -- tx clock (internal)
  rxstate       :  rxfsmtype;
  rxclk         :  std_logic_vector(2 downto 0); -- rx clock divider
  rxdb          :  std_logic_vector(1 downto 0);  -- rx delay
  dpar          :  std_ulogic;  -- rx data parity (internal)
  rxtick        :  std_ulogic;  -- rx clock (internal)
  tick          :  std_ulogic;  -- rx clock (internal)
  scaler        :  std_logic_vector(sbits-1 downto 0);
  brate         :  std_logic_vector(sbits-1 downto 0);
  rxf           :  std_logic_vector(4 downto 0); --  rx data filtering buffer
  txd           :  std_ulogic;  -- transmitter data
  rfifoirqen    :  std_ulogic;  -- receiver fifo interrupt enable
  tfifoirqen    :  std_ulogic;  -- transmitter fifo interrupt enable
  irqcnt        :  std_logic_vector(5 downto 0); -- delay counter for rx irq
 --fifo counters
  rwaddr        :  std_logic_vector(log2x(fifosize) - 1 downto 0);
  rraddr        :  std_logic_vector(log2x(fifosize) - 1 downto 0);
  traddr        :  std_logic_vector(log2x(fifosize) - 1 downto 0);
  twaddr        :  std_logic_vector(log2x(fifosize) - 1 downto 0);
  rcnt          :  std_logic_vector(log2x(fifosize) downto 0);
  tcnt          :  std_logic_vector(log2x(fifosize) downto 0);
end record;

constant rcntzero : std_logic_vector(log2x(fifosize) downto 0) := (others => '0');
constant addrzero : std_logic_vector(log2x(fifosize)-1 downto 0) := (others => '0');
constant sbitszero : std_logic_vector(sbits-1 downto 0) := (others => '0');
constant fifozero : fifo := (others => (others => '0'));

constant RES : uartregs :=
  (bank_axi => NASTI_SLAVE_BANK_RESET, rxen => '0', txen => '0', rirqen => '0', tirqen => '0', parsel => '0',
   paren => '0', flow => '0', loopb => '0', debug => '0', rsempty => '1',
   tsempty => '1', tsemptyirqen => '0', break => '0', breakirqen => '0',
   ovf => '0', parerr => '0', frame => '0', ctsn => (others => '0'),
   rtsn => '1', extclken => '0', extclk => '0', rhold => fifozero,
   rshift => (others => '0'), tshift => (others => '1'), thold => fifozero,
   irq => '0', irqpend => '0', delayirqen => '0', tpar => '0', txstate => idle,
   txclk => (others => '0'), txtick => '0', rxstate => idle,
   rxclk => (others => '0'), rxdb => (others => '0'), dpar => '0',rxtick => '0',
   tick => '0', scaler => sbitszero, brate => sbitszero, rxf => (others => '0'),
   txd => '1', rfifoirqen => '0', tfifoirqen => '0', irqcnt => (others => '0'),
   rwaddr => addrzero, rraddr => addrzero, traddr => addrzero, twaddr => addrzero,
   rcnt => rcntzero, tcnt => rcntzero);


  procedure procedureWriteReg(
         r      : in uartregs;
         addr   : in integer;
         strobs : in std_logic_vector(3 downto 0);
         val    : in std_logic_vector(31 downto 0);
         v      : out uartregs) is
  variable tfull : std_logic;
  begin
    v := r;
    tfull := r.tcnt(log2x(fifosize));
    if conv_integer(strobs) /= 0 then
      case addr is
      when 0 =>
        if fifosize = 1 then
          v.thold(0) := val(7 downto 0);
          v.tcnt(0) := '1';
        else
          v.thold(conv_integer(r.twaddr)) := val(7 downto 0);
          if not (tfull = '1') then
            v.twaddr := r.twaddr + 1;
            v.tcnt :=  r.tcnt + 1;
          end if;
        end if;
      when 1 =>
        v.frame      := val(6);
        v.parerr     := val(5);
        v.ovf        := val(4);
        v.break      := val(3);
      when 2 =>
        v.tsemptyirqen := val(14);
        v.delayirqen := val(13);
        v.breakirqen := val(12);
        v.debug      := val(11);
        if fifosize /= 1 then
          v.rfifoirqen := val(10);
          v.tfifoirqen := val(9);
        end if;
        v.extclken   := val(8);
        v.loopb      := val(7);
        v.flow       := val(6);
        v.paren      := val(5);
        v.parsel     := val(4);
        v.tirqen     := val(3);
        v.rirqen     := val(2);
        v.txen       := val(1);
        v.rxen       := val(0);
      when 3 =>
        v.brate      := val(sbits-1 downto 0);
        v.scaler     := val(sbits-1 downto 0);
      when 4 =>
        -- Write RX fifo and generate irq
        if flow /= 0 then
          v.rhold(conv_integer(r.rwaddr)) := val(7 downto 0);
          if fifosize = 1 then 
             v.rcnt(0) := '1';
          else 
             v.rwaddr := r.rwaddr + 1;
             v.rcnt := r.rcnt + 1;
          end if;

          if r.debug = '1' then
              v.irq := r.irq or r.rirqen;
          end if;
          
        end if;
      when others =>
      end case;
    end if;
  end procedure;

  procedure procedureReadReg(
         r    : in uartregs;
         addr : in integer;
         val  : out std_logic_vector(31 downto 0)) is
  begin
    val := X"00000000";
    case addr is
    when 0 =>
        val(7 downto 0) := r.rhold(conv_integer(r.rraddr));
    when 1 =>
      if fifosize /= 1 then
        val(26 + log2x(fifosize) downto 26) := r.rcnt;
        val(20 + log2x(fifosize) downto 20) := r.tcnt;
        --val(10 downto 7) := rfull & tfull & rhalffull & thalffull;
      end if;
      val(6 downto 0) := r.frame & r.parerr & r.ovf
              &  r.break 
              & '1'--thempty 
              & r.tsempty
              & '0';--dready;

    when 2 =>
      if fifosize > 1 then
        val(31) := '1';
      end if;
      val(14) := r.tsemptyirqen;
      val(13) := r.delayirqen;
      val(12) := r.breakirqen;
      val(11) := r.debug;
      if fifosize /= 1 then
        val(10 downto 9) := r.rfifoirqen & r.tfifoirqen;
      end if;
      val(8 downto 0) := r.extclken & r.loopb &
           r.flow & r.paren & r.parsel & r.tirqen & 
           r.rirqen & r.txen & r.rxen;

    when 3 =>
       val(sbits-1 downto 0) := r.brate;

    when 4 =>
        -- Read TX FIFO.
        val(7 downto 0) := r.thold(conv_integer(r.traddr));
    when others =>  val := X"badef00d";
    end case;
  end procedure;

signal r, rin : uartregs;

begin
  uartop : process(nrst, r, i, uarti )
  variable rdata : std_logic_vector(31 downto 0);
  variable scaler : std_logic_vector(sbits-1 downto 0);
  variable rxclk, txclk : std_logic_vector(2 downto 0);
  variable rxd, ctsn : std_ulogic;
--  variable irq : std_logic_vector(NAHBIRQ-1 downto 0);
  variable paddress : std_logic_vector(7 downto 2);
  variable v : uartregs;
  variable thalffull : std_ulogic;
  variable rhalffull : std_ulogic;
  variable rfull : std_ulogic;
  variable tfull : std_ulogic;
  variable dready : std_ulogic;
  variable thempty : std_ulogic;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata2 : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);

  begin

    v := r; 
    --irq := (others => '0'); irq(pirq) := r.irq;
    --v.irq := '0'; 
    v.txtick := '0'; v.rxtick := '0'; v.tick := '0';
    rdata := (others => '0'); v.rxdb(1) := r.rxdb(0);
    dready := '0'; thempty := '1'; thalffull := '1'; rhalffull := '0';
    v.ctsn := r.ctsn(0) & uarti.ctsn;

    if fifosize = 1 then
      dready := r.rcnt(0); rfull := dready; tfull := r.tcnt(0);
      thempty := not tfull;
    else
      tfull := r.tcnt(log2x(fifosize)); rfull := r.rcnt(log2x(fifosize));
      if (r.rcnt(log2x(fifosize)) or r.rcnt(log2x(fifosize) - 1)) = '1' then
        rhalffull := '1';
      end if;
      if ((r.tcnt(log2x(fifosize)) or r.tcnt(log2x(fifosize) - 1))) = '1' then
        thalffull := '0';
      end if;
      if r.rcnt /= rcntzero then dready := '1'; end if;
      if r.tcnt /= rcntzero then thempty := '0'; end if;
    end if;

-- scaler

    scaler := r.scaler - 1;
    if (r.rxen or r.txen) = '1' then
      v.scaler := scaler;
      v.tick := scaler(sbits-1) and not r.scaler(sbits-1);
      if v.tick = '1' then v.scaler := r.brate; end if;
    end if;

-- optional external uart clock
    v.extclk := uarti.extclk;
    if r.extclken = '1' then v.tick := r.extclk and not uarti.extclk; end if;

-- read/write registers
    procedureAxi4(i, config, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
       raddr_reg(n) := conv_integer(r.bank_axi.raddr(ALIGNMENT_BYTES*n)(7 downto 2));
       procedureReadReg(r, 
                        raddr_reg(n),
                        rdata2(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n));

       -- Update RX FIFO:
       if raddr_reg(n) = 0 then 
         if fifosize = 1 then 
           v.rcnt(0) := '0';
         else
           if r.rcnt /= rcntzero then
             v.rraddr := r.rraddr + 1; 
             v.rcnt   := r.rcnt - 1;
           end if;
         end if;
       end if;

        -- Update TX FIFO.
       if raddr_reg(n) = 4 then 
         if r.debug = '1' and r.tcnt /= rcntzero then
            if fifosize = 1 then
                v.tcnt(0) := '0';
            else
                v.traddr := r.traddr + 1;
                v.tcnt := r.tcnt - 1;
            end if;
         end if;
       end if;
    end loop;


    if i.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      wdata := i.w_data;
      wstrb := i.w_strb;
      for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(ALIGNMENT_BYTES*n)(7 downto 2));
         procedureWriteReg(v, 
                           waddr_reg(n),
                           wstrb(ALIGNMENT_BYTES*(n+1)-1 downto ALIGNMENT_BYTES*n),
                           wdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n),
                           v);
      end loop;
    end if;

    o <= functionAxi4Output(r.bank_axi, rdata2);


-- tx clock

    txclk := r.txclk + 1;
    if r.tick = '1' then
      v.txclk := txclk;
      v.txtick := r.txclk(2) and not txclk(2);
    end if;

-- rx clock

    rxclk := r.rxclk + 1;
    if r.tick = '1' then
      v.rxclk := rxclk;
      v.rxtick := r.rxclk(2) and not rxclk(2);
    end if;

    if (r.rxtick and r.delayirqen) = '1' then
      v.irqcnt := v.irqcnt + 1;
    end if;

    if r.irqcnt(5 downto 4) = "11" then
      --v.irq := v.irq or (r.delayirqen and r.irqpend); -- make sure no tx irqs are lost !
      v.irqpend := '0';
    end if;

-- filter rx data

--    v.rxf := r.rxf(6 downto 0) & uarti.rxd;
--    if ((r.rxf(7) & r.rxf(7) & r.rxf(7) & r.rxf(7) & r.rxf(7) & r.rxf(7) &
--       r.rxf(7)) = r.rxf(6 downto 0))
--    then v.rxdb(0) := r.rxf(7); end if;

    v.rxf(1 downto 0) := r.rxf(0) & uarti.rxd;  -- meta-stability filter
    if r.tick = '1' then
      v.rxf(4 downto 2) := r.rxf(3 downto 1);
    end if;
    v.rxdb(0) := (r.rxf(4) and r.rxf(3)) or (r.rxf(4) and r.rxf(2)) or 
                  (r.rxf(3) and r.rxf(2));
-- loop-back mode
    if r.loopb = '1' then
      v.rxdb(0) := r.tshift(0); ctsn := dready and not r.rsempty;
    elsif (flow = 1) then ctsn := r.ctsn(1); else ctsn := '0'; end if;
    rxd := r.rxdb(0);

-- transmitter operation

    case r.txstate is
    when idle =>        -- idle state
      if (r.txtick = '1') then v.tsempty := '1'; end if;
      
      if ((not r.debug and r.txen and (not thempty) and r.txtick) and
          ((not ctsn) or not r.flow)) = '1' then
          v.txstate := data;
          v.tpar := r.parsel; v.tsempty := '0';
          v.txclk := "00" & r.tick; v.txtick := '0';
          v.tshift := "10" & r.thold(conv_integer(r.traddr)) & '0';
          if fifosize = 1 then
              --v.irq := r.irq or r.tirqen; v.tcnt(0) := '0';
          else
              v.traddr := r.traddr + 1;
              v.tcnt := r.tcnt - 1;
          end if;
      end if;
      
    when data =>        -- transmit data frame
      if r.txtick = '1' then
        v.tpar := r.tpar xor r.tshift(1);
        v.tshift := '1' & r.tshift(10 downto 1);
        if r.tshift(10 downto 1) = "1111111110" then
          if r.paren = '1' then
            v.tshift(0) := r.tpar; v.txstate := cparity;
          else
            v.tshift(0) := '1'; v.txstate := stopbit;
          end if;
        end if;
      end if;
    when cparity =>     -- transmit parity bit
      if r.txtick = '1' then
        v.tshift := '1' & r.tshift(10 downto 1); v.txstate := stopbit;
      end if;
    when stopbit =>     -- transmit stop bit
      if r.txtick = '1' then
        v.tshift := '1' & r.tshift(10 downto 1); v.txstate := idle;
      end if;

    end case;

-- writing of tx data register must be done after tx fsm to get correct
-- operation of thempty flag

--    if (apbi.psel(pindex) and apbi.penable and apbi.pwrite) = '1' then
--      case paddress(4 downto 2) is
--      when "000" =>
--        if fifosize = 1 then
--          v.thold(0) := apbi.pwdata(7 downto 0); v.tcnt(0) := '1';
--        else
--          v.thold(conv_integer(r.twaddr)) := apbi.pwdata(7 downto 0);
--          if not (tfull = '1') then
--            v.twaddr := r.twaddr + 1; v.tcnt :=  v.tcnt + 1;
--          end if;
--        end if;
--      when others => null;
--      end case;
--    end if;

-- receiver operation

    case r.rxstate is
    when idle =>        -- wait for start bit
      if ((r.rsempty = '0') and not (rfull = '1')) then
          v.rsempty := '1';
          v.rhold(conv_integer(r.rwaddr)) := r.rshift;
          if fifosize = 1 then v.rcnt(0) := '1';
          else v.rwaddr := r.rwaddr + 1; v.rcnt := v.rcnt + 1; end if;
      end if;
      if (r.rxen and r.rxdb(1) and (not rxd)) = '1' then
        v.rxstate := startbit; v.rshift := (others => '1'); v.rxclk := "100";
        if v.rsempty = '0' then v.ovf := '1'; end if;
        v.rsempty := '0'; v.rxtick := '0';
      end if;
    when startbit =>    -- check validity of start bit
      if r.rxtick = '1' then
        if rxd = '0' then
          v.rshift := rxd & r.rshift(7 downto 1); v.rxstate := data;
          v.dpar := r.parsel;
        else
          v.rxstate := idle;
        end if;
      end if;
    when data =>        -- receive data frame
      if r.rxtick = '1' then
        v.dpar := r.dpar xor rxd;
        v.rshift := rxd & r.rshift(7 downto 1);
        if r.rshift(0) = '0' then
          if r.paren = '1' then v.rxstate := cparity;
          else v.rxstate := stopbit; v.dpar := '0'; end if;
        end if;
      end if;
    when cparity =>     -- receive parity bit
      if r.rxtick = '1' then
        v.dpar := r.dpar xor rxd; v.rxstate := stopbit;
      end if;
    when stopbit =>     -- receive stop bit
      if r.rxtick = '1' then
        if r.delayirqen = '0' then
          v.irq := v.irq or r.rirqen; -- make sure no tx irqs are lost !
        end if;
        if rxd = '1' then
          if r.delayirqen = '1' then
            v.irqpend := r.rirqen; v.irqcnt := (others => '0');
          end if;
          v.parerr := r.parerr or r.dpar; v.rsempty := r.dpar;
          if not (rfull = '1') and (r.dpar = '0') then
            v.rsempty := '1';
            v.rhold(conv_integer(r.rwaddr)) := r.rshift;
            if fifosize = 1 then v.rcnt(0) := '1';
            else v.rwaddr := r.rwaddr + 1; v.rcnt := v.rcnt + 1; end if;
          end if;
        else
          if r.rshift = "00000000" then
            v.break := '1';
            v.irq := v.irq or r.breakirqen;
          else v.frame := '1'; end if;
          v.rsempty := '1';
        end if;
        v.rxstate := idle;
      end if;
    end case;

    if r.rxtick = '1' then
      v.rtsn := (rfull and not r.rsempty) or r.loopb;
    end if;

    v.txd := r.tshift(0) or r.loopb or r.debug;

    if fifosize /= 1 then
      if thempty = '0' and v.tcnt = rcntzero then
        v.irq := v.irq or r.tirqen;
      end if;
      v.irq := v.irq or (r.tfifoirqen and r.txen and thalffull);
      v.irq := v.irq or (r.rfifoirqen and r.rxen and rhalffull);
      if (r.rfifoirqen and r.rxen and rhalffull) = '1' then
        v.irqpend := '0';
      end if;
    end if;

    v.irq := v.irq or (r.tsemptyirqen and v.tsempty and not r.tsempty); 




    rin <= v;


    uarto.txd <= r.txd; uarto.rtsn <= r.rtsn;
    uarto.scaler <= (others => '0');
    uarto.scaler(sbits-1 downto 0) <= r.scaler;
    uarto.txen <= r.txen; uarto.rxen <= r.rxen;
    uarto.flow <= '0';
  
  end process;


  regs : process(clk)
  begin
    if nrst = '0' then
      r <= RES;
      -- Sync. registers not reset
      r.ctsn <= rin.ctsn;
      r.rxf <= rin.rxf;
    elsif rising_edge(clk) then
      r <= rin;
    end if;
  end process;

end;