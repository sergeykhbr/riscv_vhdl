`timescale 1ns/10ps

(* dont_touch="true" *) module rom_fpga_64
#(
    parameter abits = 12,
    parameter hex_filename = ""
)
(
    input clk,
    input [31 : 0] address_lo,
    input [31 : 0] address_hi,

    input logic [63 : 0] data_in,
    input logic en,
    input logic [7 : 0]  we,

    output logic [63 : 0] data
);

import types_amba_pkg::*;

localparam int ROM_LENGTH = (1<<(abits - 3));

reg [63:0] rom_lo [ROM_LENGTH-1 : 0];
reg [63:0] rom_hi [ROM_LENGTH-1 : 0];

logic [31:0] rom_data_lo;
logic [31:0] rom_data_hi;

initial
begin
  $readmemh(hex_filename, rom_lo);
  $readmemh(hex_filename, rom_hi);
end

  // ROM READ FIRST:
  integer i;
  always_ff @(posedge clk)
  begin
    if (en) begin
      for (i = 0; i < 4; i = i + 1) begin
        if (we[i])
          rom_lo[address_lo[abits - 1 : 3]][i*8 +: 8] <= data_in[i*8 +: 8];
      end
    end
  end

  always_ff @(posedge clk)
  begin
    if (en) begin
      rom_data_lo <= rom_lo[address_lo[abits - 1 : 3]][31:0];
    end
  end

  integer j;
  always_ff @(posedge clk)
  begin
    if (en) begin
      for (j = 4; j < 8; j = j + 1) begin
        if (we[j])
          rom_hi[address_hi[abits - 1 : 3]][j*8 +: 8] <= data_in[j*8 +: 8];
      end
    end
  end

  always_ff @(posedge clk)
  begin
    if (en) begin
      rom_data_hi <= rom_hi[address_hi[abits - 1 : 3]][63:32];
    end
  end

assign data = {rom_data_hi, rom_data_lo};

endmodule: rom_fpga_64
