///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2020 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2020.3
//  \   \         Description : Xilinx Timing Simulation Library Component
//  /   /                  Bi-Directional Buffer
// /___/   /\     Filename : IOBUF_DCIEN.v
// \   \  /  \    Timestamp : Wed Dec  8 17:04:24 PST 2010
//  \___\/\___\
//
// Revision:
//    12/08/10 - Initial version.
//    03/28/11 - CR 603466 fix
//    06/15/11 - CR 613347 -- made ouput logic_1 when IBUFDISABLE is active
//    08/31/11 - CR 623170 -- Tristate powergating support
//    09/20/11 - CR 624774, 625725 -- Removed attributes IBUF_DELAY_VALUE, IFD_DELAY_VALUE and CAPACITANCE
//    09/20/11 - CR 625564 -- Fixed Tristate powergating polarity
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    10/22/14 - Added #1 to $finish (CR 808642).
// End Revision

`timescale  1 ps / 1 ps


`celldefine

module IOBUF_DCIEN #(
`ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
`endif
  parameter integer DRIVE = 12,
  parameter IBUF_LOW_PWR = "TRUE",
  parameter IOSTANDARD = "DEFAULT",
  parameter SIM_DEVICE = "7SERIES",
  parameter SLEW = "SLOW",
  parameter USE_IBUFDISABLE = "TRUE"
)(
  output O,

  inout IO,

  input DCITERMDISABLE,
  input I,
  input IBUFDISABLE,
  input T
);

// define constants
  localparam MODULE_NAME = "IOBUF_DCIEN";
  
// Parameter encodings and registers
  localparam IBUF_LOW_PWR_FALSE = 1;
  localparam IBUF_LOW_PWR_TRUE = 0;
  localparam SIM_DEVICE_7SERIES = 0;
  localparam SIM_DEVICE_ULTRASCALE = 1;
  localparam SIM_DEVICE_VERSAL_AI_CORE = 2;
  localparam SIM_DEVICE_VERSAL_AI_CORE_ES1 = 3;
  localparam SIM_DEVICE_VERSAL_AI_EDGE = 4;
  localparam SIM_DEVICE_VERSAL_AI_EDGE_ES1 = 5;
  localparam SIM_DEVICE_VERSAL_AI_EDGE_ES2 = 6;
  localparam SIM_DEVICE_VERSAL_AI_RF = 7;
  localparam SIM_DEVICE_VERSAL_AI_RF_ES1 = 8;
  localparam SIM_DEVICE_VERSAL_AI_RF_ES2 = 9;
  localparam SIM_DEVICE_VERSAL_HBM = 10;
  localparam SIM_DEVICE_VERSAL_HBM_ES1 = 11;
  localparam SIM_DEVICE_VERSAL_HBM_ES2 = 12;
  localparam SIM_DEVICE_VERSAL_PREMIUM = 13;
  localparam SIM_DEVICE_VERSAL_PREMIUM_ES1 = 14;
  localparam SIM_DEVICE_VERSAL_PREMIUM_ES2 = 15;
  localparam SIM_DEVICE_VERSAL_PRIME = 16;
  localparam SIM_DEVICE_VERSAL_PRIME_ES1 = 17;
  localparam SLEW_FAST = 1;
  localparam SLEW_MEDIUM = 2;
  localparam SLEW_SLOW = 0;
  localparam USE_IBUFDISABLE_FALSE = 0;
  localparam USE_IBUFDISABLE_TRUE = 1;
  localparam USE_IBUFDISABLE_T_CONTROL = 2;

  
// include dynamic registers - XILINX test only
  reg trig_attr = 1'b0;
  localparam integer DRIVE_REG = DRIVE;
  localparam [40:1] IBUF_LOW_PWR_REG = IBUF_LOW_PWR;
  localparam [56:1] IOSTANDARD_REG = IOSTANDARD;
  localparam [144:1] SIM_DEVICE_REG = SIM_DEVICE;
  localparam [48:1] SLEW_REG = SLEW;
  localparam [72:1] USE_IBUFDISABLE_REG = USE_IBUFDISABLE;

`ifdef XIL_XECLIB
  wire [4:0] DRIVE_BIN;
  wire IBUF_LOW_PWR_BIN;
  wire [4:0] SIM_DEVICE_BIN;
  wire [1:0] SLEW_BIN;
  wire [1:0] USE_IBUFDISABLE_BIN;
`else
  reg [4:0] DRIVE_BIN;
  reg IBUF_LOW_PWR_BIN;
  reg [4:0] SIM_DEVICE_BIN;
  reg [1:0] SLEW_BIN;
  reg [1:0] USE_IBUFDISABLE_BIN;
`endif

`ifdef XIL_XECLIB
reg glblGTS = 1'b0;
`else
tri0 glblGTS = glbl.GTS;
`endif

