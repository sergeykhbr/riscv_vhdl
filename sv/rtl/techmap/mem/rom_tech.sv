`timescale 1ns/10ps

module rom_tech
#(
    parameter abits = 12,
    parameter sim_hexfile = ""
)
(
    input                                                 clk,
    input types_amba_pkg::global_addr_array_type                address,
    output logic [types_amba_pkg::CFG_SYSBUS_DATA_BITS - 1 : 0] data
);

import config_target_pkg::*;

`ifdef TARGET_INFERRED

    rom_inferred
    #(
        .abits(abits),
	.hex_filename(sim_hexfile)
    )
    ROM
    (
        .clk(clk),
        .address(address),
        .data(data)
    );

`elsif TARGET_KC705

    rom_fpga_64
    #(
        .abits(abits),
        .hex_filename(sim_hexfile)
    )
    ROM64
    (
        .clk(clk),
        .address_lo(address[0]),
        .address_hi(address[1]),
        .data_in(64'b0),
        .en(1'b1),
        .we(8'b0),
        .data(data[63:0])
    );

`else
    initial $error("INSTANCE macro is undefined, check technology-dependent memories.");

`endif //WF_USE_INFERRED_ROMS

`ifdef DISPLAY_MEMORY_INSTANCE_INFORMATION

localparam int ROM_LENGTH = 2**(abits - $clog2(types_amba_pkg::CFG_SYSBUS_DATA_BYTES));

initial begin 
  $display("");
  $display("****************************************************");
  $display("rom_tech *******************************************");
  $display("unique_tag = rom_tech_W%0d_D%0d_C%0d", (types_amba_pkg::CFG_SYSBUS_DATA_BITS), 2**abits, (2**abits)*(amba_pkg::CFG_SYSBUS_DATA_BITS));
  $display("****************************************************");
  $display("full path     =  %m");
  $display("abits         =  %d",abits);
  $display("Summary ********************************************");
  $display("Width         =  %d",(types_amba_pkg::CFG_SYSBUS_DATA_BITS));
  $display("Depth         =  %d",ROM_LENGTH);
  $display("Capacity      =  %d bits",ROM_LENGTH*(types_amba_pkg::CFG_SYSBUS_DATA_BITS));  
  $display("****************************************************");
  $display("");
end
`endif

endmodule: rom_tech
