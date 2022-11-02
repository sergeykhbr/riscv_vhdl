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
    parameter longint xaddr = 0,
    parameter longint xmask = 'hffff0,
    parameter integer cpu_total = 4
)
(
    input clk,
    input nrst,
    output types_amba_pkg::axi4_slave_config_type o_cfg,
    input types_amba_pkg::axi4_slave_in_type i_axi,
    output types_amba_pkg::axi4_slave_out_type o_axi,
    output logic [63:0] o_mtimer,       // shadow read-only access from Harts
    output logic [cpu_total-1:0] o_msip,
    output logic [cpu_total-1:0] o_mtip
);


  
  import types_amba_pkg::*;

  const axi4_slave_config_type xconfig = '{
     PNP_CFG_TYPE_SLAVE, // descrtype
     PNP_CFG_SLAVE_DESCR_BYTES, // descrsize
     xaddr, // xaddr
     xmask, // xmask
     VENDOR_OPTIMITECH, // vid
     OPTIMITECH_CLINT // did
  };


  typedef struct {
    global_addr_array_type raddr;
    logic [cpu_total-1:0] msip;
    logic [cpu_total-1:0] mtip;
    logic [63:0] mtime;
    logic [63:0] mtimecmp[0:cpu_total-1];
  } clint_registers_type;


  clint_registers_type r, rin;

  logic [CFG_SYSBUS_DATA_BITS-1 : 0] wb_dev_rdata;
  global_addr_array_type wb_bus_raddr;
  logic w_bus_re;
  global_addr_array_type wb_bus_waddr;
  logic w_bus_we;
  logic [CFG_SYSBUS_DATA_BYTES-1 : 0] wb_bus_wstrb;
  logic [CFG_SYSBUS_DATA_BITS-1 : 0] wb_bus_wdata;


  axi_slv #(
    .async_reset(async_reset)
  ) axi0 (
    .i_clk(clk),
    .i_nrst(nrst),
    .i_xcfg(xconfig), 
    .i_xslvi(i_axi),
    .o_xslvo(o_axi),
    .i_ready(1'b1),
    .i_rdata(wb_dev_rdata),
    .o_re(w_bus_re),
    .o_r32(),
    .o_radr(wb_bus_raddr),
    .o_wadr(wb_bus_waddr),
    .o_we(w_bus_we),
    .o_wstrb(wb_bus_wstrb),
    .o_wdata(wb_bus_wdata)
  );

  always_comb begin : main_proc
  
    clint_registers_type v;
    logic [13:0] raddr[0:CFG_WORDS_ON_BUS-1];
    logic [9:0] waddr[0:CFG_WORDS_ON_BUS-1];
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;
    logic [31 : 0] tmp;

    v = r;

    v.mtip = '0;
    v.raddr = wb_bus_raddr;
    v.mtime = r.mtime + 1;
    
    for (int n = 0; n <= cpu_total-1; n++) begin
        if (r.mtime >= r.mtimecmp[n]) begin 
            v.mtip[n] = 1'b1;
        end
    end


    for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
       tmp = '0;
       waddr[n] = '0;                     // to avoid latches
       raddr[n] = r.raddr[n][13 : 0];
       case (r.raddr[n][15 : 14])
           2'b00:
               tmp[0] = r.msip[int'(r.raddr[n][13 : 2])];
           2'b01:
               if (raddr[n][2] == 1'b0) begin
                   tmp = r.mtimecmp[int'(r.raddr[n][13 : 3])][31:0];
               end else begin
                   tmp = r.mtimecmp[int'(r.raddr[n][13 : 3])][63:32];
               end
           2'b10: 
               if (raddr[n] == 14'h3ff8) begin
                   tmp = r.mtime[31:0];
               end else if (raddr[n] == 14'h3ffc) begin
                   tmp = r.mtime[63:32];
               end
           default:;
       endcase
       vrdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES] = tmp;
    end


    if (w_bus_we == 1'b1) begin
      for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
         if (wb_bus_wstrb[CFG_ALIGN_BYTES*n +: CFG_ALIGN_BYTES] != 0) begin
             tmp = wb_bus_wdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES];
             waddr[n] = wb_bus_waddr[n][13 : 0];
             case (int'(wb_bus_waddr[n][15:14]))
             2'b00:
                 v.msip[int'(r.raddr[n][13 : 2])] = tmp[0];
             2'b01:
                 if (raddr[n][2] == 1'b0) begin
                     v.mtimecmp[int'(r.raddr[n][13 : 3])][31:0] = tmp;
                 end else begin
                     v.mtimecmp[int'(r.raddr[n][13 : 3])][63:32] = tmp;
                 end
             // Cannot write into mtime
//             2'b10:
//               if (waddr[n] == 14'h3ff8) begin
//                   v.mtime[31:0] = tmp;
//               end else if (waddr[n] == 14'h3ffc) begin
//                   v.mtime[63:32] = tmp;
//               end
             default:;
           endcase
         end
      end
    end

    if (~async_reset & (nrst == 1'b0)) begin
        v.msip = '0;
        v.mtip = '0;
        v.mtime = '0;
        for (int i = 0; i < cpu_total; i++) begin
            v.mtimecmp[i] = '0;
        end
    end
    rin = v;

    wb_dev_rdata = vrdata;

    o_msip = r.msip;
    o_mtip = r.mtip;
    o_mtimer = r.mtime;
  end : main_proc

  assign o_cfg = xconfig;

  // registers
  
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

