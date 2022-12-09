///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2016 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2017.1
//  \   \         Description : Xilinx Unified Simulation Library Component
//  /   /                  D Flip-Flop with Clock Enable and Synchronous Set
// /___/   /\     Filename : FDSE.v
// \   \  /  \
//  \___\/\___\
//
// Revision:
//    08/25/10 - Initial version.
//    10/20/10 - remove unused pin line from table.
//    12/08/11 - add MSGON and XON attributes (CR636891)
//    01/16/12 - 640813 - add MSGON and XON functionality
//    04/16/13 - PR683925 - add invertible pin support.
// End Revision

`timescale  1 ps / 1 ps

`celldefine 

module FDSE #(
  `ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
  parameter MSGON = "TRUE",
  parameter XON = "TRUE",
  `endif
  parameter [0:0] INIT = 1'b1,
  parameter [0:0] IS_C_INVERTED = 1'b0,
  parameter [0:0] IS_D_INVERTED = 1'b0,
  parameter [0:0] IS_S_INVERTED = 1'b0
)(
  output Q,
  
  input C,
  input CE,
  input D,
  input S
);

    reg [0:0] IS_C_INVERTED_REG = IS_C_INVERTED;
    reg [0:0] IS_D_INVERTED_REG = IS_D_INVERTED;
    reg [0:0] IS_S_INVERTED_REG = IS_S_INVERTED;
    
    tri0 glblGSR = glbl.GSR;

`ifdef XIL_TIMING
    wire D_dly, C_dly, CE_dly;
    wire S_dly;
`endif

// begin behavioral model

  reg Q_out;

  assign #100 Q = Q_out;

// end behavioral model

    always @(glblGSR)
      if (glblGSR) 
        assign Q_out = INIT;
      else
        deassign Q_out;

