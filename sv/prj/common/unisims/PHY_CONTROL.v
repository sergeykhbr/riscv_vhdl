///////////////////////////////////////////////////////
//  Copyright (c) 2009 Xilinx Inc.
//  All Right Reserved.
///////////////////////////////////////////////////////
//
//   ____   ___
//  /   /\/   / 
// /___/  \  /     Vendor      : Xilinx 
// \  \    \/      Version     :  12.1
//  \  \           Description : Xilinx Simulation Library Component
//  /  /                         7SERIES Phaser Phy Control
// /__/   /\       Filename    : PHY_CONTROL.v
// \  \  /  \ 
//  \__\/\__ \                    
//                                 
//  Date:     Comment:
//  23APR2010 Initial UNI/UNP/SIM version from yml
//  10JUN2010 yml update
//  02JUL2010 enable secureip
//  20AUG2010 yml, rtl update
//  28SEP2010 yml, rtl update
//  29SEP2010 yml update
//  28OCT2010 rtl update
//  05NOV2010 update defaults
//  11JAN2011 586040 correct spelling XIL_TIMING vs XIL_TIMIMG
//  14FEB2011 593832 yml, rtl update
//  16MAR2011 601917 dly.yml update
//  14APR2011 606310 yml update
//  15AUG2011 621681 remove SIM_SPEEDUP, make default
///////////////////////////////////////////////////////

`timescale 1 ps / 1 ps 

`celldefine

module PHY_CONTROL (
  AUXOUTPUT,
  INBURSTPENDING,
  INRANKA,
  INRANKB,
  INRANKC,
  INRANKD,
  OUTBURSTPENDING,
  PCENABLECALIB,
  PHYCTLALMOSTFULL,
  PHYCTLEMPTY,
  PHYCTLFULL,
  PHYCTLREADY,

  MEMREFCLK,
  PHYCLK,
  PHYCTLMSTREMPTY,
  PHYCTLWD,
  PHYCTLWRENABLE,
  PLLLOCK,
  READCALIBENABLE,
  REFDLLLOCK,
  RESET,
  SYNCIN,
  WRITECALIBENABLE
);

`ifdef XIL_TIMING
  parameter LOC = "UNPLACED";
`endif
  parameter integer AO_TOGGLE = 0;
  parameter [3:0] AO_WRLVL_EN = 4'b0000;
  parameter BURST_MODE = "FALSE";
  parameter integer CLK_RATIO = 1;
  parameter integer CMD_OFFSET = 0;
  parameter integer CO_DURATION = 0;
  parameter DATA_CTL_A_N = "FALSE";
  parameter DATA_CTL_B_N = "FALSE";
  parameter DATA_CTL_C_N = "FALSE";
  parameter DATA_CTL_D_N = "FALSE";
  parameter DISABLE_SEQ_MATCH = "TRUE";
  parameter integer DI_DURATION = 0;
  parameter integer DO_DURATION = 0;
  parameter integer EVENTS_DELAY = 63;
  parameter integer FOUR_WINDOW_CLOCKS = 63;
  parameter MULTI_REGION = "FALSE";
  parameter PHY_COUNT_ENABLE = "FALSE";
  parameter integer RD_CMD_OFFSET_0 = 0;
  parameter integer RD_CMD_OFFSET_1 = 00;
  parameter integer RD_CMD_OFFSET_2 = 0;
  parameter integer RD_CMD_OFFSET_3 = 0;
  parameter integer RD_DURATION_0 = 0;
  parameter integer RD_DURATION_1 = 0;
  parameter integer RD_DURATION_2 = 0;
  parameter integer RD_DURATION_3 = 0;
  parameter SYNC_MODE = "FALSE";
  parameter integer WR_CMD_OFFSET_0 = 0;
  parameter integer WR_CMD_OFFSET_1 = 0;
  parameter integer WR_CMD_OFFSET_2 = 0;
  parameter integer WR_CMD_OFFSET_3 = 0;
  parameter integer WR_DURATION_0 = 0;
  parameter integer WR_DURATION_1 = 0;
  parameter integer WR_DURATION_2 = 0;
  parameter integer WR_DURATION_3 = 0;
  
`ifdef XIL_TIMING
  localparam in_delay = 0;
  localparam out_delay = 0;
`else
  localparam in_delay = 1;
  localparam out_delay = 100;
