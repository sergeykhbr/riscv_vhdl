`timescale 1ns/10ps

module rom_tech #(
    parameter int abits = 6,
    parameter int log2_dbytes = 3,
    parameter filename = ""
)
(
    input logic i_clk,                                      // CPU clock
    input logic [abits-1:0] i_addr,
    output logic [(8 * (2**log2_dbytes))-1:0] o_rdata
);


localparam integer dbytes = (2**log2_dbytes);
localparam integer dbits = 8*dbytes;
localparam int ROM_LENGTH = 2**(abits - log2_dbytes);

// TODO: check and generate dbits == 64, otherwise assert

    rom_inferred_2x32
    #(
        .abits(abits-log2_dbytes),
	.filename(filename)
    )
    ROM
    (
        .clk(i_clk),
        .address(i_addr[abits-1: log2_dbytes]),
        .data(o_rdata)
    );

endmodule: rom_tech
