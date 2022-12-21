//! @brief Hardware Configuration storage with the AMBA AXI4 interface.

module apb_pnp #(

    parameter async_reset = 1'b0,
    parameter int cfg_slots = 1,
    parameter logic [31 : 0] hw_id = 32'h20221123,
    parameter logic [3 : 0] cpu_max = 4'd1,
    parameter logic l2cache_ena = 8'd1,
    parameter logic [7 : 0] plic_irq_max = 8'd127

)(

  input sys_clk,
  input nrst,
  input types_amba_pkg::mapinfo_type i_mapinfo,
  input types_amba_pkg::dev_config_type i_cfg[0:cfg_slots-1],
  output types_amba_pkg::dev_config_type o_cfg,
  input types_amba_pkg::apb_in_type i,
  output types_amba_pkg::apb_out_type o,
  output logic o_irq                            // self-test interrupt request on write into read only register
);

  import types_amba_pkg::*;
  import types_bus1_pkg::*;

  // Descriptor size 8 x 32-bits word = 32 Bytes
  typedef logic [31 : 0] config_map  [0 : 8*cfg_slots-1];

  typedef struct {
    logic [31 : 0] fw_id;
    logic [63 : 0] idt; //! debug counter
    logic [63 : 0] malloc_addr; //! dynamic allocation addr
    logic [63 : 0] malloc_size; //! dynamic allocation size
    logic [63 : 0] fwdbg1; //! FW marker for the debug porposes
    logic [63 : 0] fwdbg2;
    logic [63 : 0] fwdbg3;
    logic irq;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
  } registers;

  const registers R_RESET = '{
    '0, '0, '0,
    '0, '0, '0, '0,
    1'b0,
    1'b0,// resp_valid
    '0,  // resp_data
    1'b0 // resp_err
  };

  registers r, rin;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;

apb_slv #(
   .async_reset(async_reset),
   .vid(VENDOR_OPTIMITECH),
   .did(OPTIMITECH_PNP)
) apb0 (
    .i_clk(sys_clk),
    .i_nrst(nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_apbi(i),
    .o_apbo(o),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .i_resp_valid(r.resp_valid),
    .i_resp_rdata(r.resp_rdata),
    .i_resp_err(r.resp_err)
);


always_comb
  begin : main_proc
    registers v;
    config_map cfgmap;
    logic [31 : 0] vrdata;

    v = r;

    v.irq = 1'b0;
    vrdata = '0;

    for (int k = 0; k <= cfg_slots-1; k++) begin
      cfgmap[8*k] = {8'h00, 8'h00, 6'b000000, i_cfg[k].descrtype, i_cfg[k].descrsize};
      cfgmap[8*k+1] = {i_cfg[k].vid, i_cfg[k].did};
      cfgmap[8*k+2] = '0; // reserved lsb
      cfgmap[8*k+3] = '0; // reserved msb
      cfgmap[8*k+4] = i_cfg[k].addr_start[31:0];
      cfgmap[8*k+5] = i_cfg[k].addr_start[63:32];
      cfgmap[8*k+6] = i_cfg[k].addr_end[31:0];
      cfgmap[8*k+7] = i_cfg[k].addr_end[63:32];
    end


    if (wb_req_addr[11:2] == 9'd0) begin
        vrdata = hw_id;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.irq = 1'b1;
        end
    end else if (wb_req_addr[11:2] == 9'd1) begin
        vrdata = r.fw_id;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fw_id = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd2) begin
        vrdata = {cpu_max[3:0],
                  3'd0,    // reserved
                  l2cache_ena,
                  8'h0,
                  cfg_slots[7:0],
                  plic_irq_max[7:0]};
    end else if (wb_req_addr[11:2] == 9'd3) begin
        vrdata = '0;
    end else if (wb_req_addr[11:2] == 9'd4) begin
        vrdata = r.idt[31:0];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.idt[31:0] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd5) begin
        vrdata = r.idt[63:32];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.idt[63:32] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd6) begin
        vrdata = r.malloc_addr[31:0];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_addr[31:0] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd7) begin
        vrdata = r.malloc_addr[63:32];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_addr[63:32] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd8) begin
        vrdata = r.malloc_size[31:0];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_size[31:0] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd9) begin
        vrdata = r.malloc_size[63:32];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_size[63:32] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd10) begin
        vrdata = r.fwdbg1[31:0];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg1[31:0] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd11) begin
        vrdata = r.fwdbg1[63:32];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg1[63:32] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd12) begin
        vrdata = r.fwdbg2[31:0];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg2[31:0] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd13) begin
        vrdata = r.fwdbg2[63:32];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg2[63:32] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd14) begin
        vrdata = r.fwdbg3[31:0];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg3[31:0] = wb_req_wdata;
        end
    end else if (wb_req_addr[11:2] == 9'd15) begin
        vrdata = r.fwdbg3[63:32];
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg3[63:32] = wb_req_wdata;
        end
    end else if ((int'(wb_req_addr[11:2]) >= 16) & (int'(wb_req_addr[11:2]) < 16+8*cfg_slots)) begin
        vrdata = cfgmap[int'(wb_req_addr[11:2]) - 16];
    end

    v.resp_valid = w_req_valid;
    v.resp_rdata = vrdata;
    v.resp_err = '0;

    if (~async_reset & (nrst == 1'b0)) begin
        v = R_RESET;
    end

    o_irq = r.irq;

    rin = v;
  end : main_proc

  generate

   if(async_reset) begin: gen_async_reset

      always_ff@(posedge sys_clk, negedge nrst)
          if(!nrst)
              r <= R_RESET;
          else
              r <= rin;

   end
   else begin: gen_sync_reset

      always_ff@(posedge sys_clk)
              r <= rin;

   end

  endgenerate

endmodule