`endif
  localparam INCLK_DELAY = 0;
  localparam OUTCLK_DELAY = 0;
  localparam MODULE_NAME = "PHY_CONTROL";

  output PHYCTLALMOSTFULL;
  output PHYCTLEMPTY;
  output PHYCTLFULL;
  output PHYCTLREADY;
  output [1:0] INRANKA;
  output [1:0] INRANKB;
  output [1:0] INRANKC;
  output [1:0] INRANKD;
  output [1:0] PCENABLECALIB;
  output [3:0] AUXOUTPUT;
  output [3:0] INBURSTPENDING;
  output [3:0] OUTBURSTPENDING;

  input MEMREFCLK;
  input PHYCLK;
  input PHYCTLMSTREMPTY;
  input PHYCTLWRENABLE;
  input PLLLOCK;
  input READCALIBENABLE;
  input REFDLLLOCK;
  input RESET;
  input SYNCIN;
  input WRITECALIBENABLE;
  input [31:0] PHYCTLWD;

  reg [0:0] BURST_MODE_BINARY;
  reg [0:0] DATA_CTL_A_N_BINARY;
  reg [0:0] DATA_CTL_B_N_BINARY;
  reg [0:0] DATA_CTL_C_N_BINARY;
  reg [0:0] DATA_CTL_D_N_BINARY;
  reg [0:0] DISABLE_SEQ_MATCH_BINARY;
  reg [0:0] MULTI_REGION_BINARY;
  reg [0:0] PHY_COUNT_ENABLE_BINARY;
  reg [0:0] SPARE_BINARY;
  reg [0:0] SYNC_MODE_BINARY;
  reg [2:0] CLK_RATIO_BINARY;
  reg [2:0] CO_DURATION_BINARY;
  reg [2:0] DI_DURATION_BINARY;
  reg [2:0] DO_DURATION_BINARY;
  reg [3:0] AO_TOGGLE_BINARY;
  reg [3:0] AO_WRLVL_EN_BINARY;
  reg [5:0] CMD_OFFSET_BINARY;
  reg [5:0] EVENTS_DELAY_BINARY;
  reg [5:0] FOUR_WINDOW_CLOCKS_BINARY;
  reg [5:0] RD_CMD_OFFSET_0_BINARY;
  reg [5:0] RD_CMD_OFFSET_1_BINARY;
  reg [5:0] RD_CMD_OFFSET_2_BINARY;
  reg [5:0] RD_CMD_OFFSET_3_BINARY;
  reg [5:0] RD_DURATION_0_BINARY;
  reg [5:0] RD_DURATION_1_BINARY;
  reg [5:0] RD_DURATION_2_BINARY;
  reg [5:0] RD_DURATION_3_BINARY;
  reg [5:0] WR_CMD_OFFSET_0_BINARY;
  reg [5:0] WR_CMD_OFFSET_1_BINARY;
  reg [5:0] WR_CMD_OFFSET_2_BINARY;
  reg [5:0] WR_CMD_OFFSET_3_BINARY;
  reg [5:0] WR_DURATION_0_BINARY;
  reg [5:0] WR_DURATION_1_BINARY;
  reg [5:0] WR_DURATION_2_BINARY;
  reg [5:0] WR_DURATION_3_BINARY;

  tri0 GSR = glbl.GSR;
`ifdef XIL_TIMING
  reg notifier;
