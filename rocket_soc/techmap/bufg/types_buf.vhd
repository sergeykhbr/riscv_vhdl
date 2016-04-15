----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Declaration types_buf package components.
------------------------------------------------------------------------------
--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--! Technology constants definition.
library techmap;
use techmap.gencomp.all;

--! @brief      Declaration of 'virtual' Buffers components.
package types_buf is

  --! @brief Clock signals multiplexer.
  --! @param[in] tech  Technology selector.
  --! @param[out] O    Output clock signal.
  --! @param[in] I1    Input clock signal 1.
  --! @param[in] I2    Input clock signal 2.
  --! @param[in] S     Input signals switcher: 
  --!                      0 = I1; 1 = I2.
  component bufgmux_tech is
    generic (
      tech : integer := 0;
      tmode_always_ena : boolean := false
    );
    port (
      O        : out std_ulogic;
      I1       : in std_ulogic;
      I2       : in std_ulogic;
      S        : in std_ulogic);
  end component;

  --! @brief Input PAD buffer.
  --! @details This buffer makes sense only for ASIC implementation.
  --! @param[in] tech  Technology selector.
  --! @param[out] o    Output buffered signal.
  --! @param[in] i     Input unbuffered signal.
  component ibuf_tech is generic (generic_tech : integer := 0);
  port (
    o  : out std_logic;
    i  : in std_logic
  );
  end component; 
  
  --! @brief Output PAD buffer.
  --! @details This buffer makes sense only for ASIC implementation.
  --! @param[in] tech  Technology selector.
  --! @param[out] o    Output signal directly connected to the ASIC output pin.
  --! @param[in] i     Input signal.
  component obuf_tech is generic (generic_tech : integer := 0);
  port (
    o  : out std_logic;
    i  : in std_logic
  );
  end component; 

  --! @brief Input/Output PAD buffer.
  --! @param[in]    tech Technology selector.
  --! @param[out]   o    Output signal 
  --! @param[inout] io   Bi-directional signal.
  --! @param[in]    i    Input signal 
  --! @param[in]    t    Controlling signal: 0 = in; 1=out
  --!
  --! Example:
  --! @code
  --!    entity foo is port (
  --!       io_gpio : inout std_logic
  --!    )
  --!    end foo;
  --!    architecture rtl of foo is
  --!      signal ob_gpio_direction : std_logic;
  --!      signal ob_gpio_opins     : std_logic;
  --!      signal ib_gpio_ipins     : std_logic;
  --!      ...
  --!    begin
  --!      ob_gpio_direction <= '1';
  --!
  --!      iob   : iobuf_tech generic map(kintex7) 
  --!          port map (ib_gpio_ipins, io_gpio, ob_gpio_opins, ob_gpio_direction);
  --! 
  --!      reg : process(clk, nrst) begin
  --!         if rising_edge(clk) then 
  --!            reg1 <= ib_gpio_ipins;
  --!            ob_gpio_opins <= reg2;
  --!         end;
  --!      end process;
  --!    end;
  --! @endcode
  component iobuf_tech is generic (generic_tech : integer := 0);
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
  end component;

  --! @brief Gigabit buffer with differential inputs.
  --! @param[in] gclk_p Differential clock input.
  --! @param[in] gclk_n Differential clock inversed input.
  --! @param[out] o_clk Unbuffered clock output.
  component igdsbuf_tech is
  generic (
    generic_tech : integer := 0
  );
  port (
    gclk_p : in std_logic;
    gclk_n : in std_logic;
    o_clk  : out std_logic
  );
  end component; 


end;
