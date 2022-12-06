`timescale 1ns/10ps

module rom_tech
#(
    parameter abits = 12,
    parameter integer log2_dbytes = 3,  // 2^log2_dbytes = number of bytes on data bus
    parameter filename = ""
)
(
    input clk,
    input [abits-1: 0] address,
    output logic [8*(2**log2_dbytes) - 1 : 0] data
);

import config_target_pkg::*;

localparam integer dbytes = (2**log2_dbytes);
localparam integer dbits = 8*dbytes;
localparam int ROM_LENGTH = 2**(abits - log2_dbytes);

// TODO: check and generate dbits == 64, otherwise assert

`ifdef TARGET_INFERRED

    rom_inferred_2x32
    #(
        .abits(abits-log2_dbytes),
	.filename(filename)
    )
    ROM
    (
        .clk(clk),
        .address(address[abits-1: log2_dbytes]),
        .data(data)
    );

`elsif TARGET_KC705

    rom_inferred_2x32
    #(
        .abits(abits-log2_dbytes),
	.filename(filename)
    )
    ROM
    (
        .clk(clk),
        .address(address[abits-1: log2_dbytes]),
        .data(data)
    );

`else
    initial $error("INSTANCE macro is undefined, check technology-dependent memories.");

`endif //target_...

`ifdef DISPLAY_MEMORY_INSTANCE_INFORMATION

initial begin 
  $display("");
  $display("****************************************************");
  $display("rom_tech *******************************************");
  $display("unique_tag = rom_tech_W%0d_D%0d_C%0d", (dbits), 2**abits, (2**abits)*(dbits));
  $display("****************************************************");
  $display("full path     =  %m");
  $display("abits         =  %d",abits);
  $display("Summary ********************************************");
  $display("Width         =  %d",(dbits));
  $display("Depth         =  %d",ROM_LENGTH);
  $display("Capacity      =  %d bits",ROM_LENGTH*(dbits));  
  $display("****************************************************");
  $display("");
end
`endif

endmodule: rom_tech
