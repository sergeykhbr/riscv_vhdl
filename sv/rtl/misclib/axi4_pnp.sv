//! @brief Hardware Configuration storage with the AMBA AXI4 interface.

module axi4_pnp #(

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
  input types_amba_pkg::axi4_slave_in_type i,
  output types_amba_pkg::axi4_slave_out_type o,
  output logic o_irq                            // self-test interrupt request on write into read only register
);

  import types_amba_pkg::*;
  import types_bus0_pkg::*;
  import types_misc::*;
  import types_common::*;

  // Descriptor size 4 x 64-bits word = 32 Bytes
  typedef logic [63 : 0] config_map  [0 : 4*cfg_slots-1];

  typedef struct {
    logic [31 : 0] fw_id;
    logic [63 : 0] idt; //! debug counter
    logic [63 : 0] malloc_addr; //! dynamic allocation addr
    logic [63 : 0] malloc_size; //! dynamic allocation size
    logic [63 : 0] fwdbg1; //! FW marker for the debug porposes
    logic [63 : 0] fwdbg2;
    logic [63 : 0] fwdbg3;
    logic irq;
    logic [63 : 0] rdata;
  } registers;

  const registers R_RESET = '{
    '0, '0, '0,
    '0, '0, '0, '0,
    1'b0,
    '0
  };

  registers r, rin;

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;

axi_slv #(
   .async_reset(async_reset),
   .vid(VENDOR_OPTIMITECH),
   .did(OPTIMITECH_PNP)
) axi0 (
    .i_clk(sys_clk),
    .i_nrst(nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(i),
    .o_xslvo(o),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(1'b1),
    .i_resp_valid(1'b1),
    .i_resp_rdata(r.rdata),
    .i_resp_err(1'd0)
);


always_comb
  begin : main_proc
    registers v;
    config_map cfgmap;
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;

    v = r;

    v.irq = 1'b0;
    vrdata = '0;

    for (int k = 0; k <= cfg_slots-1; k++) begin
      cfgmap[4*k] = {i_cfg[k].vid, i_cfg[k].did,
                     8'h00, 8'h00, 6'b000000, i_cfg[k].descrtype, i_cfg[k].descrsize};
      cfgmap[4*k+1] = '0; // reserved
      cfgmap[4*k+2] = i_cfg[k].addr_start;
      cfgmap[4*k+3] = i_cfg[k].addr_end;
    end


    if (wb_req_addr[11:3] == 9'd0) begin
        vrdata = {r.fw_id, hw_id};
        if ((w_req_valid & w_req_write) == 1'b1) begin
            if ((|wb_req_wstrb[3:0]) == 1'b1) begin
                v.irq = 1'b1;
            end
            if ((|wb_req_wstrb[7:4]) == 1'b1) begin
                v.fw_id = wb_req_wdata[63:32];
            end
        end
    end else if (wb_req_addr[11:3] == 9'd1) begin
        vrdata = {32'd0,
                  cpu_max[3:0],
                  3'd0,    // reserved
                  l2cache_ena,
                  8'h0,
                  cfg_slots[7:0],
                  plic_irq_max[7:0]};
    end else if (wb_req_addr[11:3] == 9'd2) begin
        vrdata = r.idt;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.idt = wb_req_wdata;
        end
    end else if (wb_req_addr[11:3] == 9'd3) begin
        vrdata = r.malloc_addr;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_addr = wb_req_wdata;
        end
    end else if (wb_req_addr[11:3] == 9'd4) begin
        vrdata = r.malloc_size;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.malloc_size = wb_req_wdata;
        end
    end else if (wb_req_addr[11:3] == 9'd5) begin
        vrdata = r.fwdbg1;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg1 = wb_req_wdata;
        end
    end else if (wb_req_addr[11:3] == 9'd6) begin
        vrdata = r.fwdbg2;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg2 = wb_req_wdata;
        end
    end else if (wb_req_addr[11:3] == 9'd7) begin
        vrdata = r.fwdbg3;
        if ((w_req_valid & w_req_write) == 1'b1) begin
            v.fwdbg3 = wb_req_wdata;
        end
    end else if ((int'(wb_req_addr[11:3]) >= 8) & (int'(wb_req_addr[11:3]) < 8+4*cfg_slots)) begin
        vrdata = cfgmap[int'(wb_req_addr[11:3]) - 8];
    end

    v.rdata = vrdata;

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

