///////////////////////////////////////////////////////
//  Copyright (c) 2010 Xilinx Inc.
//  All Right Reserved.
///////////////////////////////////////////////////////
//
//   ____   ___
//  /   /\/   / 
// /___/  \  /     Vendor      : Xilinx 
// \  \    \/      Version     : 13.1
//  \  \           Description : Xilinx Simulation Library Component
//  /  /                         7SERIES OUT FIFO
// /__/   /\       Filename    : OUT_FIFO.v
// \  \  /  \ 
//  \__\/\__ \                    
//                                 
//  Date:     Comment:
//  15MAR2010 Initial UNI/UNP/SIM version from yml
//  03JUN2010 yml update
//  10JUN2010 yml update
//  29JUN2010 enable encrypted rtl
//  10AUG2010 yml, rtl update
//  28SEP2010 minor clean up
//            add width checks
//  28OCT2010 rtl update
//  05NOV2010 update defaults
//  11JAN2011 586040 correct spelling XIL_TIMING vs XIL_TIMIMG
//  15AUG2011 621681 remove SIM_SPEEDUP, make default
//  21SEP2011 625537 period checks on RDCLK, WRCLK
//  16FEB2012 645871 add conditions to RDEN -> Q delays
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

`ifdef XIL_TIMING
  parameter LOC = "UNPLACED";
`endif
  parameter integer ALMOST_EMPTY_VALUE = 1;
  parameter integer ALMOST_FULL_VALUE = 1;
  parameter ARRAY_MODE = "ARRAY_MODE_8_X_4";
  parameter OUTPUT_DISABLE = "FALSE";
  parameter SYNCHRONOUS_MODE = "FALSE";
  
`ifdef XIL_TIMING
  localparam in_delay = 0;
  localparam out_delay = 0;
`else
  localparam in_delay = 1;
  localparam out_delay = 10;
`endif
  localparam INCLK_DELAY = 0;
  localparam OUTCLK_DELAY = 0;
  localparam MODULE_NAME = "OUT_FIFO";

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

  reg [0:0] ARRAY_MODE_BINARY;
  reg [0:0] OUTPUT_DISABLE_BINARY;
  reg [0:0] SLOW_RD_CLK_BINARY;
  reg [0:0] SLOW_WR_CLK_BINARY;
  reg [0:0] SYNCHRONOUS_MODE_BINARY;
  reg [3:0] SPARE_BINARY;
  reg [7:0] ALMOST_EMPTY_VALUE_BINARY;
  reg [7:0] ALMOST_FULL_VALUE_BINARY;

  tri0 GSR = glbl.GSR;
`ifdef XIL_TIMING
  reg notifier;
