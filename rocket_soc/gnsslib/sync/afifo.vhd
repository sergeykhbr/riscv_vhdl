library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
    
entity afifo is
    generic (
        abits : integer := 4;
        dbits : integer := 8
    );
    port (
        i_nrst      : in  std_logic;
        -- Reading port.
        i_rclk      : in  std_logic;
        i_rd_ena    : in  std_logic;
        o_data      : out std_logic_vector (dbits-1 downto 0);
        o_empty     : out std_logic;
        -- Writing port.
        i_wclk      : in  std_logic;
        i_wr_ena    : in  std_logic;
        i_data      : in  std_logic_vector (dbits-1 downto 0);
        o_full      : out std_logic
	 
    );
end entity;
architecture rtl of afifo is
    ----/Internal connections & variables------
    constant FIFO_DEPTH :integer := 2**abits;

    type RAM is array (integer range <>)of std_logic_vector (dbits-1 downto 0);
    signal Mem : RAM (0 to FIFO_DEPTH-1);
    
    signal pNextWordToWrite     :std_logic_vector (abits-1 downto 0);
    signal pNextWordToRead      :std_logic_vector (abits-1 downto 0);
    signal EqualAddresses       :std_logic;
    signal NextWriteAddressEn   :std_logic;
    signal NextReadAddressEn    :std_logic;
    signal Set_Status           :std_logic;
    signal Rst_Status           :std_logic;
    signal Status               :std_logic;
    signal PresetFull           :std_logic;
    signal PresetEmpty          :std_logic;
    signal empty,full           :std_logic;
    signal r_stat               :std_logic;
    
    component GrayCounter is
    generic (
        generic_width : integer := 4
    );
    port (                            --'Gray' code count output.
        i_nrst : in  std_logic;       -- Count reset.
        i_clk  : in  std_logic;       -- Input clock
        i_ena  : in  std_logic;       -- Count enable.
        o_cnt  : out std_logic_vector (generic_width-1 downto 0)
    );
    end component;
begin

    process (i_rclk) begin
        if (rising_edge(i_rclk)) then
            if (i_rd_ena = '1' and empty = '0') then
                o_data <= Mem(conv_integer(pNextWordToRead));
            end if;
        end if;
    end process;
            
    --'Data_in' logic:
    process (i_wclk) begin
        if (rising_edge(i_wclk)) then
            if (i_wr_ena = '1' and full = '0') then
                Mem(conv_integer(pNextWordToWrite)) <= i_data;
            end if;
        end if;
    end process;

    --Fifo addresses support logic: 
    NextWriteAddressEn <= i_wr_ena and (not full);
    NextReadAddressEn  <= i_rd_ena and (not empty);
           
    --Addreses (Gray counters) logic:
    GrayCounter_pWr : GrayCounter
    generic map (
        generic_width => abits
    ) port map (
        i_nrst  => i_nrst,
        i_clk   => i_wclk,
        i_ena   => NextWriteAddressEn,
        o_cnt   => pNextWordToWrite
    );
       
    GrayCounter_pRd : GrayCounter
    generic map (
        generic_width => abits
    ) port map (
        i_nrst  => i_nrst,
        i_clk   => i_rclk,
        i_ena   => NextReadAddressEn,
        o_cnt   => pNextWordToRead
    );

    --'EqualAddresses' logic:
    EqualAddresses <= '1' when (pNextWordToWrite = pNextWordToRead) else '0';

    --'Quadrant selectors' logic:
    process (pNextWordToWrite, pNextWordToRead)
        variable set_status_bit0 :std_logic;
        variable set_status_bit1 :std_logic;
        variable rst_status_bit0 :std_logic;
        variable rst_status_bit1 :std_logic;
    begin
        set_status_bit0 := pNextWordToWrite(abits-2) xnor pNextWordToRead(abits-1);
        set_status_bit1 := pNextWordToWrite(abits-1) xor  pNextWordToRead(abits-2);
        Set_Status <= set_status_bit0 and set_status_bit1;
        
        rst_status_bit0 := pNextWordToWrite(abits-2) xor  pNextWordToRead(abits-1);
        rst_status_bit1 := pNextWordToWrite(abits-1) xnor pNextWordToRead(abits-2);
        Rst_Status      <= rst_status_bit0 and rst_status_bit1;
    end process;
    
    --'Status' latch logic:
    r_stat <= Rst_Status or (not i_nrst);
    process (i_rclk, Set_Status, r_stat) begin--D Latch w/ Asynchronous Clear & Preset.
        if r_stat = '1' then 
            Status <= '0';
        elsif (Set_Status = '1') then
            Status <= '1';  --Going 'Full'.
        elsif rising_edge(i_rclk) then
            Status <= Status;
        end if;
    end process;
    
    --'Full_out' logic for the writing port:
    PresetFull <= Status and EqualAddresses;  --'Full' Fifo.
    
    process (i_wclk, PresetFull) begin --D Flip-Flop w/ Asynchronous Preset.
        if (PresetFull = '1') then
            full <= '1';
        elsif (rising_edge(i_wclk)) then
            full <= '0';
        end if;
    end process;
    o_full <= full;
    
    --'Empty_out' logic for the reading port:
    PresetEmpty <= not Status and EqualAddresses;  --'Empty' Fifo.
    
    process (i_rclk, PresetEmpty) begin --D Flip-Flop w/ Asynchronous Preset.
        if (PresetEmpty = '1') then
            empty <= '1';
        elsif (rising_edge(i_rclk)) then
            empty <= '0';
        end if;
    end process;
    
    o_empty <= empty;
end architecture;
