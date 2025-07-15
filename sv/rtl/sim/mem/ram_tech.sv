`timescale 1ns/10ps

module ram_tech
#(
    parameter abits = 12,
    parameter dbits = 64
)
(
    input                      i_clk,
    input [abits - 1:0]        i_addr,
    output logic [dbits - 1:0] o_rdata,
    input                      i_wena,
    input [dbits - 1:0]        i_wdata
);

    ram_inferred
    #(
        .abits(abits),
        .dbits(dbits)
    ) RAM (
        .i_clk,
        .i_addr,
        .o_rdata,
        .i_wena,
        .i_wdata
    );

endmodule: ram_tech