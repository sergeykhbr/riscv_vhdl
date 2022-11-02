//!
//! Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
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

module plic #(
    parameter bit async_reset = 0,
    parameter longint xaddr = 0,
    parameter longint xmask = 'hffc00,
    parameter integer ctxmax = 8,
    parameter integer irqmax = 128
)
(
    input clk,
    input nrst,
    output types_amba_pkg::axi4_slave_config_type o_cfg,
    input types_amba_pkg::axi4_slave_in_type i_axi,
    output types_amba_pkg::axi4_slave_out_type o_axi,
    input [irqmax-1:0] i_irq_request,  // [0] must be tight to GND
    output logic [ctxmax-1:0] o_ip
);


  
  import types_amba_pkg::*;


  const axi4_slave_config_type xconfig = '{
     PNP_CFG_TYPE_SLAVE, // descrtype
     PNP_CFG_SLAVE_DESCR_BYTES, // descrsize
     xaddr, // xaddr
     xmask, // xmask
     VENDOR_OPTIMITECH, // vid
     OPTIMITECH_PLIC // did
  };

  typedef struct {
    logic [3:0] priority_th;
    logic [1023:0] ie;       // interrupt enable per contet
    logic [9:0] irq_idx;     // currently selected most prio irq
    logic [9:0] irq_prio;    // currently selected prio level
  } plic_context_type;

  typedef struct {
    global_addr_array_type raddr;
    logic [4*1024-1:0] src_priority;       // Prioirty 4 bits per irq
    logic [1023:0] pending;
    logic [ctxmax-1:0] ip;
    plic_context_type ctx[0:ctxmax-1];
  } plic_registers_type;


  plic_registers_type r, rin;

  logic [CFG_SYSBUS_DATA_BITS-1 : 0] wb_dev_rdata;
  global_addr_array_type wb_bus_raddr;
  logic w_bus_re;
  global_addr_array_type wb_bus_waddr;
  logic w_bus_we;
  logic [CFG_SYSBUS_DATA_BYTES-1 : 0] wb_bus_wstrb;
  logic [CFG_SYSBUS_DATA_BITS-1 : 0] wb_bus_wdata;

    logic [4*1024-1:0] wb_src_priority;
    logic [1023:0] wb_pending;
    logic [ctxmax-1:0] wb_ip;
    plic_context_type wb_ctx;

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
  
    plic_registers_type v;
    logic [9:0] raddr[0:CFG_WORDS_ON_BUS-1];
    logic [9:0] waddr[0:CFG_WORDS_ON_BUS-1];
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;
    logic [31 : 0] tmp;
    logic [9:0] vb_irq_idx[ctxmax-1:0];   // currently selected most prio irq
    logic [9:0] vb_irq_prio[ctxmax-1:0];  // currently selected prio level
    int rctx_idx[0:CFG_WORDS_ON_BUS-1];

    v.src_priority = r.src_priority;
    v.pending = r.pending;
    v.ip = '0;
    for (int n = 0; n <= ctxmax-1; n++) begin
        v.ctx[n].priority_th = r.ctx[n].priority_th;
        v.ctx[n].ie = r.ctx[n].ie;
        v.ctx[n].irq_idx = r.ctx[n].irq_idx;
        v.ctx[n].irq_prio = r.ctx[n].irq_prio;

        vb_irq_idx[n] = '0;
        vb_irq_prio[n] = '0;
    end

    v.raddr = wb_bus_raddr;
    
    for (int i = 1; i <= irqmax; i++) begin
        if (i_irq_request[i] && r.src_priority[4*i +: 4] > 4'd0) begin
            v.pending[i] = 1'b1;
        end
    end

    for (int i = 0; i <= irqmax-1; i++) begin
        for (int n = 0; n <= ctxmax-1; n++) begin
            if (r.pending[i] && r.ctx[n].ie[i]
                && r.src_priority[4*i +: 4] > r.ctx[n].priority_th
                && r.src_priority[4*i +: 4] > vb_irq_prio[n]) begin
                 // Most prio irq and prio level
                 vb_irq_idx[n] = i;
                 vb_irq_prio[n] = r.src_priority[4*i +: 4];
            end
        end
    end

    for (int n = 0; n <= ctxmax-1; n++) begin
        v.ctx[n].irq_idx = vb_irq_idx[n];
        v.ctx[n].irq_prio = vb_irq_prio[n];
        v.ip[n] = (|vb_irq_idx[n]);
    end


    for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
       tmp = '0;
       waddr[n] = '0;                     // to avoid latches
       raddr[n] = r.raddr[n][21:12];
       if (raddr[n] == 10'h000) begin  // src_prioirty
           // 0x000000..0x001000: Irq 0 unused
           if (((|r.raddr[n][11 : 2])) == 1'b1) begin
               tmp[3:0] = r.src_priority[4*int'(r.raddr[n][11 : 2]) +: 4];
           end
       end else if (raddr[n] == 10'h001) begin
           // 0x001000..0x001080
           tmp = r.pending[32*int'(r.raddr[n][6:2]) +: 32];
       end else if (raddr[n] == 10'h002
                 && r.raddr[n][11:7] < ctxmax) begin
           // First 32 context of 15867 support only
           // 0x002000,0x002080,...,0x200000
           tmp = r.ctx[int'(r.raddr[n][11:7])].ie[32*int'(r.raddr[n][6:2]) +: 32];
       end else if (raddr[n] >= 10'h200 && r.raddr[n][20:12] < ctxmax) begin
           // 0x200000,0x201000,...,0x4000000
           rctx_idx[n] = int'(r.raddr[n][20:12]);
           if (r.raddr[n][11:2] == 10'h000) begin
               // masking (disabling) all interrupt with <= priority
               tmp[3:0] = r.ctx[rctx_idx[n]].priority_th;
           end else if (r.raddr[n][11:2] == 10'h001) begin
               // claim/ complete. Reading clears pending bit
               tmp[9:0] = r.ctx[rctx_idx[n]].irq_idx;
               if (r.ip[rctx_idx[n]] == 1'b1) begin
                   v.pending[r.ctx[rctx_idx[n]].irq_idx] = 1'b0;
               end
           end else begin
              // reserved
           end
       end
       vrdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES] = tmp;
    end


    if (w_bus_we == 1'b1) begin
      for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
         if (wb_bus_wstrb[CFG_ALIGN_BYTES*n +: CFG_ALIGN_BYTES] != 0) begin
             tmp = wb_bus_wdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES];
             waddr[n] = wb_bus_waddr[n][21 : 12];
             if (waddr[n] == 10'h000) begin
                 v.src_priority[4*int'(wb_bus_waddr[n][11 : 2]) +: 4] = tmp[3:0];
                 v.src_priority[3:0] = 0;
             end else if (waddr[n] == 10'h001) begin
                 // 0x001000..0x001080
                 v.pending[32*int'(wb_bus_waddr[n][6:2]) +: 32] = tmp;
             end else if (waddr[n] == 10'h002
                       && wb_bus_waddr[n][11:7] < ctxmax) begin
                 // First 32 context of 15867 support only
                 // 0x002000,0x002080,...,0x200000
                 v.ctx[int'(wb_bus_waddr[n][11:7])].ie[32*int'(wb_bus_waddr[n][6:2]) +: 32] = tmp;
             end else if (waddr[n] >= 10'h200 && wb_bus_waddr[n][20:12] < ctxmax) begin
                 // 0x200000,0x201000,...,0x4000000
                 if (wb_bus_waddr[n][11:2] == 10'h000) begin
                     // masking (disabling) all interrupt with <= priority
                     v.ctx[int'(wb_bus_waddr[n][20:12])].priority_th = tmp[3:0];
                 end else if (wb_bus_waddr[n][11:2] == 10'h001) begin
                     // claim/ complete. Reading clears pedning bit
                     v.ctx[int'(wb_bus_waddr[n][20:12])].irq_idx = 0;//tmp[9:0];
                 end else begin
                    // reserved
                 end
             end
         end
      end
    end

    if (~async_reset & (nrst == 1'b0)) begin
        v.src_priority = '0;
        v.pending = '0;
        v.ip = '0;
        for (int n = 0; n <= ctxmax-1; n++) begin
            v.ctx[n].priority_th[n] = '0;
            v.ctx[n].ie = '0;
            v.ctx[n].irq_idx = '0;
            v.ctx[n].irq_prio = '0;
        end
    end
    rin = v;

    o_ip = r.ip;

    wb_dev_rdata = vrdata;
  end : main_proc

  // To debug registers values use the following signals.
  assign wb_src_priority = r.src_priority;
  assign wb_pending = r.pending;
  assign wb_ip = r.ip;
  assign wb_ctx = r.ctx[0];

  assign o_cfg = xconfig;

  // registers
  
  generate 

   if (async_reset) begin: gen_async_reset
    
      always_ff@(posedge clk, negedge nrst) begin
          if (!nrst) begin
              r.src_priority <= '0;
              r.ip <= '0;
              r.pending <= '0;
              for (int n = 0; n <= ctxmax-1; n++) begin
                  r.ctx_priority_th[n] <= '0;
                  r.ctx_ie[n] <= '0;
                  r.irq_idx[n] <= '0;
                  r.irq_prio[n] <= '0;
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