`ifndef XIL_XECLIB
  reg attr_test;
  reg attr_err;
  
  initial begin
  trig_attr = 1'b0;
  `ifdef XIL_ATTR_TEST
    attr_test = 1'b1;
  `else
    attr_test = 1'b0;
  `endif
    attr_err = 1'b0;
    #1;
    trig_attr = ~trig_attr;
  end
`endif

`ifdef XIL_XECLIB
  assign DRIVE_BIN = DRIVE_REG;
  
  assign IBUF_LOW_PWR_BIN =
      (IBUF_LOW_PWR_REG == "TRUE") ? IBUF_LOW_PWR_TRUE :
      (IBUF_LOW_PWR_REG == "FALSE") ? IBUF_LOW_PWR_FALSE :
       IBUF_LOW_PWR_TRUE;
  
  assign SIM_DEVICE_BIN =
      (SIM_DEVICE_REG == "7SERIES") ? SIM_DEVICE_7SERIES :
      (SIM_DEVICE_REG == "ULTRASCALE") ? SIM_DEVICE_ULTRASCALE :
      (SIM_DEVICE_REG == "VERSAL_AI_CORE") ? SIM_DEVICE_VERSAL_AI_CORE :
      (SIM_DEVICE_REG == "VERSAL_AI_CORE_ES1") ? SIM_DEVICE_VERSAL_AI_CORE_ES1 :
      (SIM_DEVICE_REG == "VERSAL_AI_EDGE") ? SIM_DEVICE_VERSAL_AI_EDGE :
      (SIM_DEVICE_REG == "VERSAL_AI_EDGE_ES1") ? SIM_DEVICE_VERSAL_AI_EDGE_ES1 :
      (SIM_DEVICE_REG == "VERSAL_AI_EDGE_ES2") ? SIM_DEVICE_VERSAL_AI_EDGE_ES2 :
      (SIM_DEVICE_REG == "VERSAL_AI_RF") ? SIM_DEVICE_VERSAL_AI_RF :
      (SIM_DEVICE_REG == "VERSAL_AI_RF_ES1") ? SIM_DEVICE_VERSAL_AI_RF_ES1 :
      (SIM_DEVICE_REG == "VERSAL_AI_RF_ES2") ? SIM_DEVICE_VERSAL_AI_RF_ES2 :
      (SIM_DEVICE_REG == "VERSAL_HBM") ? SIM_DEVICE_VERSAL_HBM :
      (SIM_DEVICE_REG == "VERSAL_HBM_ES1") ? SIM_DEVICE_VERSAL_HBM_ES1 :
      (SIM_DEVICE_REG == "VERSAL_HBM_ES2") ? SIM_DEVICE_VERSAL_HBM_ES2 :
      (SIM_DEVICE_REG == "VERSAL_PREMIUM") ? SIM_DEVICE_VERSAL_PREMIUM :
      (SIM_DEVICE_REG == "VERSAL_PREMIUM_ES1") ? SIM_DEVICE_VERSAL_PREMIUM_ES1 :
      (SIM_DEVICE_REG == "VERSAL_PREMIUM_ES2") ? SIM_DEVICE_VERSAL_PREMIUM_ES2 :
      (SIM_DEVICE_REG == "VERSAL_PRIME") ? SIM_DEVICE_VERSAL_PRIME :
      (SIM_DEVICE_REG == "VERSAL_PRIME_ES1") ? SIM_DEVICE_VERSAL_PRIME_ES1 :
       SIM_DEVICE_7SERIES;
  
  assign SLEW_BIN =
      (SLEW_REG == "SLOW") ? SLEW_SLOW :
      (SLEW_REG == "FAST") ? SLEW_FAST :
      (SLEW_REG == "MEDIUM") ? SLEW_MEDIUM :
       SLEW_SLOW;
  
  assign USE_IBUFDISABLE_BIN =
      (USE_IBUFDISABLE_REG == "TRUE") ? USE_IBUFDISABLE_TRUE :
      (USE_IBUFDISABLE_REG == "FALSE") ? USE_IBUFDISABLE_FALSE :
      (USE_IBUFDISABLE_REG == "T_CONTROL") ? USE_IBUFDISABLE_T_CONTROL :
       USE_IBUFDISABLE_TRUE;
  
