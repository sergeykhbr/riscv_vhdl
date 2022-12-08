///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  Source Synchronous Input Deserializer for Virtex7
// /___/   /\     Filename : ISERDESE2.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    01/19/10 - Initial version.
//    10/31/14 - Added inverter functionality for IS_*_INVERTED parameter (CR 828995).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale 1 ps / 1 ps 

`celldefine

module ISERDESE2 (
  O,
  Q1,
  Q2,
  Q3,
  Q4,
  Q5,
  Q6,
  Q7,
  Q8,
  SHIFTOUT1,
  SHIFTOUT2,

  BITSLIP,
  CE1,
  CE2,
  CLK,
  CLKB,
  CLKDIV,
  CLKDIVP,
  D,
  DDLY,
  DYNCLKDIVSEL,
  DYNCLKSEL,
  OCLK,
  OCLKB,
  OFB,
  RST,
  SHIFTIN1,
  SHIFTIN2
);

  parameter DATA_RATE = "DDR";
  parameter integer DATA_WIDTH = 4;
  parameter DYN_CLKDIV_INV_EN = "FALSE";
  parameter DYN_CLK_INV_EN = "FALSE";
  parameter [0:0] INIT_Q1 = 1'b0;
  parameter [0:0] INIT_Q2 = 1'b0;
  parameter [0:0] INIT_Q3 = 1'b0;
  parameter [0:0] INIT_Q4 = 1'b0;
  parameter INTERFACE_TYPE = "MEMORY";
  parameter IOBDELAY = "NONE";
  parameter [0:0] IS_CLKB_INVERTED = 1'b0;
  parameter [0:0] IS_CLKDIVP_INVERTED = 1'b0;
  parameter [0:0] IS_CLKDIV_INVERTED = 1'b0;
  parameter [0:0] IS_CLK_INVERTED = 1'b0;
  parameter [0:0] IS_D_INVERTED = 1'b0;
  parameter [0:0] IS_OCLKB_INVERTED = 1'b0;
  parameter [0:0] IS_OCLK_INVERTED = 1'b0;
  parameter integer NUM_CE = 2;
  parameter OFB_USED = "FALSE";
  parameter SERDES_MODE = "MASTER";
  parameter [0:0] SRVAL_Q1 = 1'b0;
  parameter [0:0] SRVAL_Q2 = 1'b0;
  parameter [0:0] SRVAL_Q3 = 1'b0;
  parameter [0:0] SRVAL_Q4 = 1'b0;

`ifdef XIL_TIMING

  parameter LOC = "UNPLACED";

`endif

  output O;
  output Q1;
  output Q2;
  output Q3;
  output Q4;
  output Q5;
  output Q6;
  output Q7;
  output Q8;
  output SHIFTOUT1;
  output SHIFTOUT2;

  input BITSLIP;
  input CE1;
  input CE2;
  input CLK;
  input CLKB;
  input CLKDIV;
  input CLKDIVP;
  input D;
  input DDLY;
  input DYNCLKDIVSEL;
  input DYNCLKSEL;
  input OCLK;
  input OCLKB;
  input OFB;
  input RST;
  input SHIFTIN1;
  input SHIFTIN2;

  wire CLK_in;
  wire CLKB_in;
  wire CLKDIV_in;
  wire CLKDIVP_in;
  wire D_in;
  wire OCLK_in;
  wire OCLKB_in;
   
  assign CLK_in = IS_CLK_INVERTED ^ CLK;
  assign CLKB_in = IS_CLKB_INVERTED ^ CLKB;
  assign CLKDIV_in = IS_CLKDIV_INVERTED ^ CLKDIV;
  assign CLKDIVP_in = IS_CLKDIVP_INVERTED ^ CLKDIVP;
  assign D_in = IS_D_INVERTED ^ D;
  assign OCLK_in = IS_OCLK_INVERTED ^ OCLK;
  assign OCLKB_in = IS_OCLKB_INVERTED ^ OCLKB;
   
  ISERDESE2_bb inst_bb (
             .O(O),
             .Q1(Q1),
             .Q2(Q2),
             .Q3(Q3),
             .Q4(Q4),
             .Q5(Q5),
             .Q6(Q6),
             .Q7(Q7),
             .Q8(Q8),
             .SHIFTOUT1(SHIFTOUT1),
             .SHIFTOUT2(SHIFTOUT2),
             .BITSLIP(BITSLIP),
             .CE1(CE1),
             .CE2(CE2),
             .CLK(CLK_in),
             .CLKB(CLKB_in),
             .CLKDIV(CLKDIV_in),
             .CLKDIVP(CLKDIVP_in),
             .D(D_in),
             .DDLY(DDLY),
             .DYNCLKDIVSEL(DYNCLKDIVSEL),
             .DYNCLKSEL(DYNCLKSEL),
             .OCLK(OCLK_in),
             .OCLKB(OCLKB_in),
             .OFB(OFB),
             .RST(RST),
             .SHIFTIN1(SHIFTIN1),
             .SHIFTIN2(SHIFTIN2)
             );

endmodule

module ISERDESE2_bb (
  output O,
  output Q1,
  output Q2,
  output Q3,
  output Q4,
  output Q5,
  output Q6,
  output Q7,
  output Q8,
  output SHIFTOUT1,
  output SHIFTOUT2,

  input BITSLIP,
  input CE1,
  input CE2,
  input CLK,
  input CLKB,
  input CLKDIV,
  input CLKDIVP,
  input D,
  input DDLY,
  input DYNCLKDIVSEL,
  input DYNCLKSEL,
  input OCLK,
  input OCLKB,
  input OFB,
  input RST,
  input SHIFTIN1,
  input SHIFTIN2
);

endmodule

`endcelldefine