`endif

  initial begin
    case (ALMOST_EMPTY_VALUE)
      1 : ALMOST_EMPTY_VALUE_BINARY <= 8'b01000001;
      2 : ALMOST_EMPTY_VALUE_BINARY <= 8'b01100011;
      default : begin
        $display("Attribute Syntax Error : The Attribute ALMOST_EMPTY_VALUE on %s instance %m is set to %d.  Legal values for this attribute are 1 to 2.", MODULE_NAME, ALMOST_EMPTY_VALUE);
        #1 $finish;
      end
    endcase

    case (ALMOST_FULL_VALUE)
      1 : ALMOST_FULL_VALUE_BINARY <= 8'b01000001;
      2 : ALMOST_FULL_VALUE_BINARY <= 8'b01100011;
      default : begin
        $display("Attribute Syntax Error : The Attribute ALMOST_FULL_VALUE on %s instance %m is set to %d.  Legal values for this attribute are 1 to 2.", MODULE_NAME, ALMOST_FULL_VALUE);
        #1 $finish;
      end
    endcase

    case (ARRAY_MODE)
      "ARRAY_MODE_8_X_4" : ARRAY_MODE_BINARY <= 1'b1;
      "ARRAY_MODE_4_X_4" : ARRAY_MODE_BINARY <= 1'b0;
      default : begin
        $display("Attribute Syntax Error : The Attribute ARRAY_MODE on %s instance %m is set to %s.  Legal values for this attribute are ARRAY_MODE_8_X_4 or ARRAY_MODE_4_X_4.", MODULE_NAME, ARRAY_MODE);
        #1 $finish;
      end
    endcase

    case (OUTPUT_DISABLE)
      "FALSE" : OUTPUT_DISABLE_BINARY <= 1'b0;
      "TRUE" : OUTPUT_DISABLE_BINARY <= 1'b1;
      default : begin
        $display("Attribute Syntax Error : The Attribute OUTPUT_DISABLE on %s instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE.", MODULE_NAME, OUTPUT_DISABLE);
        #1 $finish;
      end
    endcase

    SLOW_RD_CLK_BINARY <= 1'b0;
    SLOW_WR_CLK_BINARY <= 1'b0;
    SPARE_BINARY <= 4'b0;

    case (SYNCHRONOUS_MODE)
      "FALSE" : SYNCHRONOUS_MODE_BINARY <= 1'b0;
      default : begin
        $display("Attribute Syntax Error : The Attribute SYNCHRONOUS_MODE on %s instance %m is set to %s.  The legal value for this attribute is FALSE.", MODULE_NAME, SYNCHRONOUS_MODE);
        #1 $finish;
      end
    endcase

  end

  wire [3:0] delay_Q0;
  wire [3:0] delay_Q1;
  wire [3:0] delay_Q2;
  wire [3:0] delay_Q3;
  wire [3:0] delay_Q4;
  wire [3:0] delay_Q7;
  wire [3:0] delay_Q8;
  wire [3:0] delay_Q9;
  wire [7:0] delay_Q5;
  wire [7:0] delay_Q6;
  wire delay_ALMOSTEMPTY;
  wire delay_ALMOSTFULL;
  wire delay_EMPTY;
  wire delay_FULL;
  wire [3:0] delay_SCANOUT;

  wire [7:0] delay_D0;
  wire [7:0] delay_D1;
  wire [7:0] delay_D2;
  wire [7:0] delay_D3;
  wire [7:0] delay_D4;
  wire [7:0] delay_D5;
  wire [7:0] delay_D6;
  wire [7:0] delay_D7;
  wire [7:0] delay_D8;
  wire [7:0] delay_D9;
  wire delay_RDCLK;
  wire delay_RDEN;
  wire delay_RESET;
  wire delay_SCANENB = 1'b1;
  wire delay_TESTMODEB = 1'b1;
  wire delay_TESTREADDISB = 1'b1;
  wire delay_TESTWRITEDISB = 1'b1;
  wire [3:0] delay_SCANIN = 4'hf;
  wire delay_WRCLK;
  wire delay_WREN;
  wire delay_GSR;

  assign #(out_delay) ALMOSTEMPTY = delay_ALMOSTEMPTY;
  assign #(out_delay) ALMOSTFULL = delay_ALMOSTFULL;
  assign #(out_delay) EMPTY = delay_EMPTY;
  assign #(out_delay) FULL = delay_FULL;
  assign #(out_delay) Q0 = delay_Q0;
  assign #(out_delay) Q1 = delay_Q1;
  assign #(out_delay) Q2 = delay_Q2;
  assign #(out_delay) Q3 = delay_Q3;
  assign #(out_delay) Q4 = delay_Q4;
  assign #(out_delay) Q5 = delay_Q5;
  assign #(out_delay) Q6 = delay_Q6;
  assign #(out_delay) Q7 = delay_Q7;
  assign #(out_delay) Q8 = delay_Q8;
  assign #(out_delay) Q9 = delay_Q9;

`ifndef XIL_TIMING
  assign #(INCLK_DELAY) delay_RDCLK = RDCLK;
  assign #(INCLK_DELAY) delay_WRCLK = WRCLK;

  assign #(in_delay) delay_D0 = D0;
  assign #(in_delay) delay_D1 = D1;
  assign #(in_delay) delay_D2 = D2;
  assign #(in_delay) delay_D3 = D3;
  assign #(in_delay) delay_D4 = D4;
  assign #(in_delay) delay_D5 = D5;
  assign #(in_delay) delay_D6 = D6;
  assign #(in_delay) delay_D7 = D7;
  assign #(in_delay) delay_D8 = D8;
  assign #(in_delay) delay_D9 = D9;
  assign #(in_delay) delay_RDEN = RDEN;
`endif
  assign #(in_delay) delay_RESET = RESET;
`ifndef XIL_TIMING
  assign #(in_delay) delay_WREN = WREN;
