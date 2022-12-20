//!
//! Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may ~use this file except in compliance with the License.
//! You may obtain a copy of the License at
//!
//!     http://www.apache.org/licenses/LICENSE-2.0
//!
//! Unless required by applicable law or agreed to in writing, software
//! distributed under the License is distributed on an "AS IS" BASIS,
//! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//! See the License for the specific language governing permissions and
//! limitations under the License.
//!

module clint #(
    parameter bit async_reset = 0,
    parameter integer cpu_total = 4
)
(
    input clk,
    input nrst,
    input types_amba_pkg::mapinfo_type i_mapinfo,
    output types_amba_pkg::dev_config_type o_cfg,
    input types_amba_pkg::axi4_slave_in_type i_axi,
    output types_amba_pkg::axi4_slave_out_type o_axi,
    output logic [63:0] o_mtimer,       // shadow read-only access from Harts
    output logic [cpu_total-1:0] o_msip,
    output logic [cpu_total-1:0] o_mtip
);


  
import types_amba_pkg::*;

typedef struct {
    logic [cpu_total-1:0] msip;
    logic [cpu_total-1:0] mtip;
    logic [63:0] mtime;
    logic [63:0] mtimecmp[0:cpu_total-1];
    logic [63:0] rdata;
} clint_registers_type;


clint_registers_type r, rin;

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic [7:0] wb_req_size;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;


axi_slv #(
   .async_reset(async_reset),
   .vid(VENDOR_OPTIMITECH),
   .did(OPTIMITECH_CLINT)
) axi0 (
    .i_clk(clk),
    .i_nrst(nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(i_axi),
    .o_xslvo(o_axi),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_size(wb_req_size),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(1'b1),
    .i_resp_valid(1'b1),
    .i_resp_rdata(r.rdata),
    .i_resp_err(1'd0)
);

always_comb begin : main_proc
  
    clint_registers_type v;
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;
    int regidx;

    v = r;

    vrdata = 0;
    v.mtip = '0;
    v.mtime = r.mtime + 1;
    regidx = int'(wb_req_addr[13 : 3]);
    
    for (int n = 0; n <= cpu_total-1; n++) begin
        if (r.mtime >= r.mtimecmp[n]) begin 
            v.mtip[n] = 1'b1;
        end
    end

    case (wb_req_addr[15 : 14])
    2'b00: begin
        vrdata[0] = r.msip[regidx];
        vrdata[32] = r.msip[regidx + 1];
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            if ((|wb_req_wstrb[3:0]) == 1'b1) begin
                v.msip[regidx] = wb_req_wdata[0];
            end
            if ((|wb_req_wstrb[7:4]) == 1'b1) begin
                v.msip[regidx + 1] = wb_req_wdata[32];
            end
        end
    end
    2'b01: begin
        vrdata = r.mtimecmp[regidx];
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            v.mtimecmp[regidx] = wb_req_wdata;
        end
    end
    2'b10: begin
        if (wb_req_addr[13:3] == 11'h7ff) begin
            vrdata = r.mtime;  // [RO]
        end
    end
    default:;
    endcase
    v.rdata = vrdata;


    if (~async_reset & (nrst == 1'b0)) begin
        v.msip = '0;
        v.mtip = '0;
        v.mtime = '0;
        for (int i = 0; i < cpu_total; i++) begin
            v.mtimecmp[i] = '0;
        end
    end
    rin = v;

    o_msip = r.msip;
    o_mtip = r.mtip;
    o_mtimer = r.mtime;
  end : main_proc

 
  generate 

   if (async_reset) begin: gen_async_reset
    
      always_ff@(posedge clk, negedge nrst) begin
          if (!nrst) begin
              r.msip <= '0;
              r.mtip <= '0;
              r.mtime <= '0;
              for (int i = 0; i < cpu_total; i++) begin
                  r.mtimecmp[i] <= '0;
              end
          end else begin
              r <= rin;
          end
      end
          
   end else begin: gen_sync_reset

      always_ff@(posedge clk) begin
          r <= rin;  
      end

   end

  endgenerate 

endmodule

