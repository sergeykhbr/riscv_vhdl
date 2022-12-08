///////////////////////////////////////////////////////
//  Copyright (c) 1995/2015 Xilinx, Inc.
//  All Right Reserved.
///////////////////////////////////////////////////////
//
//   ____   ___
//  /   /\/   / 
// /___/  \  /     Vendor      : Xilinx 
// \  \    \/      Version     : 2015.4
//  \  \           Description : Xilinx Formal Library Component
//  /  /                         7SERIES OUT FIFO
// /__/   /\       Filename    : OUT_FIFO.v
// \  \  /  \ 
//  \__\/\__ \                    
//                                 
///////////////////////////////////////////////////////
//  Revision:    Date:     Comment:
//       0.1:    15MAR2010 Initial UNI/UNP/SIM Version from yml
///////////////////////////////////////////////////////

`timescale 1 ps / 1 ps 

`celldefine

module OUT_FIFO (
  ALMOSTEMPTY,
  ALMOSTFULL,
  EMPTY,
  FULL,
  Q0,
  Q1,
  Q2,
  Q3,
  Q4,
  Q5,
  Q6,
  Q7,
  Q8,
  Q9,

  D0,
  D1,
  D2,
  D3,
  D4,
  D5,
  D6,
  D7,
  D8,
  D9,
  RDCLK,
  RDEN,
  RESET,
  WRCLK,
  WREN
);

  parameter integer ALMOST_EMPTY_VALUE = 1;
  parameter integer ALMOST_FULL_VALUE = 1;
  parameter ARRAY_MODE = "ARRAY_MODE_8_X_4";
  parameter OUTPUT_DISABLE = "FALSE";
  parameter SYNCHRONOUS_MODE = "FALSE";

`ifdef XIL_TIMING

  parameter LOC = "UNPLACED";

`endif

  output ALMOSTEMPTY;
  output ALMOSTFULL;
  output EMPTY;
  output FULL;
  output [3:0] Q0;
  output [3:0] Q1;
  output [3:0] Q2;
  output [3:0] Q3;
  output [3:0] Q4;
  output [3:0] Q7;
  output [3:0] Q8;
  output [3:0] Q9;
  output [7:0] Q5;
  output [7:0] Q6;

  input RDCLK;
  input RDEN;
  input RESET;
  input WRCLK;
  input WREN;
  input [7:0] D0;
  input [7:0] D1;
  input [7:0] D2;
  input [7:0] D3;
  input [7:0] D4;
  input [7:0] D5;
  input [7:0] D6;
  input [7:0] D7;
  input [7:0] D8;
  input [7:0] D9;
  
endmodule

`endcelldefine
