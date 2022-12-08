///////////////////////////////////////////////////////
//  Copyright (c) 1995/2015 Xilinx, Inc.
//  All Right Reserved.
///////////////////////////////////////////////////////
//
//   ____   ___
//  /   /\/   / 
// /___/  \  /     Vendor      : Xilinx 
// \  \    \/      Version : 2015.4
//  \  \           Description : Xilinx Formal Library Component
//  /  /                         7SERIES PHASER IN
// /__/   /\       Filename    : PHASER_IN_PHY.v
// \  \  /  \ 
//  \__\/\__ \                    
//                                 
/////////////////////////////////////////////////////////
//  Revision: Comment:
//  22APR2010 Initial UNI/UNP/SIM version from yaml
//    10/31/14 - Added inverter functionality for IS_*_INVERTED parameter (CR 828995).
// End Revision
/////////////////////////////////////////////////////////

`timescale 1 ps / 1 ps 

`celldefine

module PHASER_IN_PHY (
  COUNTERREADVAL,
  DQSFOUND,
  DQSOUTOFRANGE,
  FINEOVERFLOW,
  ICLK,
  ICLKDIV,
  ISERDESRST,
  PHASELOCKED,
  RCLK,
  WRENABLE,

  BURSTPENDINGPHY,
  COUNTERLOADEN,
  COUNTERLOADVAL,
  COUNTERREADEN,
  ENCALIBPHY,
  FINEENABLE,
  FINEINC,
  FREQREFCLK,
  MEMREFCLK,
  PHASEREFCLK,
  RANKSELPHY,
  RST,
  RSTDQSFIND,
  SYNCIN,
  SYSCLK
);

`ifdef XIL_TIMING
  parameter LOC = "UNPLACED";
`endif

  parameter BURST_MODE = "FALSE";
  parameter integer CLKOUT_DIV = 4;
  parameter [0:0] DQS_AUTO_RECAL = 1'b1;
  parameter DQS_BIAS_MODE = "FALSE"; 
  parameter [2:0] DQS_FIND_PATTERN = 3'b001;
  parameter integer FINE_DELAY = 0;
  parameter FREQ_REF_DIV = "NONE";
  parameter [0:0] IS_RST_INVERTED = 1'b0;
  parameter real MEMREFCLK_PERIOD = 0.000;
  parameter OUTPUT_CLK_SRC = "PHASE_REF";
  parameter real PHASEREFCLK_PERIOD = 0.000;
  parameter real REFCLK_PERIOD = 0.000;
  parameter integer SEL_CLK_OFFSET = 5;
  parameter SYNC_IN_DIV_RST = "FALSE";
  parameter WR_CYCLES = "FALSE";
  

  output DQSFOUND;
  output DQSOUTOFRANGE;
  output FINEOVERFLOW;
  output ICLK;
  output ICLKDIV;
  output ISERDESRST;
  output PHASELOCKED;
  output RCLK;
  output WRENABLE;
  output [5:0] COUNTERREADVAL;

  input BURSTPENDINGPHY;
  input COUNTERLOADEN;
  input COUNTERREADEN;
  input FINEENABLE;
  input FINEINC;
  input FREQREFCLK;
  input MEMREFCLK;
  input PHASEREFCLK;
  input RST;
  input RSTDQSFIND;
  input SYNCIN;
  input SYSCLK;
  input [1:0] ENCALIBPHY;
  input [1:0] RANKSELPHY;
  input [5:0] COUNTERLOADVAL;

  wire RST_in;
   
  assign RST_in = RST ^ IS_RST_INVERTED;
   
  PHASER_IN_PHY_bb inst_bb (
               .COUNTERREADVAL(COUNTERREADVAL),
               .DQSFOUND(DQSFOUND),
               .DQSOUTOFRANGE(DQSOUTOFRANGE),
               .FINEOVERFLOW(FINEOVERFLOW),
               .ICLK(ICLK),
               .ICLKDIV(ICLKDIV),
               .ISERDESRST(ISERDESRST),
               .PHASELOCKED(PHASELOCKED),
               .RCLK(RCLK),
               .WRENABLE(WRENABLE),
               .BURSTPENDINGPHY(BURSTPENDINGPHY),
               .COUNTERLOADEN(COUNTERLOADEN),
               .COUNTERLOADVAL(COUNTERLOADVAL),
               .COUNTERREADEN(COUNTERREADEN),
               .ENCALIBPHY(ENCALIBPHY),
               .FINEENABLE(FINEENABLE),
               .FINEINC(FINEINC),
               .FREQREFCLK(FREQREFCLK),
               .MEMREFCLK(MEMREFCLK),
               .PHASEREFCLK(PHASEREFCLK),
               .RANKSELPHY(RANKSELPHY),
               .RST(RST_in),
               .RSTDQSFIND(RSTDQSFIND),
               .SYNCIN(SYNCIN),
               .SYSCLK(SYSCLK)
               );

endmodule

module PHASER_IN_PHY_bb (
  output DQSFOUND,
  output DQSOUTOFRANGE,
  output FINEOVERFLOW,
  output ICLK,
  output ICLKDIV,
  output ISERDESRST,
  output PHASELOCKED,
  output RCLK,
  output WRENABLE,
  output [5:0] COUNTERREADVAL,

  input BURSTPENDINGPHY,
  input COUNTERLOADEN,
  input COUNTERREADEN,
  input FINEENABLE,
  input FINEINC,
  input FREQREFCLK,
  input MEMREFCLK,
  input PHASEREFCLK,
  input RST,
  input RSTDQSFIND,
  input SYNCIN,
  input SYSCLK,
  input [1:0] ENCALIBPHY,
  input [1:0] RANKSELPHY,
  input [5:0] COUNTERLOADVAL
);

endmodule

`endcelldefine
