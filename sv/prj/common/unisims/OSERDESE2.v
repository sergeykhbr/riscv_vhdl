//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  Source Synchronous Output Serializer Virtex7
// /___/   /\     Filename : OSERDESE2.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    01/29/10 - Initial version.
//    10/31/14 - Added inverter functionality for IS_*_INVERTED parameter (CR 828995).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale 1 ps / 1 ps 

`celldefine

module OSERDESE2 (
  OFB,
  OQ,
  SHIFTOUT1,
  SHIFTOUT2,
  TBYTEOUT,
  TFB,
  TQ,

  CLK,
  CLKDIV,
  D1,
  D2,
  D3,
  D4,
  D5,
  D6,
  D7,
  D8,
  OCE,
  RST,
  SHIFTIN1,
  SHIFTIN2,
  T1,
  T2,
  T3,
  T4,
  TBYTEIN,
  TCE
);

  parameter DATA_RATE_OQ = "DDR";
  parameter DATA_RATE_TQ = "DDR";
  parameter integer DATA_WIDTH = 4;
  parameter [0:0] INIT_OQ = 1'b0;
  parameter [0:0] INIT_TQ = 1'b0;
  parameter [0:0] IS_CLKDIV_INVERTED = 1'b0;
  parameter [0:0] IS_CLK_INVERTED = 1'b0;
  parameter [0:0] IS_D1_INVERTED = 1'b0;
  parameter [0:0] IS_D2_INVERTED = 1'b0;
  parameter [0:0] IS_D3_INVERTED = 1'b0;
  parameter [0:0] IS_D4_INVERTED = 1'b0;
  parameter [0:0] IS_D5_INVERTED = 1'b0;
  parameter [0:0] IS_D6_INVERTED = 1'b0;
  parameter [0:0] IS_D7_INVERTED = 1'b0;
  parameter [0:0] IS_D8_INVERTED = 1'b0;
  parameter [0:0] IS_T1_INVERTED = 1'b0;
  parameter [0:0] IS_T2_INVERTED = 1'b0;
  parameter [0:0] IS_T3_INVERTED = 1'b0;
  parameter [0:0] IS_T4_INVERTED = 1'b0;
  parameter SERDES_MODE = "MASTER";
  parameter [0:0] SRVAL_OQ = 1'b0;
  parameter [0:0] SRVAL_TQ = 1'b0;
  parameter TBYTE_CTL = "FALSE";
  parameter TBYTE_SRC = "FALSE";
  parameter integer TRISTATE_WIDTH = 4;

`ifdef XIL_TIMING

  parameter LOC = "UNPLACED";

`endif
  
  output OFB;
  output OQ;
  output SHIFTOUT1;
  output SHIFTOUT2;
  output TBYTEOUT;
  output TFB;
  output TQ;

  input CLK;
  input CLKDIV;
  input D1;
  input D2;
  input D3;
  input D4;
  input D5;
  input D6;
  input D7;
  input D8;
  input OCE;
  input RST;
  input SHIFTIN1;
  input SHIFTIN2;
  input T1;
  input T2;
  input T3;
  input T4;
  input TBYTEIN;
  input TCE;

   wire CLK_in;
   wire CLKDIV_in;
   wire D1_in;
   wire D2_in;
   wire D3_in;
   wire D4_in;
   wire D5_in;
   wire D6_in;
   wire D7_in;
   wire D8_in;
   wire T1_in;
   wire T2_in;
   wire T3_in;
   wire T4_in;
   
   assign CLK_in = CLK ^ IS_CLK_INVERTED;
   assign CLKDIV_in = CLKDIV ^ IS_CLKDIV_INVERTED;
   assign D1_in = D1 ^ IS_D1_INVERTED;
   assign D2_in = D2 ^ IS_D2_INVERTED;
   assign D3_in = D3 ^ IS_D3_INVERTED;
   assign D4_in = D4 ^ IS_D4_INVERTED;
   assign D5_in = D5 ^ IS_D5_INVERTED;
   assign D6_in = D6 ^ IS_D6_INVERTED;
   assign D7_in = D7 ^ IS_D7_INVERTED;
   assign D8_in = D8 ^ IS_D8_INVERTED;
   assign T1_in = T1 ^ IS_T1_INVERTED;
   assign T2_in = T2 ^ IS_T2_INVERTED;
   assign T3_in = T3 ^ IS_T3_INVERTED;
   assign T4_in = T4 ^ IS_T4_INVERTED;
   
  OSERDESE2_bb inst_bb (
             .OFB(OFB),
             .OQ(OQ),
             .SHIFTOUT1(SHIFTOUT1),
             .SHIFTOUT2(SHIFTOUT2),
             .TBYTEOUT(TBYTEOUT),
             .TFB(TFB),
             .TQ(TQ),
             .CLK(CLK_in),
             .CLKDIV(CLKDIV_in),
             .D1(D1_in),
             .D2(D2_in),
             .D3(D3_in),
             .D4(D4_in),
             .D5(D5_in),
             .D6(D6_in),
             .D7(D7_in),
             .D8(D8_in),
             .OCE(OCE),
             .RST(RST),
             .SHIFTIN1(SHIFTIN1),
             .SHIFTIN2(SHIFTIN2),
             .T1(T1_in),
             .T2(T2_in),
             .T3(T3_in),
             .T4(T4_in),
             .TBYTEIN(TBYTEIN),
             .TCE(TCE)
             );

endmodule

module OSERDESE2_bb (
  output OFB,
  output OQ,
  output SHIFTOUT1,
  output SHIFTOUT2,
  output TBYTEOUT,
  output TFB,
  output TQ,

  input CLK,
  input CLKDIV,
  input D1,
  input D2,
  input D3,
  input D4,
  input D5,
  input D6,
  input D7,
  input D8,
  input OCE,
  input RST,
  input SHIFTIN1,
  input SHIFTIN2,
  input T1,
  input T2,
  input T3,
  input T4,
  input TBYTEIN,
  input TCE
);

endmodule

`endcelldefine
