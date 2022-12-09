///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2020 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2020.3
//  \   \         Description : Xilinx Timing Simulation Library Component
//  /   /                  3-State Diffential Signaling I/O Buffer
// /___/   /\     Filename : IOBUFDS_DIFF_OUT_DCIEN.v
// \   \  /  \    Timestamp : Thu Apr 29 14:59:30 PDT 2010
//  \___\/\___\
//
// Revision:
//    04/29/10 - Initial version.
//    03/28/11 - CR 603466 fix
//    06/15/11 - CR 613347 -- made ouput logic_1 when IBUFDISABLE is active
//    08/31/11 - CR 623170 -- Tristate powergating support
//    09/20/11 - CR 625564 -- Fixed Tristate powergating polarity
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    10/22/14 - Added #1 to $finish (CR 808642).
// End Revision

`timescale  1 ps / 1 ps


`celldefine

module IOBUFDS_DIFF_OUT_DCIEN #(
`ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
`endif
  parameter DIFF_TERM = "FALSE",
  parameter DQS_BIAS = "FALSE",
  parameter IBUF_LOW_PWR = "TRUE",
  parameter IOSTANDARD = "DEFAULT",
  parameter SIM_DEVICE = "7SERIES",
  parameter USE_IBUFDISABLE = "TRUE"
)(
  output O,
  output OB,

  inout IO,
  inout IOB,

  input DCITERMDISABLE,
  input I,
  input IBUFDISABLE,
  input TM,
  input TS
);

// define constants
  localparam MODULE_NAME = "IOBUFDS_DIFF_OUT_DCIEN";

// Parameter encodings and registers
  localparam DIFF_TERM_FALSE = 0;
  localparam DIFF_TERM_TRUE = 1;
  localparam DQS_BIAS_FALSE = 0;
  localparam DQS_BIAS_TRUE = 1;
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
  localparam USE_IBUFDISABLE_FALSE = 0;
  localparam USE_IBUFDISABLE_TRUE = 1;
  localparam USE_IBUFDISABLE_T_CONTROL = 2;

  
// include dynamic registers - XILINX test only
  reg trig_attr = 1'b0;
  localparam [40:1] DIFF_TERM_REG = DIFF_TERM;
  localparam [40:1] DQS_BIAS_REG = DQS_BIAS;
  localparam [40:1] IBUF_LOW_PWR_REG = IBUF_LOW_PWR;
  localparam [56:1] IOSTANDARD_REG = IOSTANDARD;
  localparam [144:1] SIM_DEVICE_REG = SIM_DEVICE;
  localparam [72:1] USE_IBUFDISABLE_REG = USE_IBUFDISABLE;

`ifdef XIL_XECLIB
  wire DIFF_TERM_BIN;
  wire DQS_BIAS_BIN;
  wire IBUF_LOW_PWR_BIN;
  wire [4:0] SIM_DEVICE_BIN;
  wire [1:0] USE_IBUFDISABLE_BIN;
`else
  reg DIFF_TERM_BIN;
  reg DQS_BIAS_BIN;
  reg IBUF_LOW_PWR_BIN;
  reg [4:0] SIM_DEVICE_BIN;
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
  assign DIFF_TERM_BIN =
      (DIFF_TERM_REG == "FALSE") ? DIFF_TERM_FALSE :
      (DIFF_TERM_REG == "TRUE") ? DIFF_TERM_TRUE :
       DIFF_TERM_FALSE;
  
  assign DQS_BIAS_BIN =
      (DQS_BIAS_REG == "FALSE") ? DQS_BIAS_FALSE :
      (DQS_BIAS_REG == "TRUE") ? DQS_BIAS_TRUE :
       DQS_BIAS_FALSE;
  
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
  
  assign USE_IBUFDISABLE_BIN =
      (USE_IBUFDISABLE_REG == "TRUE") ? USE_IBUFDISABLE_TRUE :
      (USE_IBUFDISABLE_REG == "FALSE") ? USE_IBUFDISABLE_FALSE :
      (USE_IBUFDISABLE_REG == "T_CONTROL") ? USE_IBUFDISABLE_T_CONTROL :
       USE_IBUFDISABLE_TRUE;
  
