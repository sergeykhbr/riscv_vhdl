//!
//! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may not use this file except in compliance with the License.
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

module axictrl_bus0
#(parameter async_reset = 1)
(
  input                                         i_clk,
  input                                         i_nrst,
  input     types_bus0_pkg::bus0_xslv_cfg_vector      i_slvcfg,
  (* mark_debug = "true" *)
  input     types_bus0_pkg::bus0_xslv_out_vector      i_slvo,
  (* mark_debug = "true" *)
  input     types_bus0_pkg::bus0_xmst_out_vector      i_msto,
  (* mark_debug = "true" *)
  output    types_bus0_pkg::bus0_xslv_in_vector       o_slvi,
  (* mark_debug = "true" *)
  output    types_bus0_pkg::bus0_xmst_in_vector       o_msti,
  output logic [types_bus0_pkg::CFG_BUS0_XMST_TOTAL-1:0]    o_bus_util_w,
  output logic [types_bus0_pkg::CFG_BUS0_XMST_TOTAL-1:0]    o_bus_util_r
);

import types_bus0_pkg::*;
import types_amba_pkg::*;

typedef axi4_master_out_type nasti_master_out_vector_miss [0:CFG_BUS0_XMST_TOTAL];

typedef axi4_master_in_type nasti_master_in_vector_miss [0:CFG_BUS0_XMST_TOTAL];

typedef axi4_slave_out_type nasti_slave_out_vector_miss [0:CFG_BUS0_XSLV_TOTAL];

typedef axi4_slave_in_type nasti_slave_in_vector_miss [0:CFG_BUS0_XSLV_TOTAL];

typedef struct {
    logic [$clog2(CFG_BUS0_XMST_TOTAL)-1:0] r_midx;
    logic [$clog2(CFG_BUS0_XSLV_TOTAL+1)-1:0] r_sidx;
    logic [$clog2(CFG_BUS0_XMST_TOTAL)-1:0] w_midx;
    logic [$clog2(CFG_BUS0_XSLV_TOTAL+1)-1:0] w_sidx;
    logic [$clog2(CFG_BUS0_XMST_TOTAL)-1:0] b_midx;
    logic [$clog2(CFG_BUS0_XSLV_TOTAL+1)-1:0] b_sidx;
} reg_type;

const reg_type R_RESET = '{
    CFG_BUS0_XMST_TOTAL, CFG_BUS0_XSLV_TOTAL,
    CFG_BUS0_XMST_TOTAL, CFG_BUS0_XSLV_TOTAL,
    CFG_BUS0_XMST_TOTAL, CFG_BUS0_XSLV_TOTAL
};

reg_type rin, r;

axi4_slave_in_type defslv_i;
axi4_slave_out_type defslv_o;

  axi_defslv #(
    .async_reset(async_reset)
  ) xdef0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_xslvi(defslv_i),
    .o_xslvo(defslv_o)
  );


