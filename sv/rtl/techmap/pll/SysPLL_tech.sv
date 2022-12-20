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

import config_target_pkg::*;

`ifdef TARGET_INFERRED

    SysPLL_inferred inf0
    (
        .CLK_IN(i_clk_tcxo),
        .CLK_OUT1(o_clk_sys),
        .CLK_OUT2(o_clk_ddr),
        .RESET(i_reset),
        .LOCKED(o_locked)
    );

`elsif TARGET_KC705

    SysPLL_kc705 kc705
    (
      .i_reset     (i_reset),
      .i_clk_tcxo  (i_clk_tcxo),
      .o_clk_sys   (o_clk_sys),
      .o_clk_ddr   (o_clk_ddr),
      .o_locked    (o_locked)
    );
       
`else
   
   $error("WF_SYSPLL_INSTANCE macro is undefined.");
       
`endif //WF_USE_INFERRED_SYSPLL

endmodule