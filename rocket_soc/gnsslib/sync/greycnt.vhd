library ieee;
    use ieee.std_logic_1164.all;
    use ieee.std_logic_unsigned.all;
    use ieee.std_logic_arith.all;
    
entity GrayCounter is
    generic (
        generic_width : integer := 4
    );
    port (                            --'Gray' code count output.
        i_nrst : in  std_logic;       -- Count reset.
        i_clk  : in  std_logic;       -- Input clock
        i_ena  : in  std_logic;       -- Count enable.
        o_cnt  : out std_logic_vector (generic_width-1 downto 0)
    );
end entity;

architecture rtl of GrayCounter is
    type regs is record
        bin_cnt  : std_logic_vector (generic_width-1 downto 0);
        grey_cnt : std_logic_vector (generic_width-1 downto 0);
    end record;
    signal r : regs;
begin
    process (i_clk, i_nrst) begin
        if i_nrst = '0' then
            r.bin_cnt  <= conv_std_logic_vector(1, generic_width);
            r.grey_cnt <= (others=>'0');
        elsif (rising_edge(i_clk)) then
            if i_ena = '1' then
                r.bin_cnt  <= r.bin_cnt + 1;
                r.grey_cnt <= r.bin_cnt(generic_width-1) & 
                                 (r.bin_cnt(generic_width-2 downto 0) xor 
                                  r.bin_cnt(generic_width-1 downto 1));
            end if;
        end if;
    end process;
    
    o_cnt <= r.grey_cnt;
    
end architecture;
