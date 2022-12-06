`timescale 1ns/10ps

module rom_inferred_2x32
#(
    parameter abits = 12,
    parameter filename = ""  // without '.hex' extension
)
(
    input clk,
    input logic [abits-1: 0] address,
    output logic [63 : 0] data
);

localparam hexfile_lo = {filename, "_lo.hex"};
localparam hexfile_hi = {filename, "_hi.hex"};

rom_inferred_32
#(
    .abits(abits),
    .hex_filename(hexfile_lo)
) w0 (
    .clk(clk),
    .address(address),
    .data(data[31:0])
);


rom_inferred_32
#(
    .abits(abits),
    .hex_filename(hexfile_hi)
) w1 (
    .clk(clk),
    .address(address),
    .data(data[63:32])
);

endmodule: rom_inferred_2x32