`endif

  initial begin
    case (BURST_MODE)
      "FALSE" : BURST_MODE_BINARY <= 1'b0;
      "TRUE" : BURST_MODE_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute BURST_MODE on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, BURST_MODE);
        #1 $finish;
      end
    endcase

    case (CLK_RATIO)
      1 : CLK_RATIO_BINARY <= 3'b000;
      2 : CLK_RATIO_BINARY <= 3'b001;
      4 : CLK_RATIO_BINARY <= 3'b010;
      8 : CLK_RATIO_BINARY <= 3'b100;
      default : begin
        $display("Attribute Syntax Error : The Attribute CLK_RATIO on %s instance %m is set to %d.  Legal values for this attribute are 1, 2, 4 or 8.", MODULE_NAME, CLK_RATIO, 1);
        #1 $finish;
      end
    endcase

    case (DATA_CTL_A_N)
      "FALSE" : DATA_CTL_A_N_BINARY <= 1'b0;
      "TRUE" : DATA_CTL_A_N_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute DATA_CTL_A_N on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, DATA_CTL_A_N);
        #1 $finish;
      end
    endcase

    case (DATA_CTL_B_N)
      "FALSE" : DATA_CTL_B_N_BINARY <= 1'b0;
      "TRUE" : DATA_CTL_B_N_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute DATA_CTL_B_N on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, DATA_CTL_B_N);
        #1 $finish;
      end
    endcase

    case (DATA_CTL_C_N)
      "FALSE" : DATA_CTL_C_N_BINARY <= 1'b0;
      "TRUE" : DATA_CTL_C_N_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute DATA_CTL_C_N on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, DATA_CTL_C_N);
        #1 $finish;
      end
    endcase

    case (DATA_CTL_D_N)
      "FALSE" : DATA_CTL_D_N_BINARY <= 1'b0;
      "TRUE" : DATA_CTL_D_N_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute DATA_CTL_D_N on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, DATA_CTL_D_N);
        #1 $finish;
      end
    endcase

    case (DISABLE_SEQ_MATCH)
      "TRUE" : DISABLE_SEQ_MATCH_BINARY <= 1'b1;
      "FALSE" : DISABLE_SEQ_MATCH_BINARY <= 1'b0;
      default : begin
        $display("Attribute Syntax Error : The Attribute DISABLE_SEQ_MATCH on %s instance %m is set to %s.  Legal values for this attribute are TRUE or FALSE.", MODULE_NAME, DISABLE_SEQ_MATCH);
        #1 $finish;
      end
    endcase

    case (MULTI_REGION)
      "FALSE" : MULTI_REGION_BINARY <= 1'b0;
      "TRUE" : MULTI_REGION_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute MULTI_REGION on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, MULTI_REGION);
        #1 $finish;
      end
    endcase

    case (PHY_COUNT_ENABLE)
      "FALSE" : PHY_COUNT_ENABLE_BINARY <= 1'b0;
      "TRUE" : PHY_COUNT_ENABLE_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute PHY_COUNT_ENABLE on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, PHY_COUNT_ENABLE);
        #1 $finish;
      end
    endcase

    SPARE_BINARY <= 1'b0;

    case (SYNC_MODE)
      "TRUE" : SYNC_MODE_BINARY <= 1'b1;
      "FALSE" : SYNC_MODE_BINARY <= 1'b0;
      default : begin
        $display("Attribute Syntax Error : The Attribute SYNC_MODE on %s instance %m is set to %s.  Legal values for this attribute are TRUE or FALSE.", MODULE_NAME, SYNC_MODE);
        #1 $finish;
      end
    endcase

    if ((AO_TOGGLE >= 0) && (AO_TOGGLE <= 15))
      AO_TOGGLE_BINARY <= AO_TOGGLE;
    else begin
      $display("Attribute Syntax Error : The Attribute AO_TOGGLE on %s instance %m is set to %d.  Legal values for this attribute are 0 to 15.", MODULE_NAME, AO_TOGGLE);
      #1 $finish;
    end

    if ((AO_WRLVL_EN >= 4'b0000) && (AO_WRLVL_EN <= 4'b1111))
      AO_WRLVL_EN_BINARY <= AO_WRLVL_EN;
    else begin
      $display("Attribute Syntax Error : The Attribute AO_WRLVL_EN on %s instance %m is set to %b.  Legal values for this attribute are 4'b0000 to 4'b1111.", MODULE_NAME, AO_WRLVL_EN);
      #1 $finish;
    end

    if ((CMD_OFFSET >= 0) && (CMD_OFFSET <= 63))
      CMD_OFFSET_BINARY <= CMD_OFFSET;
    else begin
      $display("Attribute Syntax Error : The Attribute CMD_OFFSET on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, CMD_OFFSET);
      #1 $finish;
    end

    if ((CO_DURATION >= 0) && (CO_DURATION <= 7))
      CO_DURATION_BINARY <= CO_DURATION;
    else begin
      $display("Attribute Syntax Error : The Attribute CO_DURATION on %s instance %m is set to %d.  Legal values for this attribute are 0 to 7.", MODULE_NAME, CO_DURATION);
      #1 $finish;
    end

    if ((DI_DURATION >= 0) && (DI_DURATION <= 7))
      DI_DURATION_BINARY <= DI_DURATION;
    else begin
      $display("Attribute Syntax Error : The Attribute DI_DURATION on %s instance %m is set to %d.  Legal values for this attribute are 0 to 7.", MODULE_NAME, DI_DURATION);
      #1 $finish;
    end

    if ((DO_DURATION >= 0) && (DO_DURATION <= 7))
      DO_DURATION_BINARY <= DO_DURATION;
    else begin
      $display("Attribute Syntax Error : The Attribute DO_DURATION on %s instance %m is set to %d.  Legal values for this attribute are 0 to 7.", MODULE_NAME, DO_DURATION);
      #1 $finish;
    end

    if ((EVENTS_DELAY >= 0) && (EVENTS_DELAY <= 63))
      EVENTS_DELAY_BINARY <= EVENTS_DELAY;
    else begin
      $display("Attribute Syntax Error : The Attribute EVENTS_DELAY on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, EVENTS_DELAY);
      #1 $finish;
    end

    if ((FOUR_WINDOW_CLOCKS >= 0) && (FOUR_WINDOW_CLOCKS <= 63))
      FOUR_WINDOW_CLOCKS_BINARY <= FOUR_WINDOW_CLOCKS;
    else begin
      $display("Attribute Syntax Error : The Attribute FOUR_WINDOW_CLOCKS on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, FOUR_WINDOW_CLOCKS);
      #1 $finish;
    end

    if ((RD_CMD_OFFSET_0 >= 0) && (RD_CMD_OFFSET_0 <= 63))
      RD_CMD_OFFSET_0_BINARY <= RD_CMD_OFFSET_0;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_CMD_OFFSET_0 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_CMD_OFFSET_0);
      #1 $finish;
    end

    if ((RD_CMD_OFFSET_1 >= 0) && (RD_CMD_OFFSET_1 <= 63))
      RD_CMD_OFFSET_1_BINARY <= RD_CMD_OFFSET_1;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_CMD_OFFSET_1 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_CMD_OFFSET_1);
      #1 $finish;
    end

    if ((RD_CMD_OFFSET_2 >= 0) && (RD_CMD_OFFSET_2 <= 63))
      RD_CMD_OFFSET_2_BINARY <= RD_CMD_OFFSET_2;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_CMD_OFFSET_2 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_CMD_OFFSET_2);
      #1 $finish;
    end

    if ((RD_CMD_OFFSET_3 >= 0) && (RD_CMD_OFFSET_3 <= 63))
      RD_CMD_OFFSET_3_BINARY <= RD_CMD_OFFSET_3;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_CMD_OFFSET_3 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_CMD_OFFSET_3);
      #1 $finish;
    end

    if ((RD_DURATION_0 >= 0) && (RD_DURATION_0 <= 63))
      RD_DURATION_0_BINARY <= RD_DURATION_0;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_DURATION_0 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_DURATION_0);
      #1 $finish;
    end

    if ((RD_DURATION_1 >= 0) && (RD_DURATION_1 <= 63))
      RD_DURATION_1_BINARY <= RD_DURATION_1;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_DURATION_1 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_DURATION_1);
      #1 $finish;
    end

    if ((RD_DURATION_2 >= 0) && (RD_DURATION_2 <= 63))
      RD_DURATION_2_BINARY <= RD_DURATION_2;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_DURATION_2 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_DURATION_2);
      #1 $finish;
    end

    if ((RD_DURATION_3 >= 0) && (RD_DURATION_3 <= 63))
      RD_DURATION_3_BINARY <= RD_DURATION_3;
    else begin
      $display("Attribute Syntax Error : The Attribute RD_DURATION_3 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, RD_DURATION_3);
      #1 $finish;
    end

    if ((WR_CMD_OFFSET_0 >= 0) && (WR_CMD_OFFSET_0 <= 63))
      WR_CMD_OFFSET_0_BINARY <= WR_CMD_OFFSET_0;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_CMD_OFFSET_0 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_CMD_OFFSET_0);
      #1 $finish;
    end

    if ((WR_CMD_OFFSET_1 >= 0) && (WR_CMD_OFFSET_1 <= 63))
      WR_CMD_OFFSET_1_BINARY <= WR_CMD_OFFSET_1;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_CMD_OFFSET_1 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_CMD_OFFSET_1);
      #1 $finish;
    end

    if ((WR_CMD_OFFSET_2 >= 0) && (WR_CMD_OFFSET_2 <= 63))
      WR_CMD_OFFSET_2_BINARY <= WR_CMD_OFFSET_2;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_CMD_OFFSET_2 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_CMD_OFFSET_2);
      #1 $finish;
    end

    if ((WR_CMD_OFFSET_3 >= 0) && (WR_CMD_OFFSET_3 <= 63))
      WR_CMD_OFFSET_3_BINARY <= WR_CMD_OFFSET_3;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_CMD_OFFSET_3 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_CMD_OFFSET_3);
      #1 $finish;
    end

    if ((WR_DURATION_0 >= 0) && (WR_DURATION_0 <= 63))
      WR_DURATION_0_BINARY <= WR_DURATION_0;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_DURATION_0 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_DURATION_0);
      #1 $finish;
    end

    if ((WR_DURATION_1 >= 0) && (WR_DURATION_1 <= 63))
      WR_DURATION_1_BINARY <= WR_DURATION_1;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_DURATION_1 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_DURATION_1);
      #1 $finish;
    end

    if ((WR_DURATION_2 >= 0) && (WR_DURATION_2 <= 63))
      WR_DURATION_2_BINARY <= WR_DURATION_2;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_DURATION_2 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_DURATION_2);
      #1 $finish;
    end

    if ((WR_DURATION_3 >= 0) && (WR_DURATION_3 <= 63))
      WR_DURATION_3_BINARY <= WR_DURATION_3;
    else begin
      $display("Attribute Syntax Error : The Attribute WR_DURATION_3 on %s instance %m is set to %d.  Legal values for this attribute are 0 to 63.", MODULE_NAME, WR_DURATION_3);
      #1 $finish;
    end

  end

  wire [15:0] delay_TESTOUTPUT;
  wire [1:0] delay_INRANKA;
  wire [1:0] delay_INRANKB;
  wire [1:0] delay_INRANKC;
  wire [1:0] delay_INRANKD;
  wire [1:0] delay_PCENABLECALIB;
  wire [3:0] delay_AUXOUTPUT;
  wire [3:0] delay_INBURSTPENDING;
  wire [3:0] delay_OUTBURSTPENDING;
  wire delay_PHYCTLALMOSTFULL;
  wire delay_PHYCTLEMPTY;
  wire delay_PHYCTLFULL;
  wire delay_PHYCTLREADY;

  wire [15:0] delay_TESTINPUT = 16'hffff;
  wire [2:0] delay_TESTSELECT = 3'b111;
  wire [31:0] delay_PHYCTLWD;
  wire delay_MEMREFCLK;
  wire delay_PHYCLK;
  wire delay_PHYCTLMSTREMPTY;
  wire delay_PHYCTLWRENABLE;
  wire delay_PLLLOCK;
  wire delay_READCALIBENABLE;
  wire delay_REFDLLLOCK;
  wire delay_RESET;
  wire delay_SCANENABLEN = 1'b1;
  wire delay_SYNCIN;
  wire delay_WRITECALIBENABLE;
  wire delay_GSR;

  assign #(out_delay) AUXOUTPUT = delay_AUXOUTPUT;
  assign #(out_delay) INBURSTPENDING = delay_INBURSTPENDING;
  assign #(out_delay) INRANKA = delay_INRANKA;
  assign #(out_delay) INRANKB = delay_INRANKB;
  assign #(out_delay) INRANKC = delay_INRANKC;
  assign #(out_delay) INRANKD = delay_INRANKD;
  assign #(out_delay) OUTBURSTPENDING = delay_OUTBURSTPENDING;
  assign #(out_delay) PCENABLECALIB = delay_PCENABLECALIB;
  assign #(out_delay) PHYCTLALMOSTFULL = delay_PHYCTLALMOSTFULL;
  assign #(out_delay) PHYCTLEMPTY = delay_PHYCTLEMPTY;
  assign #(out_delay) PHYCTLFULL = delay_PHYCTLFULL;
  assign #(out_delay) PHYCTLREADY = delay_PHYCTLREADY;

`ifndef XIL_TIMING
  assign #(INCLK_DELAY) delay_MEMREFCLK = MEMREFCLK;
  assign #(INCLK_DELAY) delay_PHYCLK = PHYCLK;

  assign #(in_delay) delay_PHYCTLMSTREMPTY = PHYCTLMSTREMPTY;
  assign #(in_delay) delay_PHYCTLWD = PHYCTLWD;
  assign #(in_delay) delay_PHYCTLWRENABLE = PHYCTLWRENABLE;
