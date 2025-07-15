//----------------------------------------------------------------------------
//  Output     Output      Phase    Duty Cycle   Pk-to-Pk     Phase
//   Clock     Freq (MHz)  (degrees)    (%)     Jitter (ps)  Error (ps)
//----------------------------------------------------------------------------
// o_clk_sys__40.00000______0.000______50.0______135.255_____89.971
// o_clk_ddr__200.00000______0.000______50.0_______98.146_____89.971
//
//----------------------------------------------------------------------------
// Input Clock   Freq (MHz)    Input Jitter (UI)
//----------------------------------------------------------------------------
// __primary_________200.000____________0.010

`timescale 1ps/1ps

module SysPLL_tech
 (
  input         i_reset,
  input         i_clk_tcxo,
  output        o_clk_sys,       // 40 MHz
  output        o_clk_ddr,       // 200
  output        o_clk_ddr_phy,   // 800 (4:1 to ddr for UberDDR3)
  output        o_locked
 );
  // Input buffering
  //------------------------------------
wire i_clk_tcxo_clk_wiz_0;
wire clk_in2_clk_wiz_0;

  assign i_clk_tcxo_clk_wiz_0 = i_clk_tcxo;




  // Clocking PRIMITIVE
  //------------------------------------

  // Instantiation of the MMCM PRIMITIVE
  //    * Unused inputs are tied off
  //    * Unused outputs are labeled unused

  wire        w_clk_sys_unbuf;
  wire        w_clk_ddr_unbuf;
  wire        w_clk_ddr_phy_unbuf;
  wire        clkout3_unused;
  wire        clkout4_unused;
  wire        clk_out6_clk_wiz_0;
  wire        clk_out7_clk_wiz_0;

  wire [15:0] do_unused;
  wire        drdy_unused;
  wire        psdone_unused;
  wire        locked_int;
  wire        clkfbout_clk_wiz_0;
  wire        clkfbout_buf_clk_wiz_0;
  wire        clkfboutb_unused;
  wire clkout0b_unused;
  wire clkout1b_unused;
  wire clkout2b_unused;
  wire clkout3b_unused;
  wire        clkout5_unused;
  wire        clkout6_unused;
  wire        clkfbstopped_unused;
  wire        clkinstopped_unused;
  wire        reset_high;

  MMCME2_ADV
  #(.BANDWIDTH            ("OPTIMIZED"),
    .CLKOUT4_CASCADE      ("FALSE"),
    .COMPENSATION         ("ZHOLD"),
    .STARTUP_WAIT         ("FALSE"),
    .DIVCLK_DIVIDE        (1),
    .CLKFBOUT_MULT_F      (4.000),
    .CLKFBOUT_PHASE       (0.000),
    .CLKFBOUT_USE_FINE_PS ("FALSE"),
    .CLKOUT0_DIVIDE_F     (20.000),    // sys = 40 MHz
    .CLKOUT0_PHASE        (0.000),
    .CLKOUT0_DUTY_CYCLE   (0.500),
    .CLKOUT0_USE_FINE_PS  ("FALSE"),
    .CLKOUT1_DIVIDE       (4),         // ddr = 200 MHz
    .CLKOUT1_PHASE        (0.000),
    .CLKOUT1_DUTY_CYCLE   (0.500),
    .CLKOUT1_USE_FINE_PS  ("FALSE"),
    .CLKIN1_PERIOD        (5.000),
    .CLKOUT2_DIVIDE       (1),         // ddr_phy = 800 MHz
    .CLKOUT2_PHASE        (0.000),
    .CLKOUT2_DUTY_CYCLE   (0.500),
    .CLKOUT2_USE_FINE_PS  ("FALSE"),
    .CLKIN2_PERIOD        (5.000))

  mmcm_adv_inst
    // Output clocks
   (
    .CLKFBOUT            (clkfbout_clk_wiz_0),
    .CLKFBOUTB           (clkfboutb_unused),
    .CLKOUT0             (w_clk_sys_unbuf),
    .CLKOUT0B            (clkout0b_unused),
    .CLKOUT1             (w_clk_ddr_unbuf),
    .CLKOUT1B            (clkout1b_unused),
    .CLKOUT2             (clkout2_unused),
    .CLKOUT2B            (w_clk_ddr_phy_unbuf),
    .CLKOUT3             (clkout3_unused),
    .CLKOUT3B            (clkout3b_unused),
    .CLKOUT4             (clkout4_unused),
    .CLKOUT5             (clkout5_unused),
    .CLKOUT6             (clkout6_unused),
     // Input clock control
    .CLKFBIN             (clkfbout_buf_clk_wiz_0),
    .CLKIN1              (i_clk_tcxo_clk_wiz_0),
    .CLKIN2              (1'b0),
     // Tied to always select the primary input clock
    .CLKINSEL            (1'b1),
    // Ports for dynamic reconfiguration
    .DADDR               (7'h0),
    .DCLK                (1'b0),
    .DEN                 (1'b0),
    .DI                  (16'h0),
    .DO                  (do_unused),
    .DRDY                (drdy_unused),
    .DWE                 (1'b0),
    // Ports for dynamic phase shift
    .PSCLK               (1'b0),
    .PSEN                (1'b0),
    .PSINCDEC            (1'b0),
    .PSDONE              (psdone_unused),
    // Other control and status signals
    .LOCKED              (locked_int),
    .CLKINSTOPPED        (clkinstopped_unused),
    .CLKFBSTOPPED        (clkfbstopped_unused),
    .PWRDWN              (1'b0),
    .RST                 (reset_high));
  assign reset_high = i_reset; 

  assign o_locked = locked_int;
// Clock Monitor clock assigning
//--------------------------------------
 // Output buffering
  //-----------------------------------

  BUFG clkf_buf
   (.O (clkfbout_buf_clk_wiz_0),
    .I (clkfbout_clk_wiz_0));




  BUFG clkout1_buf
   (.O   (o_clk_sys),
    .I   (w_clk_sys_unbuf));


  BUFG clkout2_buf
   (.O   (o_clk_ddr),
    .I   (w_clk_ddr_unbuf));

  BUFG clkout3_buf
   (.O   (o_clk_ddr_phy),
    .I   (w_clk_ddr_phy_unbuf));


endmodule
