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

`ifdef TARGET_INFERRED
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
`elsif TARGET_KC705
    ram_fpga_distr
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
`else
    initial $error("INSTANCE macro is undefined, check technology-dependent memories.");
`endif



`ifdef DISPLAY_MEMORY_INSTANCE_INFORMATION
initial begin 
  $display("");
  $display("****************************************************");
  $display("ram_tech *******************************************");
  $display("unique_tag = ram_tech_W%0d_D%0d_C%0d", dbits, 2**abits, (2**abits)*dbits);
  $display("****************************************************");
  $display("full path     =  %m");
  $display("abits         =  %d",abits);
  $display("dbits         =  %d",dbits);
  $display("Summary ********************************************");
  $display("Width         =  %d",dbits);
  $display("Depth         =  %d",2**abits);
  $display("Capacity      =  %d bits",(2**abits)*dbits);  
  $display("****************************************************");
  $display("");
end
`endif

endmodule: ram_tech