`endif
  assign #(in_delay) delay_PLLLOCK = PLLLOCK;
  assign #(in_delay) delay_READCALIBENABLE = READCALIBENABLE;
  assign #(in_delay) delay_REFDLLLOCK = REFDLLLOCK;
  assign #(in_delay) delay_RESET = RESET;
`ifndef XIL_TIMING
  assign #(in_delay) delay_SYNCIN = SYNCIN;
`endif
  assign #(in_delay) delay_WRITECALIBENABLE = WRITECALIBENABLE;
  assign delay_GSR = GSR;

  SIP_PHY_CONTROL PHY_CONTROL_INST (
    .AO_TOGGLE (AO_TOGGLE_BINARY),
    .AO_WRLVL_EN (AO_WRLVL_EN_BINARY),
    .BURST_MODE (BURST_MODE_BINARY),
    .CLK_RATIO (CLK_RATIO_BINARY),
    .CMD_OFFSET (CMD_OFFSET_BINARY),
    .CO_DURATION (CO_DURATION_BINARY),
    .DATA_CTL_A_N (DATA_CTL_A_N_BINARY),
    .DATA_CTL_B_N (DATA_CTL_B_N_BINARY),
    .DATA_CTL_C_N (DATA_CTL_C_N_BINARY),
    .DATA_CTL_D_N (DATA_CTL_D_N_BINARY),
    .DISABLE_SEQ_MATCH (DISABLE_SEQ_MATCH_BINARY),
    .DI_DURATION (DI_DURATION_BINARY),
    .DO_DURATION (DO_DURATION_BINARY),
    .EVENTS_DELAY (EVENTS_DELAY_BINARY),
    .FOUR_WINDOW_CLOCKS (FOUR_WINDOW_CLOCKS_BINARY),
    .MULTI_REGION (MULTI_REGION_BINARY),
    .PHY_COUNT_ENABLE (PHY_COUNT_ENABLE_BINARY),
    .RD_CMD_OFFSET_0 (RD_CMD_OFFSET_0_BINARY),
    .RD_CMD_OFFSET_1 (RD_CMD_OFFSET_1_BINARY),
    .RD_CMD_OFFSET_2 (RD_CMD_OFFSET_2_BINARY),
    .RD_CMD_OFFSET_3 (RD_CMD_OFFSET_3_BINARY),
    .RD_DURATION_0 (RD_DURATION_0_BINARY),
    .RD_DURATION_1 (RD_DURATION_1_BINARY),
    .RD_DURATION_2 (RD_DURATION_2_BINARY),
    .RD_DURATION_3 (RD_DURATION_3_BINARY),
    .SPARE (SPARE_BINARY),
    .SYNC_MODE (SYNC_MODE_BINARY),
    .WR_CMD_OFFSET_0 (WR_CMD_OFFSET_0_BINARY),
    .WR_CMD_OFFSET_1 (WR_CMD_OFFSET_1_BINARY),
    .WR_CMD_OFFSET_2 (WR_CMD_OFFSET_2_BINARY),
    .WR_CMD_OFFSET_3 (WR_CMD_OFFSET_3_BINARY),
    .WR_DURATION_0 (WR_DURATION_0_BINARY),
    .WR_DURATION_1 (WR_DURATION_1_BINARY),
    .WR_DURATION_2 (WR_DURATION_2_BINARY),
    .WR_DURATION_3 (WR_DURATION_3_BINARY),

    .AUXOUTPUT (delay_AUXOUTPUT),
    .INBURSTPENDING (delay_INBURSTPENDING),
    .INRANKA (delay_INRANKA),
    .INRANKB (delay_INRANKB),
    .INRANKC (delay_INRANKC),
    .INRANKD (delay_INRANKD),
    .OUTBURSTPENDING (delay_OUTBURSTPENDING),
    .PCENABLECALIB (delay_PCENABLECALIB),
    .PHYCTLALMOSTFULL (delay_PHYCTLALMOSTFULL),
    .PHYCTLEMPTY (delay_PHYCTLEMPTY),
    .PHYCTLFULL (delay_PHYCTLFULL),
    .PHYCTLREADY (delay_PHYCTLREADY),
    .TESTOUTPUT (delay_TESTOUTPUT),
    .MEMREFCLK (delay_MEMREFCLK),
    .PHYCLK (delay_PHYCLK),
    .PHYCTLMSTREMPTY (delay_PHYCTLMSTREMPTY),
    .PHYCTLWD (delay_PHYCTLWD),
    .PHYCTLWRENABLE (delay_PHYCTLWRENABLE),
    .PLLLOCK (delay_PLLLOCK),
    .READCALIBENABLE (delay_READCALIBENABLE),
    .REFDLLLOCK (delay_REFDLLLOCK),
    .RESET (delay_RESET),
    .SCANENABLEN (delay_SCANENABLEN),
    .SYNCIN (delay_SYNCIN),
    .TESTINPUT (delay_TESTINPUT),
    .TESTSELECT (delay_TESTSELECT),
    .WRITECALIBENABLE (delay_WRITECALIBENABLE),
    .GSR (delay_GSR)
  );

`ifdef XIL_TIMING
  specify
    $period (negedge MEMREFCLK, 0:0:0, notifier);
    $period (negedge PHYCLK, 0:0:0, notifier);
    $period (posedge MEMREFCLK, 0:0:0, notifier);
    $period (posedge PHYCLK, 0:0:0, notifier);
    $setuphold (posedge MEMREFCLK, negedge PHYCTLMSTREMPTY, 0:0:0, 0:0:0, notifier,,, delay_MEMREFCLK, delay_PHYCTLMSTREMPTY);
    $setuphold (posedge MEMREFCLK, negedge SYNCIN, 0:0:0, 0:0:0, notifier,,, delay_MEMREFCLK, delay_SYNCIN);
    $setuphold (posedge MEMREFCLK, posedge PHYCTLMSTREMPTY, 0:0:0, 0:0:0, notifier,,, delay_MEMREFCLK, delay_PHYCTLMSTREMPTY);
    $setuphold (posedge MEMREFCLK, posedge SYNCIN, 0:0:0, 0:0:0, notifier,,, delay_MEMREFCLK, delay_SYNCIN);
    $setuphold (posedge PHYCLK, negedge PHYCTLWD, 0:0:0, 0:0:0, notifier,,, delay_PHYCLK, delay_PHYCTLWD);
    $setuphold (posedge PHYCLK, negedge PHYCTLWRENABLE, 0:0:0, 0:0:0, notifier,,, delay_PHYCLK, delay_PHYCTLWRENABLE);
    $setuphold (posedge PHYCLK, posedge PHYCTLWD, 0:0:0, 0:0:0, notifier,,, delay_PHYCLK, delay_PHYCTLWD);
    $setuphold (posedge PHYCLK, posedge PHYCTLWRENABLE, 0:0:0, 0:0:0, notifier,,, delay_PHYCLK, delay_PHYCTLWRENABLE);
    $width (negedge MEMREFCLK, 0:0:0, 0, notifier);
    $width (negedge PHYCLK, 0:0:0, 0, notifier);
    $width (negedge READCALIBENABLE, 0:0:0, 0, notifier);
    $width (negedge RESET, 0:0:0, 0, notifier);
    $width (negedge SYNCIN, 0:0:0, 0, notifier);
    $width (negedge WRITECALIBENABLE, 0:0:0, 0, notifier);
    $width (posedge MEMREFCLK, 0:0:0, 0, notifier);
    $width (posedge PHYCLK, 0:0:0, 0, notifier);
    $width (posedge READCALIBENABLE, 0:0:0, 0, notifier);
    $width (posedge RESET, 0:0:0, 0, notifier);
    $width (posedge SYNCIN, 0:0:0, 0, notifier);
    $width (posedge WRITECALIBENABLE, 0:0:0, 0, notifier);
    ( MEMREFCLK *> AUXOUTPUT) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> INBURSTPENDING) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> INRANKA) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> INRANKB) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> INRANKC) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> INRANKD) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> OUTBURSTPENDING) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> PCENABLECALIB) = (10:10:10, 10:10:10);
    ( MEMREFCLK *> PHYCTLEMPTY) = (10:10:10, 10:10:10);
    ( PHYCLK *> PHYCTLALMOSTFULL) = (10:10:10, 10:10:10);
    ( PHYCLK *> PHYCTLFULL) = (10:10:10, 10:10:10);
    ( PHYCLK *> PHYCTLREADY) = (10:10:10, 10:10:10);

    specparam PATHPULSE$ = 0;
  endspecify
`endif
endmodule // PHY_CONTROL

`endcelldefine