`endif
  assign delay_GSR = GSR;

  SIP_OUT_FIFO OUT_FIFO_INST (
    .ALMOST_EMPTY_VALUE (ALMOST_EMPTY_VALUE_BINARY),
    .ALMOST_FULL_VALUE (ALMOST_FULL_VALUE_BINARY),
    .ARRAY_MODE (ARRAY_MODE_BINARY),
    .OUTPUT_DISABLE (OUTPUT_DISABLE_BINARY),
    .SLOW_RD_CLK (SLOW_RD_CLK_BINARY),
    .SLOW_WR_CLK (SLOW_WR_CLK_BINARY),
    .SPARE (SPARE_BINARY),
    .SYNCHRONOUS_MODE (SYNCHRONOUS_MODE_BINARY),

    .ALMOSTEMPTY (delay_ALMOSTEMPTY),
    .ALMOSTFULL (delay_ALMOSTFULL),
    .EMPTY (delay_EMPTY),
    .FULL (delay_FULL),
    .Q0 (delay_Q0),
    .Q1 (delay_Q1),
    .Q2 (delay_Q2),
    .Q3 (delay_Q3),
    .Q4 (delay_Q4),
    .Q5 (delay_Q5),
    .Q6 (delay_Q6),
    .Q7 (delay_Q7),
    .Q8 (delay_Q8),
    .Q9 (delay_Q9),
    .SCANOUT (delay_SCANOUT),
    .D0 (delay_D0),
    .D1 (delay_D1),
    .D2 (delay_D2),
    .D3 (delay_D3),
    .D4 (delay_D4),
    .D5 (delay_D5),
    .D6 (delay_D6),
    .D7 (delay_D7),
    .D8 (delay_D8),
    .D9 (delay_D9),
    .RDCLK (delay_RDCLK),
    .RDEN (delay_RDEN),
    .RESET (delay_RESET),
    .SCANENB (delay_SCANENB),
    .SCANIN (delay_SCANIN),
    .TESTMODEB (delay_TESTMODEB),
    .TESTREADDISB (delay_TESTREADDISB),
    .TESTWRITEDISB (delay_TESTWRITEDISB),
    .WRCLK (delay_WRCLK),
    .WREN (delay_WREN),
    .GSR (delay_GSR)
  );

`ifdef XIL_TIMING
  specify
    $period (negedge RDCLK, 0:0:0, notifier);
    $period (negedge WRCLK, 0:0:0, notifier);
    $period (posedge RDCLK, 0:0:0, notifier);
    $period (posedge WRCLK, 0:0:0, notifier);
    $setuphold (posedge RDCLK, negedge RDEN, 0:0:0, 0:0:0, notifier,,, delay_RDCLK, delay_RDEN);
    $setuphold (posedge RDCLK, posedge RDEN, 0:0:0, 0:0:0, notifier,,, delay_RDCLK, delay_RDEN);
    $setuphold (posedge WRCLK, negedge D0, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D0);
    $setuphold (posedge WRCLK, negedge D1, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D1);
    $setuphold (posedge WRCLK, negedge D2, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D2);
    $setuphold (posedge WRCLK, negedge D3, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D3);
    $setuphold (posedge WRCLK, negedge D4, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D4);
    $setuphold (posedge WRCLK, negedge D5, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D5);
    $setuphold (posedge WRCLK, negedge D6, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D6);
    $setuphold (posedge WRCLK, negedge D7, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D7);
    $setuphold (posedge WRCLK, negedge D8, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D8);
    $setuphold (posedge WRCLK, negedge D9, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D9);
    $setuphold (posedge WRCLK, negedge WREN, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_WREN);
    $setuphold (posedge WRCLK, posedge D0, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D0);
    $setuphold (posedge WRCLK, posedge D1, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D1);
    $setuphold (posedge WRCLK, posedge D2, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D2);
    $setuphold (posedge WRCLK, posedge D3, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D3);
    $setuphold (posedge WRCLK, posedge D4, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D4);
    $setuphold (posedge WRCLK, posedge D5, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D5);
    $setuphold (posedge WRCLK, posedge D6, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D6);
    $setuphold (posedge WRCLK, posedge D7, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D7);
    $setuphold (posedge WRCLK, posedge D8, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D8);
    $setuphold (posedge WRCLK, posedge D9, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_D9);
    $setuphold (posedge WRCLK, posedge WREN, 0:0:0, 0:0:0, notifier,,, delay_WRCLK, delay_WREN);
    $width (negedge RDCLK, 0:0:0, 0, notifier);
    $width (negedge RESET, 0:0:0, 0, notifier);
    $width (negedge WRCLK, 0:0:0, 0, notifier);
    $width (posedge RDCLK, 0:0:0, 0, notifier);
    $width (posedge RESET, 0:0:0, 0, notifier);
    $width (posedge WRCLK, 0:0:0, 0, notifier);
    ( RDCLK *> ALMOSTEMPTY) = (10:10:10, 10:10:10);
    ( RDCLK *> EMPTY) = (10:10:10, 10:10:10);
    ( RDCLK *> Q0) = (10:10:10, 10:10:10);
    ( RDCLK *> Q1) = (10:10:10, 10:10:10);
    ( RDCLK *> Q2) = (10:10:10, 10:10:10);
    ( RDCLK *> Q3) = (10:10:10, 10:10:10);
    ( RDCLK *> Q4) = (10:10:10, 10:10:10);
    ( RDCLK *> Q5) = (10:10:10, 10:10:10);
    ( RDCLK *> Q6) = (10:10:10, 10:10:10);
    ( RDCLK *> Q7) = (10:10:10, 10:10:10);
    ( RDCLK *> Q8) = (10:10:10, 10:10:10);
    ( RDCLK *> Q9) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q0) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q1) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q2) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q3) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q4) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q5) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q6) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q7) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q8) = (10:10:10, 10:10:10);
    if (OUTPUT_DISABLE_BINARY) ( RDEN *> Q9) = (10:10:10, 10:10:10);
    ( WRCLK *> ALMOSTFULL) = (10:10:10, 10:10:10);
    ( WRCLK *> FULL) = (10:10:10, 10:10:10);

    specparam PATHPULSE$ = 0;
  endspecify
`endif
endmodule // OUT_FIFO

`endcelldefine