always_comb begin: comblogic

    reg_type                                v;

    logic [$clog2(CFG_BUS0_XMST_TOTAL)-1:0] ar_midx;
    logic [$clog2(CFG_BUS0_XMST_TOTAL)-1:0] aw_midx;
    logic [$clog2(CFG_BUS0_XSLV_TOTAL+1)-1:0] ar_sidx; // +1 miss access
    logic [$clog2(CFG_BUS0_XSLV_TOTAL+1)-1:0] aw_sidx; // +1 miss access

    nasti_master_out_vector_miss            vmsto;
    nasti_master_in_vector_miss             vmsti;
    nasti_slave_in_vector_miss              vslvi;
    nasti_slave_out_vector_miss             vslvo;

    logic                                   aw_fire;
    logic                                   ar_fire;
    logic                                   w_fire;
    logic                                   w_busy;
    logic                                   r_fire;
    logic                                   r_busy;
    logic                                   b_fire;
    logic                                   b_busy;

    // Bus statistic signals
    logic [CFG_BUS0_XMST_TOTAL-1:0]         wb_bus_util_w;
    logic [CFG_BUS0_XMST_TOTAL-1:0]         wb_bus_util_r;

    v = r;

    for(int m = 0; m <= CFG_BUS0_XMST_TOTAL-1; m++) begin
       vmsto[m] = i_msto[m];
       vmsti[m] = axi4_master_in_none;
    end

    vmsto[CFG_BUS0_XMST_TOTAL] = axi4_master_out_none;
    vmsti[CFG_BUS0_XMST_TOTAL] = axi4_master_in_none;

    for(int s=0; s <= CFG_BUS0_XSLV_TOTAL-1; s++) begin
       vslvo[s] = i_slvo[s];
       vslvi[s] = axi4_slave_in_none;
    end
    vslvo[CFG_BUS0_XSLV_TOTAL] = defslv_o;
    vslvi[CFG_BUS0_XSLV_TOTAL] = axi4_slave_in_none;

    ar_midx = CFG_BUS0_XMST_TOTAL;
    aw_midx = CFG_BUS0_XMST_TOTAL;
    ar_sidx = CFG_BUS0_XSLV_TOTAL;
    aw_sidx = CFG_BUS0_XSLV_TOTAL;

    // select master bus:
    for(int m = 0; m <= CFG_BUS0_XMST_TOTAL-1; m++) begin
        if(i_msto[m].ar_valid == 1'b1)
            ar_midx = m;
        if(i_msto[m].aw_valid == 1'b1)
            aw_midx = m;
    end

    // select slave interface
    for(int s = 0; s <= CFG_BUS0_XSLV_TOTAL-1; s++) begin
        if(i_slvcfg[s].xmask != 'h00000 &&
           (vmsto[ar_midx].ar_bits.addr[CFG_SYSBUS_ADDR_BITS-1:12]
            & i_slvcfg[s].xmask) == i_slvcfg[s].xaddr)
            ar_sidx = s;

        if(i_slvcfg[s].xmask != 'h00000 &&
           (vmsto[aw_midx].aw_bits.addr[CFG_SYSBUS_ADDR_BITS-1:12]
            & i_slvcfg[s].xmask) == i_slvcfg[s].xaddr)
            aw_sidx = s;
    end

    // Read Channel:
    ar_fire = vmsto[ar_midx].ar_valid & vslvo[ar_sidx].ar_ready;
    r_fire = vmsto[r.r_midx].r_ready & vslvo[r.r_sidx].r_valid & vslvo[r.r_sidx].r_last;
    // Write channel:
    aw_fire = vmsto[aw_midx].aw_valid & vslvo[aw_sidx].aw_ready;
    w_fire = vmsto[r.w_midx].w_valid & vmsto[r.w_midx].w_last & vslvo[r.w_sidx].w_ready;
    // Write confirm channel
    b_fire = vmsto[r.b_midx].b_ready & vslvo[r.b_sidx].b_valid;

    r_busy = 1'b0;
    if(r.r_sidx != CFG_BUS0_XSLV_TOTAL && r_fire == 1'b0)
        r_busy = 1'b1;

    w_busy = 1'b0;
    if((r.w_sidx != CFG_BUS0_XSLV_TOTAL && w_fire == 1'b0)
       || (r.b_sidx != CFG_BUS0_XSLV_TOTAL && b_fire == 1'b0))
        w_busy = 1'b1;

    b_busy = 1'b0;
    if(r.b_sidx != CFG_BUS0_XSLV_TOTAL && b_fire == 1'b0)
        b_busy = 1'b1;

    if(ar_fire == 1'b1 && r_busy == 1'b0) begin
        v.r_sidx = ar_sidx;
        v.r_midx = ar_midx;
    end
    else
    if(r_fire == 1'b1) begin
        v.r_sidx = CFG_BUS0_XSLV_TOTAL;
        v.r_midx = CFG_BUS0_XMST_TOTAL;
    end

    if(aw_fire == 1'b1 && w_busy == 1'b0) begin
        v.w_sidx = aw_sidx;
        v.w_midx = aw_midx;
    end
    else
    if(w_fire == 1'b1 && b_busy == 1'b0) begin
        v.w_sidx = CFG_BUS0_XSLV_TOTAL;
        v.w_midx = CFG_BUS0_XMST_TOTAL;
    end

    if(w_fire == 1'b1 && b_busy == 1'b0) begin
        v.b_sidx = r.w_sidx;
        v.b_midx = r.w_midx;
    end
    else
    if(b_fire == 1'b1) begin
        v.b_sidx = CFG_BUS0_XSLV_TOTAL;
        v.b_midx = CFG_BUS0_XMST_TOTAL;
    end


    vmsti[ar_midx].ar_ready = vslvo[ar_sidx].ar_ready & ~r_busy;
    vslvi[ar_sidx].ar_valid = vmsto[ar_midx].ar_valid & ~r_busy;
    vslvi[ar_sidx].ar_bits  = vmsto[ar_midx].ar_bits;
    vslvi[ar_sidx].ar_id    = vmsto[ar_midx].ar_id;
    vslvi[ar_sidx].ar_user  = vmsto[ar_midx].ar_user;

    vmsti[r.r_midx].r_valid = vslvo[r.r_sidx].r_valid;
    vmsti[r.r_midx].r_resp  = vslvo[r.r_sidx].r_resp;
    vmsti[r.r_midx].r_data  = vslvo[r.r_sidx].r_data;
    vmsti[r.r_midx].r_last  = vslvo[r.r_sidx].r_last;
    vmsti[r.r_midx].r_id    = vslvo[r.r_sidx].r_id;
    vmsti[r.r_midx].r_user  = vslvo[r.r_sidx].r_user;
    vslvi[r.r_sidx].r_ready = vmsto[r.r_midx].r_ready;

    vmsti[aw_midx].aw_ready = vslvo[aw_sidx].aw_ready & ~w_busy;
    vslvi[aw_sidx].aw_valid = vmsto[aw_midx].aw_valid & ~w_busy;
    vslvi[aw_sidx].aw_bits  = vmsto[aw_midx].aw_bits;
    vslvi[aw_sidx].aw_id    = vmsto[aw_midx].aw_id;
    vslvi[aw_sidx].aw_user  = vmsto[aw_midx].aw_user;

    vmsti[r.w_midx].w_ready = vslvo[r.w_sidx].w_ready & ~b_busy;
    vslvi[r.w_sidx].w_valid = vmsto[r.w_midx].w_valid & ~b_busy;
    vslvi[r.w_sidx].w_data = vmsto[r.w_midx].w_data;
    vslvi[r.w_sidx].w_last = vmsto[r.w_midx].w_last;
    vslvi[r.w_sidx].w_strb = vmsto[r.w_midx].w_strb;
    vslvi[r.w_sidx].w_user = vmsto[r.w_midx].w_user;

    vmsti[r.b_midx].b_valid = vslvo[r.b_sidx].b_valid;
    vmsti[r.b_midx].b_resp = vslvo[r.b_sidx].b_resp;
    vmsti[r.b_midx].b_id = vslvo[r.b_sidx].b_id;
    vmsti[r.b_midx].b_user = vslvo[r.b_sidx].b_user;
    vslvi[r.b_sidx].b_ready = vmsto[r.b_midx].b_ready;

    // Statistic
    wb_bus_util_w = '0;
    wb_bus_util_w[r.w_midx] = 1'b1;
    wb_bus_util_r = '0;
    wb_bus_util_r[r.r_midx] = 1'b1;


    if(!async_reset &&i_nrst == 1'b0)
        v = R_RESET;

    rin = v;

    for(int m = 0; m <= CFG_BUS0_XMST_TOTAL-1; m++)
       o_msti[m] = vmsti[m];

    for(int s = 0; s <= CFG_BUS0_XSLV_TOTAL-1; s++)
       o_slvi[s] = vslvi[s];

    defslv_i = vslvi[CFG_BUS0_XSLV_TOTAL];

    o_bus_util_w = wb_bus_util_w[CFG_BUS0_XMST_TOTAL-1:0];
    o_bus_util_r = wb_bus_util_r[CFG_BUS0_XMST_TOTAL-1:0];

end

// assign o_bus_util_w = wb_bus_util_w[CFG_BUS0_XMST_TOTAL-1:0];
// assign o_bus_util_r = wb_bus_util_r[CFG_BUS0_XMST_TOTAL-1:0];

// registers
generate

  if(async_reset) begin: gen_async_reset

      always_ff@(posedge i_clk, negedge i_nrst)
          if(!i_nrst)
              r <= R_RESET;
          else
              r <= rin;

  end
  else begin: gen_sync_reset

      always_ff@(posedge i_clk)
              r <= rin;

  end

endgenerate

endmodule
