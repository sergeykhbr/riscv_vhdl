-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Plug'n'Play support device with AXI4 interface.
------------------------------------------------------------------------------

--! @defgroup pnp_page Plug'n'Play support module
--! @ingroup peripheries_group
--! 
--! @section pnp_regs PNP registers mapping
--! PNP module acts like a slave AMBA AXI4 device that is directly mapped
--! into physical memory. Default address location for our implementation 
--! is defined as 0xFFFFF000. Memory size is 4 KB.
--!
--! @par HW ID register (0x000).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 32 | RO | CFG_HW_ID  | hw_id  | 31:0   | <b>HW ID</b>. Read only SoC identificator. Now it contains manually specified date in hex-format. Can be changed via CFG_HW_ID configuration parameter.
--!
--! @par FW ID register (0x004).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 32 | RW | 32'h0  | fw_id  | 31:0   | <b>Firmware ID</b>. This value is modified by bootloader or user's firmware. Can be used to simplify firmware version tracking.
--!
--! @par AXI Slots Configuration Register (0x008).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 8  | RO | CFG_TECH  | tech  | 7:0   | <b>Technology ID</b>. Read Only value specifies the target configuration. Possible values: inferred, virtex6, kintex7. Other targets ID could be added in a future.
--! | 8  | RO | CFG_NASTI_SLAVES_TOTAL  | slaves  | 15:8   | <b>Total number of AXI slave slots</b>. This value specifies maximum number of slave devices connected to the system bus. If device wasn't connected the dummy signals must be applied to the slave interface otherwise SoC behaviour isn't defined.
--! | 8  | RO | CFG_NASTI_MASTER_TOTAL  | masters  | 23:16   | <b>Total number of AXI master slots</b>. This value specifies maximum number of master devices connected to the system bus. Slot signals cannot be unconnected either.
--! | 8  | RO | 8'h0  | adc_detect  | 31:24   | <b>ADC clock detector</b>. This value is used by GNSS firmware to detect presence of the ADC clock frequency that allows to detect presence of the RF front-end board.
--!
--! @par Debug IDT register (0x010).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64'h0  | idt  | 63:0   | <b>Debug IDT</b>. This is debug register used by GNSS firmware to store debug information.
--!
--! @par Debug Memory Allocation Pointer register (0x018).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64'h0  | malloc_addr  | 63:0   | <b>Memory Allocation Pointer</b>. This is debug register used by GNSS firmware to store 'heap' pointer and allows to debug memory management.
--!
--! @par Debug Memory Allocation Size register (0x020).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64'h0  | malloc_size  | 63:0   | <b>Memory Allocation size</b>. This is debug register used by GNSS firmware to store total allocated memory size.
--!
--! @par Debug Firmware1 register (0x028).
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64'h0  | fwdbg1  | 63:0   | <b>Firmware debug1</b>. This is debug register used by GNSS firmware to store temporary information.
--! 
--! @section pnp_descriptior PNP Device descriptors
--! Our SoC implementaion provides capability to read in real-time information
--! about mapped devices. Such information is packed into special device
--! descriptors. Now we can provide 3 types of descriptors:
--!    - Master device descriptor
--!    - Slave device descriptor
--!    - Custom device descriptor
--!
--! All descriptors mapped sequentually starting from 0xFFFFF040. Each descriptor
--! implements field 'size' in Bytes that specifies offset to the next mapped
--! descriptor.
--!
--! @par Master device descriptor
--! 
--! | Bits    | Description 
--! |:-------:|:------------------------------------------------------------|
--! | [7:0]   | <b>Descriptor Size.</b> Read Only value specifies size in Bytes of the current descriptor. This value should be used as offset to the next descriptor. Master descriptor size is hardwired to PNP_CFG_MASTER_DESCR_BYTES value (8'h08).
--! | [9:8]   | <b>Descriptor Type.</b> Master descriptor type is hardwired to PNP_CFG_TYPE_MASTER value (2'b01).
--! | [31:10] | <b>Reserved.</b>
--! | [47:32] | <b>Device ID.</b> Unique Master identificator.
--! | [63:48] | <b>Vendor ID.</b> Unique Vendor identificator.
--!
--! @par Slave device descriptor
--! 
--! | Bits      | Description 
--! |:---------:|:------------------------------------------------------------|
--! | [7:0]     | <b>Descriptor Size.</b> Read Only value specifies size in Bytes of the current descriptor. This value should be used as offset to the next descriptor. Slave descriptor size is hardwired to PNP_CFG_SLAVE_DESCR_BYTES value (8'h10).
--! | [9:8]     | <b>Descriptor Type.</b> Slave descriptor type is hardwired to PNP_CFG_TYPE_SLAVE value (2'b10).
--! | [15:10]   | <b>Reserved.</b>
--! | [23:16]   | <b>IRQ ID.</b> Interrupt line index assigned to the device.
--! | [31:24]   | <b>Reserved.</b>
--! | [47:32]   | <b>Device ID.</b> Unique Master identificator.
--! | [63:48]   | <b>Vendor ID.</b> Unique Vendor identificator.
--! | [75:64]   | <b>zero.</b> Hardwired to X"000".
--! | [95:76]   | <b>Base Address Mask</b> specifies the memory region allocated for the device.
--! | [107:96]  | <b>zero.</b> Hardwired to X"000".
--! | [127:108] | <b>Base Address</b> value of the device.
--!

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;


--! @brief Hardware Configuration storage with the AMBA AXI4 interface.
entity nasti_pnp is
  generic (
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0;
    hw_id   : std_logic_vector(31 downto 0) := X"20170101"
  );
  port (
    sys_clk : in  std_logic;
    adc_clk : in  std_logic;
    nrst   : in  std_logic;
    mstcfg : in  nasti_master_cfg_vector;
    slvcfg : in  nasti_slave_cfg_vector;
    cfg    : out  nasti_slave_config_type;
    i      : in  nasti_slave_in_type;
    o      : out nasti_slave_out_type
  );
end; 
 
architecture arch_nasti_pnp of nasti_pnp is

  constant xconfig : nasti_slave_config_type := (
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_SLAVE,
     irq_idx => 0,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_PNP
  );

  type local_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of integer;

  type master_config_map is array (0 to 2*CFG_NASTI_MASTER_TOTAL-1)
       of std_logic_vector(31 downto 0);
       
  type slave_config_map is array (0 to 4*CFG_NASTI_SLAVES_TOTAL-1)
       of std_logic_vector(31 downto 0);

  type bank_type is record
    fw_id : std_logic_vector(31 downto 0);
    idt : std_logic_vector(63 downto 0); --! debug counter
    malloc_addr : std_logic_vector(63 downto 0); --! dynamic allocation addr
    malloc_size : std_logic_vector(63 downto 0); --! dynamic allocation size
    fwdbg1 : std_logic_vector(63 downto 0); --! FW marker for the debug porposes
    adc_detect : std_logic_vector(7 downto 0);
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

  constant RESET_VALUE : registers := (
        NASTI_SLAVE_BANK_RESET,
        ((others => '0'), (others => '0'), (others => '0'),
         (others => '0'), (others => '0'), (others => '0'))
  );

  signal r, rin : registers;
  --! @brief   Detector of the ADC clock.
  --! @details If this register won't equal to 0xFF, then we suppose RF front-end
  --!          not connected and FW should print message to enable 'i_int_clkrf'
  --!          jumper to make possible generation of the 1 msec interrupts.
  signal r_adc_detect : std_logic_vector(7 downto 0);

begin

  comblogic : process(i, slvcfg, mstcfg, r, r_adc_detect, nrst)
    variable v : registers;
    variable mstmap : master_config_map;
    variable slvmap : slave_config_map;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable rtmp : std_logic_vector(31 downto 0);
    variable wtmp : std_logic_vector(31 downto 0);
    variable wstrb : std_logic_vector(CFG_ALIGN_BYTES-1 downto 0);
  begin

    v := r;
    v.bank0.adc_detect := r_adc_detect;

    for k in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
      mstmap(2*k) := "00" & X"00000" & mstcfg(k).descrtype & mstcfg(k).descrsize;
      mstmap(2*k+1) := mstcfg(k).vid & mstcfg(k).did;
    end loop;

    for k in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
      slvmap(4*k) := X"00" & 
                     conv_std_logic_vector(slvcfg(k).irq_idx,8) & "000000" &
                     slvcfg(k).descrtype & slvcfg(k).descrsize;
      slvmap(4*k+1) := slvcfg(k).vid & slvcfg(k).did;
      slvmap(4*k+2)   := slvcfg(k).xmask & X"000";
      slvmap(4*k+3) := slvcfg(k).xaddr & X"000";
    end loop;


    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       raddr_reg(n) := conv_integer(r.bank_axi.raddr(n)(11 downto 2));

       rtmp := (others => '0');
       if raddr_reg(n) = 0 then 
          rtmp := hw_id;
       elsif raddr_reg(n) = 1 then 
          rtmp := r.bank0.fw_id;
       elsif raddr_reg(n) = 2 then 
          rtmp := r.bank0.adc_detect 
              & conv_std_logic_vector(CFG_NASTI_MASTER_TOTAL,8)
              & conv_std_logic_vector(CFG_NASTI_SLAVES_TOTAL,8)
              & conv_std_logic_vector(tech,8);
       elsif raddr_reg(n) = 3 then 
          -- reserved
       elsif raddr_reg(n) = 4 then 
          rtmp := r.bank0.idt(31 downto 0);
       elsif raddr_reg(n) = 5 then 
          rtmp := r.bank0.idt(63 downto 32);
       elsif raddr_reg(n) = 6 then 
          rtmp := r.bank0.malloc_addr(31 downto 0);
       elsif raddr_reg(n) = 7 then 
          rtmp := r.bank0.malloc_addr(63 downto 32);
       elsif raddr_reg(n) = 8 then 
          rtmp := r.bank0.malloc_size(31 downto 0);
       elsif raddr_reg(n) = 9 then 
          rtmp := r.bank0.malloc_size(63 downto 32);
       elsif raddr_reg(n) = 10 then 
          rtmp := r.bank0.fwdbg1(31 downto 0);
       elsif raddr_reg(n) = 11 then 
          rtmp := r.bank0.fwdbg1(63 downto 32);
       elsif raddr_reg(n) >= 16 and raddr_reg(n) < 16+2*CFG_NASTI_MASTER_TOTAL then
          rtmp := mstmap(raddr_reg(n) - 16);
       elsif raddr_reg(n) >= 16+2*CFG_NASTI_MASTER_TOTAL 
             and raddr_reg(n) < 16+2*CFG_NASTI_MASTER_TOTAL+4*CFG_NASTI_SLAVES_TOTAL then
          rtmp := slvmap(raddr_reg(n) - 16 - 2*CFG_NASTI_MASTER_TOTAL);
       end if;
       
       rdata(32*(n+1)-1 downto 32*n) := rtmp;
    end loop;


    if i.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(n)(11 downto 2));
         wtmp := i.w_data(32*(n+1)-1 downto 32*n);
         wstrb := i.w_strb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n);

         if conv_integer(wstrb) /= 0 then
           case waddr_reg(n) is
             when 1 => v.bank0.fw_id := wtmp;
             when 4 => v.bank0.idt(31 downto 0) := wtmp;
             when 5 => v.bank0.idt(63 downto 32) := wtmp;
             when 6 => v.bank0.malloc_addr(31 downto 0) := wtmp;
             when 7 => v.bank0.malloc_addr(63 downto 32) := wtmp;
             when 8 => v.bank0.malloc_size(31 downto 0) := wtmp;
             when 9 => v.bank0.malloc_size(63 downto 32) := wtmp;
             when 10 => v.bank0.fwdbg1(31 downto 0) := wtmp;
             when 11 => v.bank0.fwdbg1(63 downto 32) := wtmp;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    o <= functionAxi4Output(r.bank_axi, rdata);

    if nrst = '0' then
        v := RESET_VALUE;
    end if;
    
    rin <= v;
  end process;

  cfg <= xconfig;

  -- registers:
  regs : process(sys_clk)
  begin 
     if rising_edge(sys_clk) then 
        r <= rin;
     end if; 
  end process;

  -- ADC clock detector:
  regsadc : process(adc_clk)
  begin 
     if rising_edge(adc_clk) then 
        if nrst = '0' then
            r_adc_detect <= (others => '0');
        else
            r_adc_detect <= r_adc_detect(6 downto 0) & nrst;
        end if;
     end if; 
  end process;

end;
