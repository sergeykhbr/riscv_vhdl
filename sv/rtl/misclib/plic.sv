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
    parameter integer ctxmax = 8,
    parameter integer irqmax = 128
)
(
    input clk,
    input nrst,
    input types_amba_pkg::mapinfo_type i_mapinfo,
    output types_amba_pkg::dev_config_type o_cfg,
    input types_amba_pkg::axi4_slave_in_type i_axi,
    output types_amba_pkg::axi4_slave_out_type o_axi,
    input [irqmax-1:0] i_irq_request,  // [0] must be tight to GND
    output logic [ctxmax-1:0] o_ip
);


  
import types_amba_pkg::*;

typedef struct {
    logic [3:0] priority_th;
    logic [1023:0] ie;       // interrupt enable per contet
    logic [4*1024-1:0] ip_prio;   // interrupt pending priority per context
    logic [15:0] prio_mask;       // pending interrupts priorites
    logic [3:0] sel_prio;         // the most available priority
    logic [9:0] irq_idx;     // currently selected most prio irq
    logic [9:0] irq_prio;    // currently selected prio level
} plic_context_type;

typedef struct {
    logic [4*1024-1:0] src_priority;       // Prioirty 4 bits per irq
    logic [1023:0] pending;
    logic [ctxmax-1:0] ip;
    plic_context_type ctx[0:ctxmax-1];
    logic [63:0] rdata;
} plic_registers_type;


plic_registers_type r, rin;

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic [7:0] wb_req_size;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;

logic [4*1024-1:0] wb_src_priority;
logic [1023:0] wb_pending;
logic [ctxmax-1:0] wb_ip;
plic_context_type wb_ctx;

