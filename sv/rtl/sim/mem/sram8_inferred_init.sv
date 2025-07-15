//--------------------------------------------------------------------------
//! @file
//! @author     Sergey Khabarov
//! @brief      8-bits memory block with the generic data size parameter.
//! @details    This module absolutely similar to the 'inferred' implementation
//!             but it support initialization of the SRAM.
//!             This feature is very useful during RTL simulation so that
//!             current FW supports skipping of the copying FwImage state.
//----------------------------------------------------------------------------
//
// Write First Mode of operation


module sram8_inferred_init #(
    parameter integer abits = 12,
    parameter integer byte_idx = 0,
    parameter init_file = ""
)
(
    input clk,
    input [abits-1 : 0] address,
    output logic [7:0] rdata,
    input we,
    input [7:0] wdata
);

localparam integer SRAM_LENGTH = 2**abits;

// romimage only 256 KB, but SRAM is 512 KB so we initialize one
// half of sram = 32768 * 8 = 256 KB

const integer FILE_IMAGE_LINES_TOTAL = 32768;

typedef logic [7:0] ram_type [0 : SRAM_LENGTH-1];

ram_type ram;
logic [7 : 0] read_data;

function init_ram(input file_name);
 logic [63:0] temp_mem [0 : SRAM_LENGTH-1];
 $readmemh(init_file, temp_mem);
 for (int i = 0; i < SRAM_LENGTH; i++)
  case(byte_idx)
   0: ram[i] = temp_mem[i][0  +: 8];
   1: ram[i] = temp_mem[i][8  +: 8];
   2: ram[i] = temp_mem[i][16 +: 8];
   3: ram[i] = temp_mem[i][24 +: 8];
   4: ram[i] = temp_mem[i][32 +: 8];
   5: ram[i] = temp_mem[i][40 +: 8];
   6: ram[i] = temp_mem[i][48 +: 8];
   7: ram[i] = temp_mem[i][56 +: 8];
   default: ram[i] = temp_mem[i][0 +: 8];
  endcase
endfunction

//! @warning SIMULATION INITIALIZATION
initial begin
 void'(init_ram(init_file));
end

always_ff@(posedge clk)
begin
  if (we) begin
    ram[address] <= wdata;
    read_data <= wdata;
  end else
    read_data <= ram[address];

end

assign rdata = read_data;

`ifdef DISPLAY_MEMORY_INSTANCE_INFORMATION
initial begin 
  $display("");
  $display("****************************************************");
  $display("sram8_inferred_init ********************************");
  $display("unique_tag = sram8_inferred_init_W%0d_D%0d_C%0d", 8, 2**abits, (2**abits)*8);
  $display("****************************************************");
  $display("full path     =  %m");
  $display("abits         =  %d",abits);
  $display("Summary ********************************************");
  $display("Width         =  %d",8);
  $display("Depth         =  %d",2**abits);
  $display("Capacity      =  %d bits",(2**abits)*8);  
  $display("****************************************************");
  $display("");
end
`endif

endmodule
