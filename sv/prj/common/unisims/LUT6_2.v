///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  6-input Look-Up-Table with Two General Outputs
// /___/   /\     Filename : LUT6_2.v
// \   \  /  \
//  \___\/\___\
//
// Revision:
///////////////////////////////////////////////////////////////////////////////
//    08/08/64 - Initial version.
//    05/30/07 - Change timescale to 1 ps / 1ps.
//    10/20/14 - Removed b'x support (CR 817718).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module LUT6_2 (O5, O6, I0, I1, I2, I3, I4, I5);

  parameter [63:0] INIT = 64'h0000000000000000;

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";

`endif

  input I0, I1, I2, I3, I4, I5;

  output O5, O6;

  reg [63:0] init_reg = INIT;
  reg [31:0] init_l, init_h;
  reg O_l, O_h, tmp;
  reg O5, O6;
  
  initial begin
     init_l = init_reg[31:0];
     init_h = init_reg[63:32];
  end

  always @(I5 or O_l or O_h) begin
    O5 = O_l;
    if (I5 == 1)
      O6 = O_h;
    else if (I5 == 0)
      O6 = O_l;
    else begin
      if (O_h == 0 && O_l == 0)
         O6 = 1'b0;
      else if (O_h == 1 && O_l == 1)
         O6 = 1'b1;
      else
         O6 = 1'b0;
      end
   end
   

  always @( I4 or I3 or  I2 or  I1 or  I0 )  begin
    tmp =  I0 ^ I1  ^ I2 ^ I3 ^ I4;
    if ( tmp == 0 || tmp == 1) begin
        O_l = init_l[{I4, I3, I2, I1, I0}];
        O_h = init_h[{I4, I3, I2, I1, I0}];
    end
    else begin
      O_l =  lut4_mux4 ( 
                        { lut6_mux8 ( init_l[31:24], {I2, I1, I0}),
                          lut6_mux8 ( init_l[23:16], {I2, I1, I0}),
                          lut6_mux8 ( init_l[15:8], {I2, I1, I0}),
                          lut6_mux8 ( init_l[7:0], {I2, I1, I0}) }, { I4, I3});
      O_h =  lut4_mux4 ( 
                        { lut6_mux8 ( init_h[31:24], {I2, I1, I0}),
                          lut6_mux8 ( init_h[23:16], {I2, I1, I0}),
                          lut6_mux8 ( init_h[15:8], {I2, I1, I0}),
                          lut6_mux8 ( init_h[7:0], {I2, I1, I0}) }, { I4, I3});
     end
    end

  function lut6_mux8;
  input [7:0] d;
  input [2:0] s;
   
  begin

       if ((s[2]^s[1]^s[0] ==1) || (s[2]^s[1]^s[0] ==0))
           
           lut6_mux8 = d[s];

         else
           if ( ~(|d))
                 lut6_mux8 = 1'b0;
           else if ((&d))
                 lut6_mux8 = 1'b1;
           else if (((s[1]^s[0] ==1'b1) || (s[1]^s[0] ==1'b0)) && (d[{1'b0,s[1:0]}]===d[{1'b1,s[1:0]}]))
                 lut6_mux8 = d[{1'b0,s[1:0]}];
           else if (((s[2]^s[0] ==1) || (s[2]^s[0] ==0)) && (d[{s[2],1'b0,s[0]}]===d[{s[2],1'b1,s[0]}]))
                 lut6_mux8 = d[{s[2],1'b0,s[0]}];
           else if (((s[2]^s[1] ==1) || (s[2]^s[1] ==0)) && (d[{s[2],s[1],1'b0}]===d[{s[2],s[1],1'b1}]))
                 lut6_mux8 = d[{s[2],s[1],1'b0}];
           else if (((s[0] ==1) || (s[0] ==0)) && (d[{1'b0,1'b0,s[0]}]===d[{1'b0,1'b1,s[0]}]) &&
              (d[{1'b0,1'b0,s[0]}]===d[{1'b1,1'b0,s[0]}]) && (d[{1'b0,1'b0,s[0]}]===d[{1'b1,1'b1,s[0]}]))
                 lut6_mux8 = d[{1'b0,1'b0,s[0]}];
           else if (((s[1] ==1) || (s[1] ==0)) && (d[{1'b0,s[1],1'b0}]===d[{1'b0,s[1],1'b1}]) &&
              (d[{1'b0,s[1],1'b0}]===d[{1'b1,s[1],1'b0}]) && (d[{1'b0,s[1],1'b0}]===d[{1'b1,s[1],1'b1}]))
                 lut6_mux8 = d[{1'b0,s[1],1'b0}];
           else if (((s[2] ==1) || (s[2] ==0)) && (d[{s[2],1'b0,1'b0}]===d[{s[2],1'b0,1'b1}]) &&
              (d[{s[2],1'b0,1'b0}]===d[{s[2],1'b1,1'b0}]) && (d[{s[2],1'b0,1'b0}]===d[{s[2],1'b1,1'b1}]))
                 lut6_mux8 = d[{s[2],1'b0,1'b0}];
           else
                 lut6_mux8 = 1'b0;
   end
  endfunction


  function lut4_mux4;
  input [3:0] d;
  input [1:0] s;
   
  begin

       if ((s[1]^s[0] ==1) || (s[1]^s[0] ==0))

           lut4_mux4 = d[s];

         else if ((d[0] === d[1]) && (d[2] === d[3])  && (d[0] === d[2]) )
           lut4_mux4 = d[0];
         else if ((s[1] == 0) && (d[0] === d[1]))
           lut4_mux4 = d[0];
         else if ((s[1] == 1) && (d[2] === d[3]))
           lut4_mux4 = d[2];
         else if ((s[0] == 0) && (d[0] === d[2]))
           lut4_mux4 = d[0];
         else if ((s[0] == 1) && (d[1] === d[3]))
           lut4_mux4 = d[1];
         else
           lut4_mux4 = 1'b0;

   end
  endfunction

endmodule

`endcelldefine