`else
  always @ (trig_attr) begin
  #1;
  DIFF_TERM_BIN =
      (DIFF_TERM_REG == "FALSE") ? DIFF_TERM_FALSE :
      (DIFF_TERM_REG == "TRUE") ? DIFF_TERM_TRUE :
       DIFF_TERM_FALSE;
  
  DQS_BIAS_BIN =
      (DQS_BIAS_REG == "FALSE") ? DQS_BIAS_FALSE :
      (DQS_BIAS_REG == "TRUE") ? DQS_BIAS_TRUE :
       DQS_BIAS_FALSE;
  
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
        ((DIFF_TERM_REG != "FALSE") &&
         (DIFF_TERM_REG != "TRUE"))) begin
      $display("Error: [Unisim %s-102] DIFF_TERM attribute is set to %s.  Legal values for this attribute are FALSE or TRUE. Instance: %m", MODULE_NAME, DIFF_TERM_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((DQS_BIAS_REG != "FALSE") &&
         (DQS_BIAS_REG != "TRUE"))) begin
      $display("Error: [Unisim %s-103] DQS_BIAS attribute is set to %s.  Legal values for this attribute are FALSE or TRUE. Instance: %m", MODULE_NAME, DQS_BIAS_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((IBUF_LOW_PWR_REG != "TRUE") &&
         (IBUF_LOW_PWR_REG != "FALSE"))) begin
      $display("Error: [Unisim %s-104] IBUF_LOW_PWR attribute is set to %s.  Legal values for this attribute are TRUE or FALSE. Instance: %m", MODULE_NAME, IBUF_LOW_PWR_REG);
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
      $display("Error: [Unisim %s-107] SIM_DEVICE attribute is set to %s.  Legal values for this attribute are 7SERIES, ULTRASCALE, VERSAL_AI_CORE, VERSAL_AI_CORE_ES1, VERSAL_AI_EDGE, VERSAL_AI_EDGE_ES1, VERSAL_AI_EDGE_ES2, VERSAL_AI_RF, VERSAL_AI_RF_ES1, VERSAL_AI_RF_ES2, VERSAL_HBM, VERSAL_HBM_ES1, VERSAL_HBM_ES2, VERSAL_PREMIUM, VERSAL_PREMIUM_ES1, VERSAL_PREMIUM_ES2, VERSAL_PRIME or VERSAL_PRIME_ES1. Instance: %m", MODULE_NAME, SIM_DEVICE_REG);
      attr_err = 1'b1;
    end
    
    if ((attr_test == 1'b1) ||
        ((USE_IBUFDISABLE_REG != "TRUE") &&
         (USE_IBUFDISABLE_REG != "FALSE") &&
         (USE_IBUFDISABLE_REG != "T_CONTROL"))) begin
      $display("Error: [Unisim %s-108] USE_IBUFDISABLE attribute is set to %s.  Legal values for this attribute are TRUE, FALSE or T_CONTROL. Instance: %m", MODULE_NAME, USE_IBUFDISABLE_REG);
      attr_err = 1'b1;
    end
    
    if (attr_err == 1'b1) #1 $finish;
  end
`endif

