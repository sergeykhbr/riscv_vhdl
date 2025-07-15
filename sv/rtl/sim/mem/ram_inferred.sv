// Inferred RAM Single Port
// Write First Mode of operation

`timescale 1ns/10ps

module ram_inferred
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

logic [dbits - 1:0] r_data;
logic [dbits - 1:0] ram [0 : 2 ** abits - 1];

always_ff @(posedge i_clk)
begin: main_proc
    if(i_wena == 1'b1) begin
        ram[i_addr] <= i_wdata;
        r_data <= i_wdata;
    end else
        r_data <= ram[i_addr];
end: main_proc

assign o_rdata = r_data;

endmodule: ram_inferred
