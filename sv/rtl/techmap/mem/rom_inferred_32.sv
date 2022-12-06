`timescale 1ns/10ps

module rom_inferred_32
#(
    parameter abits = 12,
    parameter hex_filename = ""
)
(
    input clk,
    input [abits-1 : 0] address,
    output logic [31 : 0] data
);

localparam int ROM_LENGTH = 2**abits;
logic [31:0] rom [0 : ROM_LENGTH-1];

initial $readmemh(hex_filename, rom);

generate

    always_ff @(posedge clk) begin
        data <= rom[address];
    end

endgenerate

endmodule: rom_inferred_32
