///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  Input Fixed or Variable Delay Element with Fine Adjustment.
// /___/   /\     Filename : IDELAYE2_FINEDELAY.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    02/15/11 - Initial version.
//    10/31/14 - Added inverter functionality for IS_*_INVERTED parameter (CR 828995).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module IDELAYE2_FINEDELAY (
  CNTVALUEOUT,
  DATAOUT,

  C,
  CE,
  CINVCTRL,
  CNTVALUEIN,
  DATAIN,
  IDATAIN,
  IFDLY,
  INC,
  LD,
  LDPIPEEN,
  REGRST
);

    parameter CINVCTRL_SEL = "FALSE";
    parameter DELAY_SRC = "IDATAIN";
    parameter FINEDELAY = "BYPASS";
    parameter HIGH_PERFORMANCE_MODE    = "FALSE";
    parameter IDELAY_TYPE  = "FIXED";
    parameter integer IDELAY_VALUE = 0;
    parameter [0:0] IS_C_INVERTED = 1'b0;
    parameter [0:0] IS_DATAIN_INVERTED = 1'b0;
    parameter [0:0] IS_IDATAIN_INVERTED = 1'b0;
    parameter PIPE_SEL = "FALSE";
    parameter real REFCLK_FREQUENCY = 200.0;
    parameter SIGNAL_PATTERN    = "DATA";

`ifdef XIL_TIMING
    parameter LOC = "UNPLACED";
    parameter integer SIM_DELAY_D = 0;
`endif // ifdef XIL_TIMING

    output [4:0] CNTVALUEOUT;
    output DATAOUT;

    input C;
    input CE;
    input CINVCTRL;
    input [4:0] CNTVALUEIN;
    input DATAIN;
    input IDATAIN;
    input [2:0] IFDLY;
    input INC;
    input LD;
    input LDPIPEEN;
    input REGRST;

   wire C_in;
   wire DATAIN_in;
   wire IDATAIN_in;

   assign C_in = IS_C_INVERTED ^ C;
   assign DATAIN_in = IS_DATAIN_INVERTED ^ DATAIN;
   assign IDATAIN_in = IS_IDATAIN_INVERTED ^ IDATAIN;

   IDELAYE2_FINEDELAY_bb inst_bb (
                  .CNTVALUEOUT(CNTVALUEOUT),
                  .DATAOUT(DATAOUT),
                  .C(C_in),
                  .CE(CE),
                  .CINVCTRL(CINVCTRL),
                  .CNTVALUEIN(CNTVALUEIN),
                  .DATAIN(DATAIN_in),
                  .IDATAIN(IDATAIN_in),
                  .IFDLY(IFDLY),
                  .INC(INC),
                  .LD(LD),
                  .LDPIPEEN(LDPIPEEN),
                  .REGRST(REGRST)
                  );

endmodule

module IDELAYE2_FINEDELAY_bb (
    output [4:0] CNTVALUEOUT,
    output DATAOUT,

    input C,
    input CE,
    input CINVCTRL,
    input [4:0] CNTVALUEIN,
    input DATAIN,
    input IDATAIN,
    input [2:0] IFDLY,
    input INC,
    input LD,
    input LDPIPEEN,
    input REGRST
);

endmodule

`endcelldefine
