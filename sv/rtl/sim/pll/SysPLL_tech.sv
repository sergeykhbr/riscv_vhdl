////////////////////////////////////////////////////////////////////////////-
//! @author    Sergey Khabarov - sergeykhbr@gmail.com
//! @brief     Implementation of the SysPLL_tech entity
//////////////////////////////////////////////////////////////////////////////

//! @brief   SysPLL_tech entity declaration ("Virtual" PLL).
//! @details This module instantiates the certain PLL implementation
//!          depending generic technology argument.
module SysPLL_tech (
  //! Reset value. Active high.
  input         i_reset,
  //! Input clock from the external oscillator (default 200 MHz)
  input         i_clk_tcxo,
  //! System Bus clock 100MHz/40MHz (Virtex6/Spartan6)
  output        o_clk_sys, // 40 MHz
  output        o_clk_ddr, // 200 MHz
  //! PLL locked status.
  output        o_locked
);

  logic w_clk_sys;
  logic w_clk_ddr;
  logic w_locked;

  initial begin
    w_clk_sys = 0;
    w_clk_ddr = 0;
    w_locked = 0;
  end

  always #12.5ns w_clk_sys=~w_clk_sys;   // half period of system clock = 40 MHz
  always #2.5ns w_clk_ddr=~w_clk_ddr;    // half period of ddr clock = 200 MHz
  assign w_locked = #100ns 1'b1;

  assign o_clk_sys = w_clk_sys;
  assign o_clk_ddr = w_clk_ddr;
  assign o_locked = w_locked;

endmodule
