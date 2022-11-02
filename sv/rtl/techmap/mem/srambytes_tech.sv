//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Internal SRAM implementation with the byte access.
//----------------------------------------------------------------------------

module srambytes_tech #(
    parameter integer abits    = 16,
    parameter init_file = ""
)
(
    input clk,
    input types_amba_pkg::global_addr_array_type raddr,
    output logic [types_amba_pkg::CFG_SYSBUS_DATA_BITS-1 : 0] rdata,
    input types_amba_pkg::global_addr_array_type waddr,
    input we,
    input [types_amba_pkg::CFG_SYSBUS_DATA_BYTES-1 : 0] wstrb,
    input [types_amba_pkg::CFG_SYSBUS_DATA_BITS-1 : 0] wdata
);

import types_amba_pkg::*;
import config_target_pkg::*;

//! reduced name of configuration constant:

localparam integer dw = CFG_SYSBUS_ADDR_OFFSET;

typedef logic [abits-dw-1 : 0] local_addr_type [0 : CFG_SYSBUS_DATA_BYTES-1];

local_addr_type address;

logic [CFG_SYSBUS_DATA_BYTES-1 : 0] wr_ena;

//! Instantiate component for RTL simulation

`ifdef TARGET_INFERRED

generate
  
   for (genvar n = 0; n <= CFG_SYSBUS_DATA_BYTES-1; n++) begin : rx

      assign wr_ena[n] = we & wstrb[n];
      assign address[n] = (we == 1) ? waddr[n / CFG_ALIGN_BYTES][abits-1 : dw] :
                          raddr[n / CFG_ALIGN_BYTES][abits-1 : dw];
                  
      sram8_inferred_init #(
          .abits(abits-dw),
          .byte_idx(n),
          .init_file(init_file)
      ) x0 (
          .clk(clk), 
          .address(address[n]),
          .rdata(rdata[8*n+:8]),
          .we(wr_ena[n]),
          .wdata(wdata[8*n+:8])
      );
      
   end : rx

endgenerate

`elsif TARGET_KC705

   for (genvar n = 0; n <= CFG_SYSBUS_DATA_BYTES-1; n++) begin : rx

      assign wr_ena[n] = we & wstrb[n];
      assign address[n] = (we == 1) ? waddr[n / CFG_ALIGN_BYTES][abits-1 : dw] :
                          raddr[n / CFG_ALIGN_BYTES][abits-1 : dw];

      sram8_inferred_init #(
        .abits(abits-dw),
        .byte_idx(n),
        .init_file(init_file)
      ) x0 (
        .clk(clk),
        .address(address[n]),
        .rdata(rdata[8*n+:8]),
        .we(wr_ena[n]),
        .wdata(wdata[8*n+:8])
      );
      
   end : rx

`else

generate

   for (genvar n = 0; n <= CFG_SYSBUS_DATA_BYTES-1; n++) begin : rx

      assign wr_ena[n] = we & wstrb[n];
      assign address[n] = (we == 1) ? waddr[n / CFG_ALIGN_BYTES][abits-1 : dw] :
                          raddr[n / CFG_ALIGN_BYTES][abits-1 : dw];
                  
      sram8_inferred #(
          .abits(abits-dw),
          .byte_idx(n)
      ) x0 (
          .clk(clk), 
          .address(address[n]),
          .rdata(rdata[8*n+:8]),
          .we(wr_ena[n]),
          .wdata(wdata[8*n+:8])
      );
      
   end : rx
   
endgenerate

`endif

endmodule