`ifdef XIL_TIMING
generate
if (IS_C_INVERTED == 1'b0) begin : generate_block1
  always @(posedge C_dly)
    if (((S_dly ^ IS_S_INVERTED_REG) && (S !== 1'bz)) || (S === 1'bx && Q_out == 1'b1))
      Q_out <=  1'b1;
    else if (CE_dly || (CE === 1'bz) || ((CE === 1'bx) && (Q_out == (D_dly ^ IS_D_INVERTED_REG))))
      Q_out <=  D_dly ^ IS_D_INVERTED_REG;
end else begin : generate_block1
  always @(negedge C_dly)
    if (((S_dly ^ IS_S_INVERTED_REG) && (S !== 1'bz)) || (S === 1'bx && Q_out == 1'b1))
      Q_out <=  1'b1;
    else if (CE_dly || (CE === 1'bz) || ((CE === 1'bx) && (Q_out == (D_dly ^ IS_D_INVERTED_REG))))
      Q_out <=  D_dly ^ IS_D_INVERTED_REG;
end
endgenerate
`else
generate
if (IS_C_INVERTED == 1'b0) begin : generate_block1
  always @(posedge C)
    if (((S ^ IS_S_INVERTED_REG) && (S !== 1'bz)) || (S === 1'bx && Q_out == 1'b1))
      Q_out <=  1'b1;
    else if (CE || (CE === 1'bz) || ((CE === 1'bx) && (Q_out == (D ^ IS_D_INVERTED_REG))))
      Q_out <=  D ^ IS_D_INVERTED_REG;
end else begin : generate_block1
  always @(negedge C)
    if (((S ^ IS_S_INVERTED_REG) && (S !== 1'bz)) || (S === 1'bx && Q_out == 1'b1))
      Q_out <=  1'b1;
    else if (CE || (CE === 1'bz) || ((CE === 1'bx) && (Q_out == (D ^ IS_D_INVERTED_REG))))
      Q_out <=  D ^ IS_D_INVERTED_REG;
end
endgenerate
`endif

`ifdef XIL_TIMING
    reg notifier;
    wire notifier1;
`endif

`ifdef XIL_TIMING
    wire ngsr, in_out;
    wire nset;
    wire in_clk_enable, in_clk_enable_p, in_clk_enable_n;
    wire ce_clk_enable, ce_clk_enable_p, ce_clk_enable_n;
    reg init_enable = 1'b1;
    wire set_clk_enable, set_clk_enable_p, set_clk_enable_n;
`endif

`ifdef XIL_TIMING
    not (ngsr, glblGSR);
    xor (in_out, D_dly, IS_D_INVERTED_REG, Q_out);
    not (nset, (S_dly ^ IS_S_INVERTED_REG) && (S !== 1'bz));

    and (in_clk_enable, ngsr, nset, CE || (CE === 1'bz));
    and (ce_clk_enable, ngsr, nset, in_out);
    and (set_clk_enable, ngsr, CE || (CE === 1'bz), D ^ IS_D_INVERTED_REG);
    always @(negedge nset) init_enable = (MSGON =="TRUE") && ~glblGSR && (Q_out ^ INIT);

    assign notifier1 = (XON == "FALSE") ?  1'bx : notifier;
    assign ce_clk_enable_n = (MSGON =="TRUE") && ce_clk_enable && (IS_C_INVERTED == 1'b1);
    assign in_clk_enable_n = (MSGON =="TRUE") && in_clk_enable && (IS_C_INVERTED == 1'b1);
    assign set_clk_enable_n = (MSGON =="TRUE") && set_clk_enable && (IS_C_INVERTED == 1'b1);
    assign ce_clk_enable_p = (MSGON =="TRUE") && ce_clk_enable && (IS_C_INVERTED == 1'b0);
    assign in_clk_enable_p = (MSGON =="TRUE") && in_clk_enable && (IS_C_INVERTED == 1'b0);
    assign set_clk_enable_p = (MSGON =="TRUE") && set_clk_enable && (IS_C_INVERTED == 1'b0);
`endif

`ifdef XIL_TIMING
  specify
  (C => Q) = (100:100:100, 100:100:100);
  $period (negedge C &&& CE, 0:0:0, notifier);
  $period (posedge C &&& CE, 0:0:0, notifier);
  $setuphold (negedge C, negedge CE, 0:0:0, 0:0:0, notifier,ce_clk_enable_n,ce_clk_enable_n,C_dly,CE_dly);
  $setuphold (negedge C, negedge D, 0:0:0, 0:0:0, notifier,in_clk_enable_n,in_clk_enable_n,C_dly,D_dly);
  $setuphold (negedge C, negedge S, 0:0:0, 0:0:0, notifier,set_clk_enable_n,set_clk_enable_n,C_dly,S_dly);
  $setuphold (negedge C, posedge CE, 0:0:0, 0:0:0, notifier,ce_clk_enable_n,ce_clk_enable_n,C_dly,CE_dly);
  $setuphold (negedge C, posedge D, 0:0:0, 0:0:0, notifier,in_clk_enable_n,in_clk_enable_n,C_dly,D_dly);
  $setuphold (negedge C, posedge S, 0:0:0, 0:0:0, notifier,set_clk_enable_n,set_clk_enable_n,C_dly,S_dly);
  $setuphold (posedge C, negedge CE, 0:0:0, 0:0:0, notifier,ce_clk_enable_p,ce_clk_enable_p,C_dly,CE_dly);
  $setuphold (posedge C, negedge D, 0:0:0, 0:0:0, notifier,in_clk_enable_p,in_clk_enable_p,C_dly,D_dly);
  $setuphold (posedge C, negedge S, 0:0:0, 0:0:0, notifier,set_clk_enable_p,set_clk_enable_p,C_dly,S_dly);
  $setuphold (posedge C, posedge CE, 0:0:0, 0:0:0, notifier,ce_clk_enable_p,ce_clk_enable_p,C_dly,CE_dly);
  $setuphold (posedge C, posedge D, 0:0:0, 0:0:0, notifier,in_clk_enable_p,in_clk_enable_p,C_dly,D_dly);
  $setuphold (posedge C, posedge S, 0:0:0, 0:0:0, notifier,set_clk_enable_p,set_clk_enable_p,C_dly,S_dly);
  $width (negedge C &&& CE, 0:0:0, 0, notifier);
  $width (negedge S &&& init_enable, 0:0:0, 0, notifier);
  $width (posedge C &&& CE, 0:0:0, 0, notifier);
  $width (posedge S &&& init_enable, 0:0:0, 0, notifier);
  specparam PATHPULSE$ = 0;
  endspecify
`endif
endmodule

`endcelldefine