`else
  always @ (trig_attr) begin
  #1;
  DRIVE_BIN = DRIVE_REG;
  
  IBUF_LOW_PWR_BIN =
      (IBUF_LOW_PWR_REG == "TRUE") ? IBUF_LOW_PWR_TRUE :
      (IBUF_LOW_PWR_REG == "FALSE") ? IBUF_LOW_PWR_FALSE :
       IBUF_LOW_PWR_TRUE;
  
  SIM_DEVICE_BIN =
      (SIM_DEVICE_REG == "7SERIES") ? SIM_DEVICE_7SERIES :
      (SIM_DEVICE_REG == "ULTRASCALE") ? SIM_DEVICE_ULTRASCALE :
      (SIM_DEVICE_REG == "VERSAL_AI_CORE") ? SIM_DEVICE_VERSAL_AI_CORE :
      (SIM_DEVICE_REG == "VERSAL_AI_CORE_ES1") ? SIM_DEVICE_VERSAL_AI_CORE_ES1 :
      (SIM_DEVICE_REG == "VERSAL_AI_EDGE") ? SIM_DEVICE_VERSAL_AI_EDGE :
      (SIM_DEVICE_REG == "VERSAL_AI_EDGE_ES1") ? SIM_DEVICE_VERSAL_AI_EDGE_ES1 :
      (SIM_DEVICE_REG == "VERSAL_AI_EDGE_ES2") ? SIM_DEVICE_VERSAL_AI_EDGE_ES2 :
      (SIM_DEVICE_REG == "VERSAL_AI_RF") ? SIM_DEVICE_VERSAL_AI_RF :
      (SIM_DEVICE_REG == "VERSAL_AI_RF_ES1") ? SIM_DEVICE_VERSAL_AI_RF_ES1 :
      (SIM_DEVICE_REG == "VERSAL_AI_RF_ES2") ? SIM_DEVICE_VERSAL_AI_RF_ES2 :
      (SIM_DEVICE_REG == "VERSAL_HBM") ? SIM_DEVICE_VERSAL_HBM :
      (SIM_DEVICE_REG == "VERSAL_HBM_ES1") ? SIM_DEVICE_VERSAL_HBM_ES1 :
      (SIM_DEVICE_REG == "VERSAL_HBM_ES2") ? SIM_DEVICE_VERSAL_HBM_ES2 :
      (SIM_DEVICE_REG == "VERSAL_PREMIUM") ? SIM_DEVICE_VERSAL_PREMIUM :
      (SIM_DEVICE_REG == "VERSAL_PREMIUM_ES1") ? SIM_DEVICE_VERSAL_PREMIUM_ES1 :
      (SIM_DEVICE_REG == "VERSAL_PREMIUM_ES2") ? SIM_DEVICE_VERSAL_PREMIUM_ES2 :
      (SIM_DEVICE_REG == "VERSAL_PRIME") ? SIM_DEVICE_VERSAL_PRIME :
      (SIM_DEVICE_REG == "VERSAL_PRIME_ES1") ? SIM_DEVICE_VERSAL_PRIME_ES1 :
       SIM_DEVICE_7SERIES;
  
  SLEW_BIN =
      (SLEW_REG == "SLOW") ? SLEW_SLOW :
      (SLEW_REG == "FAST") ? SLEW_FAST :
      (SLEW_REG == "MEDIUM") ? SLEW_MEDIUM :
       SLEW_SLOW;
  
  USE_IBUFDISABLE_BIN =
      (USE_IBUFDISABLE_REG == "TRUE") ? USE_IBUFDISABLE_TRUE :
      (USE_IBUFDISABLE_REG == "FALSE") ? USE_IBUFDISABLE_FALSE :
      (USE_IBUFDISABLE_REG == "T_CONTROL") ? USE_IBUFDISABLE_T_CONTROL :
       USE_IBUFDISABLE_TRUE;
  
  end
`endif

