//////////////////////////////////////////////////////////////////////////////
// User entered comments
//////////////////////////////////////////////////////////////////////////////
// None
//
//////////////////////////////////////////////////////////////////////////////
// "Output    Output      Phase     Duty      Pk-to-Pk        Phase"
// "Clock    Freq (MHz) (degrees) Cycle (%) Jitter (ps)  Error (ps)"
//////////////////////////////////////////////////////////////////////////////
// CLK_OUT1____40.000______0.000______50.0______xxx.xxx_____xx.xxx
//
//////////////////////////////////////////////////////////////////////////////
// "Input Clock   Freq (MHz)    Input Jitter (UI)"
//////////////////////////////////////////////////////////////////////////////
// __primary_________200.000____________0.010

`ifndef WF_CLKIN_PERIOD
  `define WF_CLKIN_PERIOD 5.0
`endif

// WF_CPU_CLOCK valid values (MHz): 40
`define WF_DEFAULT_CPU_CLOCK 40.0

`ifndef WF_CPU_CLOCK
  `define WF_CPU_CLOCK `WF_DEFAULT_CPU_CLOCK
`endif

module SysPLL_kc705
#(
//   parameter o_clk_bus_buf = "NOBUF"
   parameter o_clk_bus_buf = "BUF"
)(
  //! Reset value. Active high.
  input         i_reset,
  //! Input clock from the external oscillator (default 200 MHz)
  input         i_clk_tcxo,
  //! System Bus clock
  output        o_clk_bus,
  //! PLL locked status.
  output        o_locked
);

    localparam real MMCM_CLKOUT0_DIVIDE_F = 1000.0 / `WF_CPU_CLOCK ;
    
    logic         clkfbout;
    logic         clkfboutb_unused;
    logic         clkout0;
    logic         clkout0b_unused;
    logic         clkout1_unused;
    logic         clkout1b_unused;
    logic         clkout2_unused;
    logic         clkout2b_unusedc;
    logic         clkout3_unused;
    logic         clkout3b_unused;
    logic         clkout4_unused;
    logic         clkout5_unused;
    logic         clkout6_unused;
    logic [15:0]  do_unused;
    logic         drdy_unused;
    logic         psdone_unused;
    logic         clkfbstopped_unused;
    logic         clkinstopped_unused;

    MMCME2_ADV
    #(
      .BANDWIDTH            ("OPTIMIZED"),
      .CLKOUT4_CASCADE      ("FALSE"),
      .COMPENSATION         ("ZHOLD"),
      .STARTUP_WAIT         ("FALSE"),
      .DIVCLK_DIVIDE        (1),
      .CLKFBOUT_MULT_F      (5.000),
      .CLKFBOUT_PHASE       (0.000),
      .CLKFBOUT_USE_FINE_PS ("FALSE"),
      .CLKOUT0_DIVIDE_F     (MMCM_CLKOUT0_DIVIDE_F),
      .CLKOUT0_PHASE        (0.000),
      .CLKOUT0_DUTY_CYCLE   (0.500),
      .CLKOUT0_USE_FINE_PS  ("FALSE"),
      .CLKIN1_PERIOD        (`WF_CLKIN_PERIOD),
      .REF_JITTER1          (0.010)
      )
    mmcm_adv_inst (
      .CLKFBOUT            (clkfbout),
      .CLKFBOUTB           (clkfboutb_unused),
      .CLKOUT0             (clkout0),
      .CLKOUT0B            (clkout0b_unused),
      .CLKOUT1             (clkout1_unused),
      .CLKOUT1B            (clkout1b_unused),
      .CLKOUT2             (clkout2_unused),
      .CLKOUT2B            (clkout2b_unused),
      .CLKOUT3             (clkout3_unused),
      .CLKOUT3B            (clkout3b_unused),
      .CLKOUT4             (clkout4_unused),
      .CLKOUT5             (clkout5_unused),
      .CLKOUT6             (clkout6_unused),
      .CLKFBIN             (clkfbout),
      .CLKIN1              (i_clk_tcxo),
      .CLKIN2              (1'b0),
      .CLKINSEL            (1'b1),
      .DADDR               ('0),
      .DCLK                (1'b0),
      .DEN                 (1'b0),
      .DI                  ('0),
      .DO                  (do_unused),
      .DRDY                (drdy_unused),
      .DWE                 (1'b0),
      .PSCLK               (1'b0),
      .PSEN                (1'b0),
      .PSINCDEC            (1'b0),
      .PSDONE              (psdone_unused),
      .LOCKED              (o_locked),
      .CLKINSTOPPED        (clkinstopped_unused),
      .CLKFBSTOPPED        (clkfbstopped_unused),
      .PWRDWN              (1'b0),
      .RST                 (i_reset)
    );

  generate
    if (o_clk_bus_buf == "NOBUF") begin: clk_bus_nobuf
      assign o_clk_bus = clkout0;
    end else begin: clk_bus_buf
      BUFG o_clk_bus_buf (.O(o_clk_bus), .I(clkout0));
    end
  endgenerate


endmodule : SysPLL_kc705
