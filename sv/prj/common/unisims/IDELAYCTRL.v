///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor      : Xilinx
// \   \   \/     Version     : 2015.3
//  \   \         Description : Xilinx Unified Simulation Library Component
//  /   /                       IDELAYE3/ODELAYE3 Tap Delay Value Control
// /___/   /\     Filename    : IDELAYCTRL.v
// \   \  /  \
//  \___\/\___\
//
// Revision:
//    03/23/04 - Initial version.
//    03/11/05 - Added LOC parameter and initialized outpus.
//    04/10/07 - CR 436682 fix, disable activity when rst is high
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    06/01/15 - 850338 - Added SIM_DEVICE and warning
// End Revision

`timescale 1 ps / 1 ps 

`celldefine

module IDELAYCTRL #(
`ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
`endif
  parameter SIM_DEVICE = "7SERIES"
)(
  output RDY,

  input REFCLK,
  input RST
);

// define constants
  localparam MODULE_NAME = "IDELAYCTRL";

// Parameter encodings and registers
  localparam SIM_DEVICE_7SERIES = 0;
  localparam SIM_DEVICE_ULTRASCALE = 1;

  reg trig_attr = 1'b0;
// include dynamic registers - XILINX test only
//`ifdef XIL_DR
//  `include "IDELAYCTRL_dr.v"
//`else
  localparam [80:1] SIM_DEVICE_REG = SIM_DEVICE;
//`endif

`ifdef XIL_ATTR_TEST
  reg attr_test = 1'b1;
`else
  reg attr_test = 1'b0;
`endif
  reg attr_err = 1'b0;

  reg RDY_out = 0;

  wire REFCLK_in;
  wire RST_in;

`ifdef XIL_TIMING
  wire REFCLK_delay;
  wire RST_delay;
`endif

  assign RDY = RDY_out;

`ifdef XIL_TIMING
  assign REFCLK_in = REFCLK_delay;
  assign RST_in = RST_delay;
`else
  assign REFCLK_in = REFCLK;
  assign RST_in = RST;
`endif

    time clock_edge;
    reg [63:0] period;
    reg clock_low, clock_high;
    reg clock_posedge, clock_negedge;
    reg lost;
    reg msg_flag = 1'b0;


  initial begin
    #1;
    trig_attr = ~trig_attr;
  end
  
  always @ (trig_attr) begin
    #1;
    if ((attr_test == 1'b1) ||
        ((SIM_DEVICE_REG != "7SERIES") &&
         (SIM_DEVICE_REG != "ULTRASCALE"))) begin
      $display("Error: [Unisim %s-104] SIM_DEVICE attribute is set to %s.  Legal values for this attribute are 7SERIES or ULTRASCALE. Instance: %m", MODULE_NAME, SIM_DEVICE_REG);
      attr_err = 1'b1;
    end
    
    if (attr_err == 1'b1) #1 $finish;
  end


    always @(RST_in, lost) begin

   if (RST_in == 1'b1) begin
     RDY_out <= 1'b0;
   end else if (lost == 1)
     RDY_out <= 1'b0;
   else if (RST_in == 1'b0 && lost == 0)
     RDY_out <= 1'b1;
    end
   
   always @(posedge RST_in) begin
     if (SIM_DEVICE_REG == "ULTRASCALE" && msg_flag == 1'b0) begin 
       $display("Info: [Unisim %s-1] RST simulation behaviour for SIM_DEVICE %s may not match hardware behaviour when I/ODELAY DELAY_FORMAT = TIME if SelectIO User Guide recommendation for I/ODELAY connections or reset sequence are not followed. For more information, refer to the Select IO Userguide. Instance: %m", MODULE_NAME, SIM_DEVICE_REG);
      msg_flag <= 1'b1;
     end
   end
    initial begin
   clock_edge <= 0;
   clock_high <= 0;
   clock_low <= 0;
   lost <= 1;
   period <= 0;
    end


    always @(posedge REFCLK_in) begin
      if(RST_in == 1'b0) begin
   clock_edge <= $time;
   if (period != 0 && (($time - clock_edge) <= (1.5 * period)))
       period <= $time - clock_edge;
   else if (period != 0 && (($time - clock_edge) > (1.5 * period)))
       period <= 0;
   else if ((period == 0) && (clock_edge != 0))
       period <= $time - clock_edge;
      end
    end
    
    always @(posedge REFCLK_in) begin
   clock_low <= 1'b0;
   clock_high <= 1'b1;
   if (period != 0)
       lost <= 1'b0;
   clock_posedge <= 1'b0;
   #((period * 9.1) / 10)
   if ((clock_low != 1'b1) && (clock_posedge != 1'b1))
       lost <= 1;
    end
    
    always @(posedge REFCLK_in) begin
   clock_negedge <= 1'b1;
    end
    
    always @(negedge REFCLK_in) begin
   clock_posedge <= 1'b1;
    end
    
    always @(negedge REFCLK_in) begin
   clock_high  <= 1'b0;
   clock_low   <= 1'b1;
   if (period != 0)
       lost <= 1'b0;
   clock_negedge <= 1'b0;
   #((period * 9.1) / 10)
   if ((clock_high != 1'b1) && (clock_negedge != 1'b1))
       lost <= 1;
    end

//*** Timing Checks Start here
`ifdef XIL_TIMING
  reg notifier;
`endif

  specify
  (RST => RDY) = (0:0:0, 0:0:0);
  (posedge RST => (RDY +: 0)) = (0:0:0, 0:0:0);
  (REFCLK => RDY) = (100:100:100, 100:100:100);
`ifdef XIL_TIMING
    $period (negedge REFCLK, 0:0:0, notifier);
    $period (posedge REFCLK, 0:0:0, notifier);
    $recrem (negedge RST, posedge REFCLK, 0:0:0, 0:0:0, notifier, , , RST_delay, REFCLK_delay);
    $recrem (posedge RST, posedge REFCLK, 0:0:0, 0:0:0, notifier, , , RST_delay, REFCLK_delay);
    $width (negedge REFCLK, 0:0:0, 0, notifier);
    $width (negedge RST, 0:0:0, 0, notifier);
    $width (posedge REFCLK, 0:0:0, 0, notifier);
    $width (posedge RST, 0:0:0, 0, notifier);
`endif
    specparam PATHPULSE$ = 0;
  endspecify

endmodule

`endcelldefine