`ifndef XIL_XECLIB
  always @ (trig_attr) begin
    #1;
    if ((attr_test == 1'b1) ||
        ((DRIVE_REG < 2) || (DRIVE_REG > 24))) begin
      $display("Error: [Unisim %s-101] DRIVE attribute is set to %d.  Legal values for this attribute are 2 to 24. Instance: %m", MODULE_NAME, DRIVE_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((IBUF_LOW_PWR_REG != "TRUE") &&
         (IBUF_LOW_PWR_REG != "FALSE"))) begin
      $display("Error: [Unisim %s-102] IBUF_LOW_PWR attribute is set to %s.  Legal values for this attribute are TRUE or FALSE. Instance: %m", MODULE_NAME, IBUF_LOW_PWR_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((SIM_DEVICE_REG != "7SERIES") &&
         (SIM_DEVICE_REG != "ULTRASCALE") &&
         (SIM_DEVICE_REG != "VERSAL_AI_CORE") &&
         (SIM_DEVICE_REG != "VERSAL_AI_CORE_ES1") &&
         (SIM_DEVICE_REG != "VERSAL_AI_EDGE") &&
         (SIM_DEVICE_REG != "VERSAL_AI_EDGE_ES1") &&
         (SIM_DEVICE_REG != "VERSAL_AI_EDGE_ES2") &&
         (SIM_DEVICE_REG != "VERSAL_AI_RF") &&
         (SIM_DEVICE_REG != "VERSAL_AI_RF_ES1") &&
         (SIM_DEVICE_REG != "VERSAL_AI_RF_ES2") &&
         (SIM_DEVICE_REG != "VERSAL_HBM") &&
         (SIM_DEVICE_REG != "VERSAL_HBM_ES1") &&
         (SIM_DEVICE_REG != "VERSAL_HBM_ES2") &&
         (SIM_DEVICE_REG != "VERSAL_PREMIUM") &&
         (SIM_DEVICE_REG != "VERSAL_PREMIUM_ES1") &&
         (SIM_DEVICE_REG != "VERSAL_PREMIUM_ES2") &&
         (SIM_DEVICE_REG != "VERSAL_PRIME") &&
         (SIM_DEVICE_REG != "VERSAL_PRIME_ES1"))) begin
      $display("Error: [Unisim %s-104] SIM_DEVICE attribute is set to %s.  Legal values for this attribute are 7SERIES, ULTRASCALE, VERSAL_AI_CORE, VERSAL_AI_CORE_ES1, VERSAL_AI_EDGE, VERSAL_AI_EDGE_ES1, VERSAL_AI_EDGE_ES2, VERSAL_AI_RF, VERSAL_AI_RF_ES1, VERSAL_AI_RF_ES2, VERSAL_HBM, VERSAL_HBM_ES1, VERSAL_HBM_ES2, VERSAL_PREMIUM, VERSAL_PREMIUM_ES1, VERSAL_PREMIUM_ES2, VERSAL_PRIME or VERSAL_PRIME_ES1. Instance: %m", MODULE_NAME, SIM_DEVICE_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((SLEW_REG != "SLOW") &&
         (SLEW_REG != "FAST") &&
         (SLEW_REG != "MEDIUM"))) begin
      $display("Error: [Unisim %s-105] SLEW attribute is set to %s.  Legal values for this attribute are SLOW, FAST or MEDIUM. Instance: %m", MODULE_NAME, SLEW_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((USE_IBUFDISABLE_REG != "TRUE") &&
         (USE_IBUFDISABLE_REG != "FALSE") &&
         (USE_IBUFDISABLE_REG != "T_CONTROL"))) begin
      $display("Error: [Unisim %s-106] USE_IBUFDISABLE attribute is set to %s.  Legal values for this attribute are TRUE, FALSE or T_CONTROL. Instance: %m", MODULE_NAME, USE_IBUFDISABLE_REG);
      attr_err = 1'b1;
    end
    
    if (attr_err == 1'b1) #1 $finish;
  end
`endif

// begin behavioral model

    wire ts;
    wire T_OR_IBUFDISABLE;
    wire out_val;
    wire disable_out;

    tri0 GTS = glbl.GTS;

    or O1 (ts, GTS, T);
    bufif0 T1 (IO, I, ts);

    and a1 (disable_out, DCITERMDISABLE, IBUFDISABLE);

//    buf B1 (O, IO);
   generate
       case (SIM_DEVICE)
         "7SERIES" : begin
                        assign out_val = 1'b1;
                     end
         default : begin
                        assign out_val = 1'b0;
                     end
        endcase
   endgenerate

    generate
       case (USE_IBUFDISABLE)
          "TRUE" :  begin
                       assign T_OR_IBUFDISABLE = ~T || IBUFDISABLE;
                       assign O = (T_OR_IBUFDISABLE == 1'b1) ? out_val : (T_OR_IBUFDISABLE == 1'b0) ? IO : 1'bx;
                    end
          "FALSE" : begin
                        assign O = IO;
                    end
          "T_CONTROL" : begin
	                   assign O = (T == 1'b1) ? IO : ((T == 1'b0) ? 1'b0 : 1'bx);
			end
       endcase
    endgenerate
//end behavioral model

`ifndef XIL_XECLIB
`ifdef XIL_TIMING
  specify
        (DCITERMDISABLE => O)   = (0:0:0,  0:0:0);
        (DCITERMDISABLE => IO)  = (0:0:0,  0:0:0);
        (I => O)                = (0:0:0,  0:0:0);
        (I => IO)               = (0:0:0,  0:0:0);
        (IO => O)               = (0:0:0,  0:0:0);
        (IBUFDISABLE => O)      = (0:0:0,  0:0:0);
        (IBUFDISABLE => IO)     = (0:0:0,  0:0:0);
        (T => O)               = (0:0:0,  0:0:0);
        (T => IO)              = (0:0:0,  0:0:0);
        specparam PATHPULSE$ = 0;
    endspecify
`endif // `ifdef XIL_TIMING
`endif
endmodule

`endcelldefine