axi_slv #(
   .async_reset(async_reset),
   .vid(VENDOR_OPTIMITECH),
   .did(OPTIMITECH_PLIC)
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
  
    plic_registers_type v;
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;
    logic [9:0] vb_irq_idx[ctxmax-1:0];   // currently selected most prio irq
    logic [9:0] vb_irq_prio[ctxmax-1:0];  // currently selected prio level
    int rctx_idx;

    v.src_priority = r.src_priority;
    v.pending = r.pending;
    v.ip = '0;
    for (int n = 0; n <= ctxmax-1; n++) begin
        v.ctx[n].priority_th = r.ctx[n].priority_th;
        v.ctx[n].ie = r.ctx[n].ie;
        v.ctx[n].ip_prio = '0;
        v.ctx[n].prio_mask = '0;
        v.ctx[n].sel_prio = '0;
        v.ctx[n].irq_idx = r.ctx[n].irq_idx;
        v.ctx[n].irq_prio = r.ctx[n].irq_prio;

        vb_irq_idx[n] = '0;
        vb_irq_prio[n] = '0;
    end
    vrdata = 0;

  
    for (int i = 1; i <= irqmax; i++) begin
        if (i_irq_request[i] && r.src_priority[4*i +: 4] > 4'd0) begin
            v.pending[i] = 1'b1;
        end
    end

    for (int n = 0; n <= ctxmax-1; n++) begin
        for (int i = 0; i <= irqmax-1; i++) begin
            if (r.pending[i] && r.ctx[n].ie[i]
                && r.src_priority[4*i +: 4] > r.ctx[n].priority_th) begin
                 v.ctx[n].ip_prio[4*i +: 4] = r.src_priority[4*i +: 4];
                 v.ctx[n].prio_mask[int'(r.src_priority[4*i +: 4])] = 1'b1;
            end
        end
    end

    // Select max priority in each context
    for (int n = 0; n <= ctxmax-1; n++) begin
        for (int i = 0; i < 16; i++) begin
            if (r.ctx[n].prio_mask[i] == 1'b1) begin
                v.ctx[n].sel_prio = i;
            end
        end
    end

    for (int n = 0; n <= ctxmax-1; n++) begin
        for (int i = 0; i <= irqmax-1; i++) begin
            if ((|r.ctx[n].sel_prio) && r.ctx[n].ip_prio[4*i +: 4] == r.ctx[n].sel_prio) begin
                 // Most prio irq and prio level
                 vb_irq_idx[n] = i;
                 vb_irq_prio[n] = r.ctx[n].sel_prio;
            end
        end
    end

    for (int n = 0; n <= ctxmax-1; n++) begin
        v.ctx[n].irq_idx = vb_irq_idx[n];
        v.ctx[n].irq_prio = vb_irq_prio[n];
        v.ip[n] = (|vb_irq_idx[n]);
    end


    rctx_idx = int'(wb_req_addr[20:12]);
    if (wb_req_addr[21:12] == 10'h000) begin  // src_prioirty
        // 0x000000..0x001000: Irq 0 unused
        if (((|wb_req_addr[11 : 3])) == 1'b1) begin
            vrdata[3:0] = r.src_priority[8*int'(wb_req_addr[11 : 3]) +: 4];
            if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
                if ((|wb_req_wstrb[3:0]) == 1'b1) begin
                    v.src_priority[8*int'(wb_req_addr[11 : 3]) +: 4] = wb_req_wdata[3:0];
                end
            end
        end

        vrdata[35:32] = r.src_priority[8*(int'(wb_req_addr[11 : 3]) + 32) +: 4];
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            if ((|wb_req_wstrb[7:4]) == 1'b1) begin
                v.src_priority[8*int'(wb_req_addr[11 : 3]) + 32 +: 4] = wb_req_wdata[35:32];
            end
        end
    end else if (wb_req_addr[21:12] == 10'h001) begin
        // 0x001000..0x001080
        vrdata = r.pending[64*int'(wb_req_addr[6:3]) +: 64];
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            if ((|wb_req_wstrb[3:0]) == 1'b1) begin
                v.pending[64*int'(wb_req_addr[6 : 3]) +: 32] = wb_req_wdata[31:0];
            end
            if ((|wb_req_wstrb[7:4]) == 1'b1) begin
                v.pending[64*int'(wb_req_addr[6 : 3]) + 32 +: 32] = wb_req_wdata[63:32];
            end
        end
    end else if (wb_req_addr[21:12] == 10'h002
                 && wb_req_addr[11:7] < ctxmax) begin
        // First 32 context of 15867 support only
        // 0x002000,0x002080,...,0x200000
        vrdata = r.ctx[int'(wb_req_addr[11:7])].ie[64*int'(wb_req_addr[6:3]) +: 64];
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            if ((|wb_req_wstrb[3:0]) == 1'b1) begin
                v.ctx[int'(wb_req_addr[11:7])].ie[64*int'(wb_req_addr[6:3]) +: 32] = wb_req_wdata[31:0];
            end
            if ((|wb_req_wstrb[7:4]) == 1'b1) begin
                v.ctx[int'(wb_req_addr[11:7])].ie[64*int'(wb_req_addr[6:3]) + 32 +: 32] = wb_req_wdata[63:32];
            end
        end
    end else if (wb_req_addr[21:12] >= 10'h200 && wb_req_addr[20:12] < ctxmax) begin
        // 0x200000,0x201000,...,0x4000000
        if (wb_req_addr[11:3] == 9'h000) begin
            // masking (disabling) all interrupt with <= priority
            vrdata[3:0] = r.ctx[rctx_idx].priority_th;
            vrdata[41:32] = r.ctx[rctx_idx].irq_idx;
            // claim/ complete. Reading clears pending bit
            if (r.ip[rctx_idx] == 1'b1) begin
                v.pending[r.ctx[rctx_idx].irq_idx] = 1'b0;
            end

            if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
                if ((|wb_req_wstrb[3:0]) == 1'b1) begin
                    v.ctx[rctx_idx].priority_th = wb_req_wdata[3:0];
                end
                if ((|wb_req_wstrb[7:4]) == 1'b1) begin
                     // claim/ complete. Reading clears pedning bit
                    v.ctx[rctx_idx].irq_idx = 0;
                end
            end
        end else begin
              // reserved
        end
    end
    v.rdata = vrdata;



    if (~async_reset & (nrst == 1'b0)) begin
        v.src_priority = '0;
        v.pending = '0;
        v.ip = '0;
        for (int n = 0; n <= ctxmax-1; n++) begin
            v.ctx[n].priority_th[n] = '0;
            v.ctx[n].ie = '0;
            v.ctx[n].ip_prio = '0;
            v.ctx[n].prio_mask = '0;
            v.ctx[n].sel_prio = '0;
            v.ctx[n].irq_idx = '0;
            v.ctx[n].irq_prio = '0;
        end
    end

    o_ip = r.ip;

    rin = v;
  end : main_proc

  // To debug registers values use the following signals.
  assign wb_src_priority = r.src_priority;
  assign wb_pending = r.pending;
  assign wb_ip = r.ip;
  assign wb_ctx = r.ctx[0];

  generate 

   if (async_reset) begin: gen_async_reset
    
      always_ff@(posedge clk, negedge nrst) begin
          if (!nrst) begin
              r.src_priority <= '0;
              r.ip <= '0;
              r.pending <= '0;
              for (int n = 0; n <= ctxmax-1; n++) begin
                  r.ctx[n].priority_th <= '0;
                  r.ctx[n].ie <= '0;
                  r.ctx[n].ip_prio <= '0;
                  r.ctx[n].prio_mask <= '0;
                  r.ctx[n].sel_prio <= '0;
                  r.ctx[n].irq_idx <= '0;
                  r.ctx[n].irq_prio <= '0;
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