// begin behavioral model



    wire t1, t2, out_val, out_b_val;
    wire T_OR_IBUFDISABLE_1;
    wire T_OR_IBUFDISABLE_2;

    tri0 GTS = glbl.GTS;

    or O1 (t1, GTS, TM);
    bufif0 B1 (IO, I, t1);

    or O2 (t2, GTS, TS);
    notif0 N2 (IOB, I, t2);

    reg O_int, OB_int;
    reg DQS_BIAS_BINARY = 1'b0;


  generate
       case (SIM_DEVICE)
         "7SERIES" : begin
                        assign out_val = 1'b1;
                        assign out_b_val = 1'b1;
                     end
         "ULTRASCALE" : begin
                        assign out_val = 1'b0;
                        assign out_b_val = 1'bx;
                     end
         default : begin
	 		assign out_val = 1'b0;
	 		assign out_b_val = 1'b0;
	             end
        endcase
   endgenerate

    always @(IO or IOB or DQS_BIAS_BINARY) begin
        if (IO == 1'b1 && IOB == 1'b0) begin
            O_int  <= IO;
            OB_int <= ~IO;
        end
        else if (IO == 1'b0 && IOB == 1'b1) begin
            O_int  <= IO;
            OB_int <= ~IO;
        end
        else if ((IO === 1'bz || IO == 1'b0) && (IOB === 1'bz || IOB == 1'b1)) begin
          if (DQS_BIAS_BINARY == 1'b1) begin
            O_int <= 1'b0;
            OB_int <= 1'b1;
          end else begin
            O_int <= 1'bx;
            OB_int <= 1'bx;
	  end
        end
        else begin
            O_int  <= 1'bx;
            OB_int <= 1'bx;
        end
    end

    generate
       case (USE_IBUFDISABLE)
          "TRUE" :  begin
                       assign T_OR_IBUFDISABLE_1 = ~TM || IBUFDISABLE;
                       assign T_OR_IBUFDISABLE_2 = ~TS || IBUFDISABLE;
                       assign O = (T_OR_IBUFDISABLE_1 == 1'b1) ? out_val : (T_OR_IBUFDISABLE_1 == 1'b0) ? O_int : 1'bx;
                       assign OB = (T_OR_IBUFDISABLE_2 == 1'b1) ? out_b_val : (T_OR_IBUFDISABLE_2 == 1'b0) ? OB_int : 1'bx;
                    end
          "FALSE" : begin
                        assign O = O_int;
                        assign OB = OB_int;
                    end
          "T_CONTROL" : begin
                           assign O = (TM == 1'b1) ? IO : ((TM == 1'b0) ? 1'b0 : 1'bx);
                           assign OB = (TS == 1'b1) ? ~IO : ((TS == 1'b0) ? 1'b1 : 1'bx);
                        end
	                  
       endcase
    endgenerate
//end behavioral model

`ifndef XIL_XECLIB
`ifdef XIL_TIMING
    specify
        (DCITERMDISABLE => O)   = (0:0:0,  0:0:0);
        (DCITERMDISABLE => OB)  = (0:0:0,  0:0:0);
        (DCITERMDISABLE => IO)  = (0:0:0,  0:0:0);
        (DCITERMDISABLE => IOB) = (0:0:0,  0:0:0);
        (I => O)                = (0:0:0,  0:0:0);
        (I => OB)               = (0:0:0,  0:0:0);
        (I => IO)               = (0:0:0,  0:0:0);
        (I => IOB)              = (0:0:0,  0:0:0);
        (IO => O)               = (0:0:0,  0:0:0);
        (IO => OB)              = (0:0:0,  0:0:0);
        (IO => IOB)             = (0:0:0,  0:0:0);
        (IOB => O)              = (0:0:0,  0:0:0);
        (IOB => OB)             = (0:0:0,  0:0:0);
        (IOB => IO)             = (0:0:0,  0:0:0);
        (IBUFDISABLE => O)      = (0:0:0,  0:0:0);
        (IBUFDISABLE => OB)     = (0:0:0,  0:0:0);
        (IBUFDISABLE => IO)     = (0:0:0,  0:0:0);
        (IBUFDISABLE => IOB)    = (0:0:0,  0:0:0);
        (TM => O)               = (0:0:0,  0:0:0);
        (TM => OB)              = (0:0:0,  0:0:0);
        (TM => IO)              = (0:0:0,  0:0:0);
        (TM => IOB)             = (0:0:0,  0:0:0);
        (TS => O)               = (0:0:0,  0:0:0);
        (TS => OB)              = (0:0:0,  0:0:0);
        (TS => IO)              = (0:0:0,  0:0:0);
        (TS => IOB)             = (0:0:0,  0:0:0);
        specparam PATHPULSE$ = 0;
    endspecify
`endif // `ifdef XIL_TIMING
`endif
    
endmodule

`endcelldefine
